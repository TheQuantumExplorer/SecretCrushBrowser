#include "performancedialog.h"
#include "mainwindow.h"
#include "ui_performancedialog.h"

PerformanceDialog::PerformanceDialog(QWidget* parent) : QDialog(parent),
                                                        ui(new Ui::PerformanceDialog),
                                                        activity(new QTimer),
                                                        timer(new QElapsedTimer) {
  ui->setupUi(this);
  this->setWindowModality(Qt::ApplicationModal);
  setAttribute(Qt::WA_Hover);
  loadHistory();

  chartView = new QChartView(this);
  ui->tabWidget->addTab(chartView, "Stat");
  computeDistribution();

  session = history.value(QDate::currentDate());
  if (session.isEmpty()) {
    fapCount = 0;
    overallStart = QTime(0, 0, 0);
  }
  else {
    fapCount = session.count() - 1;
    overallStart = session.value("overall");
  }

  activity->start(500);
  timer->start();
  connect(activity, &QTimer::timeout, this, [this]() {
    ui->overall->display(int(timer->elapsed() / 1000) + QTime(0, 0, 0).secsTo(overallStart));
  });

  connect(ui->fapButton, &QAbstractButton::toggled, this, [this](bool state) {
    if (state) {
      fapStart = timer->elapsed() / 1000;
      ui->fapButton->setText("I came!");
    }
    else {
      fapStop = timer->elapsed() / 1000;
      session.insert(QString::number(fapCount), QTime(0, 0, 0).addSecs(fapStop - fapStart));
      fapCount++;
      session.insert("overall", overallStart.addSecs(timer->elapsed() / 1000));
      history.insert(QDate::currentDate(), session);
      ui->calendar->setSelectedDate(QDate(1, 1, 1));
      ui->calendar->setSelectedDate(QDate::currentDate());
      computeDistribution();
      ui->fapButton->setText("Start Stroking");
    }
  });

  connect(activity, &QTimer::timeout, this, [this, parent]() {
    fapStop = timer->elapsed() / 1000;
    if (ui->fapButton->isChecked()) {
      int fapStop = timer->elapsed() / 1000;
      ui->fap->display(fapStop - fapStart);
      static_cast<MainWindow*>(parent)->setMessage(tr("Fap Session: ") + QString::number(fapStop - fapStart) + "s");
    }
  });

  connect(ui->calendar, &QCalendarWidget::selectionChanged, this, [this]() {
    auto data = history.value(ui->calendar->selectedDate());
    QString text = "## Naughty Summary\n| Session        | Time |\n|-------------|----------|----------|\n";
    text += QString("| Overall       |  %1 |\n").arg(data.value("overall").toString("hh::mm::ss"));
    for (int i = 0; i < data.count() - 1; i++) {
      text += QString("| %1       |  %2 | %3 |\n").arg(QString::number(i)).arg(data.value(QString::number(i)).toString("hh::mm::ss")).arg(getEmoticonForTime(data.value(QString::number(i))));
    }
    if (!data.isEmpty()) {
      ui->summary->setMarkdown(text);
    }
    else {
      ui->summary->setPlainText("None");
    }
  });
  ui->calendar->setSelectedDate(QDate(1, 1, 1));
  ui->calendar->setSelectedDate(QDate::currentDate());
}

QString PerformanceDialog::getEmoticonForTime(const QTime& time) {
  if (time.hour() >= 1) {
    return "🥵";
  }
  else if (time.minute() >= 30) {
    return "😩";
  }
  else if (time.minute() >= 10) {
    return "😅";
  }
  else if (time.minute() >= 3) {
    return "🙂";
  }
  else {
    return "😞";
  }
}

void PerformanceDialog::loadHistory() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QFile file(path + "/perf.candy");
  if (file.open(QIODevice::ReadOnly)) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromBase64(file.readAll()));
    QJsonObject jsonObject = jsonDoc.object();
    for (auto it = jsonObject.constBegin(); it != jsonObject.constEnd(); ++it) {
      QDate date = QDate::fromString(it.key(), Qt::ISODate);
      QJsonObject nestedObject = it.value().toObject();
      QHash<QString, QTime> innerMap;
      for (auto innerIt = nestedObject.constBegin(); innerIt != nestedObject.constEnd(); ++innerIt) {
        innerMap.insert(innerIt.key(), QTime::fromString(innerIt.value().toString(), "hh:mm:ss"));
      }
      history.insert(date, innerMap);
    }
  }
}

void PerformanceDialog::saveHistory() {
  ui->fapButton->setChecked(false);
  session.insert("overall", overallStart.addSecs(timer->elapsed() / 1000));
  history.insert(QDate::currentDate(), session);

  QJsonObject jsonObject;
  for (auto it = history.constBegin(); it != history.constEnd(); ++it) {
    QJsonObject nestedObject;
    for (auto innerIt = it.value().constBegin(); innerIt != it.value().constEnd(); ++innerIt) {
      nestedObject[innerIt.key()] = innerIt.value().toString("hh:mm:ss");
    }
    jsonObject[it.key().toString(Qt::ISODate)] = nestedObject;
  }

  QJsonDocument jsonDoc(jsonObject);
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(path);
  QFile file(path + "/perf.candy");
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream out(&file);
    QByteArray write = jsonDoc.toJson().toBase64();
    out << write;
  }
}

void PerformanceDialog::computeDistribution() {
  int precision = 30;
  QList<qreal> distribution(1200);
  for (auto [key, value] : history.asKeyValueRange()) {
    for (auto [subKey, subValue] : value.asKeyValueRange()) {
      if (subKey != "overall") {
        int time = QTime(0, 0, 0).secsTo(subValue);
        distribution[int(time / precision)] += 1;
      }
    }
  }
  while (!distribution.isEmpty() && distribution.last() == 0) {
    distribution.removeLast();
  }

  auto chart = new QChart();

  QBarSet* bar = new QBarSet("");
  bar->append(distribution);

  QBarSeries* barSerie = new QBarSeries();
  barSerie->append(bar);

  chart->addSeries(barSerie);

  QCategoryAxis* axisX = new QCategoryAxis();
  for (int i = 0; i < distribution.size(); ++i) {
    axisX->append(QString::number(i * precision), i);
  }
  axisX->setTitleText("Time (s)");
  chart->addAxis(axisX, Qt::AlignBottom);
  barSerie->attachAxis(axisX);

  QValueAxis* axisY = new QValueAxis();
  axisY->setRange(0, *std::max_element(distribution.begin(), distribution.end()));
  axisY->setTitleText("Count");
  chart->addAxis(axisY, Qt::AlignLeft);
  barSerie->attachAxis(axisY);

  chart->setAnimationOptions(QChart::AllAnimations);
  chart->legend()->setVisible(false);
  chart->setTheme(QChart::ChartThemeDark);
  chartView->setChart(chart);
}

PerformanceDialog::~PerformanceDialog() {
  delete ui;
}

void PerformanceDialog::hideEvent(QHideEvent* event) {
  emit(visibilityChanged(false));
}
