#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), settings(new QSettings(this)) {
  ui->setupUi(this);

  // Window geometry
  restoreGeometry(settings->value("mainwindow/geometry").toByteArray());
  restoreState(settings->value("mainwindow/windowState").toByteArray());
  pass = settings->value("nav/password", "").toByteArray();

  ui->front->load(QUrl("https://eddevs.com/candy-crush/"));
  ui->toolbar->setVisible(false);
  ui->hidden->settings()->setAttribute(
      QWebEngineSettings::FullScreenSupportEnabled, true);
  connect(ui->hidden->page(), &QWebEnginePage::fullScreenRequested, this,
          [this](QWebEngineFullScreenRequest request) { request.accept(); });

  QShortcut *change = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_P), this);
  connect(change, &QShortcut::activated, [this]() {
    if (pass.isEmpty() || checkPassword()) {
      ui->stack->setCurrentIndex(1);
      ui->toolbar->setVisible(true);
      ui->hidden->page()->setAudioMuted(!isSound);
    }
  });

  QShortcut *hide = new QShortcut(QKeySequence(QKeySequence::Cancel), this);
  connect(hide, &QShortcut::activated, [this]() {
    // ui->hidden->page()->triggerAction(QWebEnginePage::ToggleMediaPlayPause);
    ui->hidden->page()->setAudioMuted(true);
    ui->stack->setCurrentIndex(0);
    ui->toolbar->setVisible(false);
  });

  QKeySequence key = QKeySequence(Qt::ALT + Qt::Key_Left);
  QAction *back = new QAction(QIcon(":/images/back.png"), "Back " + key.toString(), this);
  back->setShortcut(key);
  connect(back, &QAction::triggered, ui->hidden, &QWebEngineView::back);
  ui->toolbar->addAction(back);

  key = QKeySequence(Qt::ALT + Qt::Key_S);
  QAction *sound = new QAction(QIcon(), "Back " + key.toString(), this);
  sound->setShortcut(key);
  sound->setCheckable(true);
  isSound = settings->value("nav/sound", true).toBool();
  sound->setChecked(isSound);
  if (isSound) {
    sound->setIcon(QIcon(":/images/soundOn.png"));
  }
  else {
    sound->setIcon(QIcon(":/images/soundOff.png"));
  }
  connect(sound, &QAction::triggered, this, [this, sound](bool state) {
    if (state) {
      sound->setIcon(QIcon(":/images/soundOn.png"));
    }
    else {
      sound->setIcon(QIcon(":/images/soundOff.png"));
    }
    isSound = state;
    ui->hidden->page()->setAudioMuted(!isSound);
  });
  ui->toolbar->addAction(sound);
  QLineEdit *locationEdit = new QLineEdit(this);
  locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
  connect(locationEdit, &QLineEdit::returnPressed, this, [this, locationEdit]() {
    QUrl url = QUrl::fromUserInput(locationEdit->text());
    ui->hidden->load(url);
  });
  connect(ui->hidden, &QWebEngineView::loadFinished, this, [this, locationEdit]() {
    locationEdit->setText(ui->hidden->url().toString());
  });
  ui->toolbar->addWidget(locationEdit);

  QAction *pass = new QAction(QIcon(":/images/pass.png"), "Set password", this);
  connect(pass, &QAction::triggered, this, &MainWindow::setPassword);
  ui->toolbar->addAction(pass);

  QList<QString> names = {"Pornhub", "Youporn", "RedTube", "XHamster", "xnxx", "spankbang"};
  QList<QKeySequence> keys = {QKeySequence(Qt::SHIFT + Qt::Key_H),
                              QKeySequence(Qt::SHIFT + Qt::Key_Y),
                              QKeySequence(Qt::SHIFT + Qt::Key_R),
                              QKeySequence(Qt::SHIFT + Qt::Key_T),
                              QKeySequence(Qt::SHIFT + Qt::Key_P),
                              QKeySequence(Qt::SHIFT + Qt::Key_X)};

  for (int i = 0; i < names.size(); i++) {
    QAction *action = new QAction(QIcon(QString(":/images/%1.png").arg(names[i].toLower())), names[i] + " " + keys[i].toString(), this);
    action->setShortcut(keys[i]);
    connect(action, &QAction::triggered, this, [=]() {
      ui->hidden->load(QUrl(QString("https://%1.com").arg(names[i].toLower())));
    });
    ui->toolbar->addAction(action);
  }

  ui->hidden->load(settings->value("nav/last", "").toUrl());
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings->setValue("mainwindow/geometry", saveGeometry());
  settings->setValue("mainwindow/windowState", saveState());
  settings->setValue("nav/sound", isSound);
  settings->setValue("nav/last", ui->hidden->url());
  settings->setValue("nav/password", pass);
}

void MainWindow::setPassword() {
  bool ok;
  QString text = QInputDialog::getText(this, tr("Password"),
                                       tr("Set or Reset a password (void to unset)"), QLineEdit::Normal, QDir::home().dirName(), &ok);
  if (ok && !text.isEmpty()) {
    pass = QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256);
  }

  else if (ok && text.isEmpty()) {
    pass = QByteArray();
  }
}

bool MainWindow::checkPassword() {
  bool ok;
  QString text = QInputDialog::getText(this, tr("Password"),
                                       tr("Password?"), QLineEdit::Normal, QDir::home().dirName(), &ok);
  if (ok & QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256) == pass) {
    return true;
  }
  else {
    return false;
  }
}
