#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), settings(new QSettings(this)), inactivity(new QTimer(this)) {
  ui->setupUi(this);

  // Window geometry
  restoreGeometry(settings->value("mainwindow/geometry").toByteArray());
  restoreState(settings->value("mainwindow/windowState").toByteArray());
  pass = settings->value("nav/password", "").toByteArray();

  // Timer
  connect(inactivity, &QTimer::timeout, this, [this]() {
    ui->hidden->page()->setAudioMuted(true);
    ui->stack->setCurrentIndex(0);
    ui->toolbar->setVisible(false);
  });

  ui->front->load(QUrl("https://eddevs.com/candy-crush/"));
  ui->toolbar->setVisible(false);
  ui->hidden->settings()->setAttribute(
      QWebEngineSettings::FullScreenSupportEnabled, true);
  connect(ui->hidden->page(), &QWebEnginePage::fullScreenRequested, this,
          [this](QWebEngineFullScreenRequest request) { request.accept(); });

  QShortcut *change = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P), this);
  connect(change, &QShortcut::activated, [this]() {
    if (pass.isEmpty() || checkPassword()) {
      ui->stack->setCurrentIndex(1);
      ui->toolbar->setVisible(true);
      ui->hidden->page()->setAudioMuted(!isSound);
    }
  });

  QShortcut *hide = new QShortcut(QKeySequence(Qt::Key_Escape), this);
  connect(hide, &QShortcut::activated, [this]() {
    // ui->hidden->page()->triggerAction(QWebEnginePage::ToggleMediaPlayPause);
    ui->hidden->page()->setAudioMuted(true);
    ui->stack->setCurrentIndex(0);
    ui->toolbar->setVisible(false);
  });

  QKeySequence key = QKeySequence(Qt::ALT | Qt::Key_Left);
  QAction *back = new QAction(QIcon(":/images/back.png"), "Back " + key.toString(), this);
  back->setShortcut(key);
  connect(back, &QAction::triggered, ui->hidden, &QWebEngineView::back);
  ui->toolbar->addAction(back);

  loadFav();
  QAction *favAction = new QAction(QIcon(":/images/favorite.png"), "Favorite", this);
  connect(favAction, &QAction::triggered, this, [this]() {
    insertFav(ui->hidden->url().toEncoded());
  });
  favMenu = new QMenu(tr("Favorites"), this);
  QMapIterator<QString, QString> i(fav);
  while (i.hasNext()) {
    i.next();
    QAction *menuAction = new QAction(i.key(), this);
    connect(menuAction, &QAction::triggered, this, [this, i]() {
      ui->hidden->setUrl(QUrl(i.value()));
    });
    QMenu *subMenu = new QMenu(this);
    QAction *subDelete = new QAction("Delete", this);
    subMenu->addAction(subDelete);
    connect(subDelete, &QAction::triggered, this, [=]() {
      menuAction->deleteLater();
      fav.remove(i.key());
    });
    menuAction->setMenu(subMenu);
    favMenu->addAction(menuAction);
  }
  favAction->setMenu(favMenu);
  ui->toolbar->addAction(favAction);

  key = QKeySequence(Qt::ALT | Qt::Key_S);
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
  QList<QKeySequence> keys = {QKeySequence(Qt::SHIFT | Qt::Key_H),
                              QKeySequence(Qt::SHIFT | Qt::Key_Y),
                              QKeySequence(Qt::SHIFT | Qt::Key_R),
                              QKeySequence(Qt::SHIFT | Qt::Key_T),
                              QKeySequence(Qt::SHIFT | Qt::Key_P),
                              QKeySequence(Qt::SHIFT | Qt::Key_X)};

  for (int i = 0; i < names.size(); i++) {
    QAction *action = new QAction(QIcon(QString(":/images/%1.png").arg(names[i].toLower())), names[i] + " " + keys[i].toString(), this);
    action->setShortcut(keys[i]);
    connect(action, &QAction::triggered, this, [=]() {
      ui->hidden->load(QUrl(QString("https://%1.com").arg(names[i].toLower())));
    });
    ui->toolbar->addAction(action);
  }

  ui->hidden->load(settings->value("nav/last", "").toUrl());

  if (settings->value("mainwindow/first", true).toBool()) {
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(QPixmap(":/images/intro.jpg"));
    msgBox.setTextFormat(Qt::MarkdownText);
    msgBox.setText(tr("### Read this message VERY carefully, this message will never appear again.  \n\nTo get to the good stuff press **%1**. You can then choose a password to securise the browser.  \nIn case of ermergency, press **%2** to return to the CandyCrush page.").arg(QKeySequence(Qt::CTRL | Qt::Key_P).toString(), QKeySequence(Qt::Key_Escape).toString()));
    msgBox.setStyleSheet("width: 800px; font-size: 20px; color: red;");
    msgBox.exec();
  }

  checkForUpdates();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings->setValue("mainwindow/geometry", saveGeometry());
  settings->setValue("mainwindow/windowState", saveState());
  settings->setValue("mainwindow/first", false);
  settings->setValue("nav/sound", isSound);
  settings->setValue("nav/last", ui->hidden->url());
  settings->setValue("nav/password", pass);
  writeFav();
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

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  inactivity->start(1000 * 120);
  return QWidget::mouseMoveEvent(event);
}

void MainWindow::loadFav() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QFile file(path + "/fav.candy");
  if (file.open(QIODevice::ReadOnly)) {
    QString decoded = QString(QByteArray::fromBase64(file.readAll()));
    QStringList lines = decoded.split("\n", Qt::SkipEmptyParts);
    for (const auto &a : lines) {
      QStringList favs = a.split(";");
      fav.insert(favs[0], favs[1]);
    }
  }
}

void MainWindow::writeFav() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(path);
  QFile file(path + "/fav.candy");
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream out(&file);
    QMapIterator<QString, QString> i(fav);
    QString saveFile;
    while (i.hasNext()) {
      i.next();
      saveFile += i.key() + ";" + i.value() + "\n";
    }
    QByteArray write = saveFile.toUtf8().toBase64();
    out << write;
  }
}

void MainWindow::insertFav(const QString &link) {
  bool ok;
  QString text = QInputDialog::getText(this, tr("Favorite label"),
                                       tr("Choose a label"), QLineEdit::Normal, link, &ok);
  if (ok & !text.isEmpty()) {
    fav.insert(text, link);
    QAction *menuAction = new QAction(text);
    connect(menuAction, &QAction::triggered, this, [this, link]() {
      ui->hidden->setUrl(QUrl(link));
    });
    QMenu *subMenu = new QMenu(this);
    QAction *subDelete = new QAction("Delete", this);
    subMenu->addAction(subDelete);
    connect(subDelete, &QAction::triggered, this, [=]() {
      menuAction->deleteLater();
      fav.remove(text);
    });
    menuAction->setMenu(subMenu);
    favMenu->addAction(menuAction);
  }
}

void MainWindow::checkForUpdates() {
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QByteArray downloadedData;
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  QEventLoop eventLoop;
  connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
  QNetworkReply *reply = manager->get(QNetworkRequest(QUrl("https://gitlab.com/secretcrush1/SecretCrushBrowser/-/raw/master/CMakeLists.txt")));
  eventLoop.exec();
  downloadedData = reply->readAll().mid(3, 5);
  reply->deleteLater();
  delete manager;
  QApplication::restoreOverrideCursor();
  if (downloadedData > QApplication::applicationVersion()) {
    ui->statusbar->showMessage(tr("A new version of SecretCrush in available!"));
  }
}
