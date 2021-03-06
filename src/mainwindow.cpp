#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), settings(new QSettings(this)), inactivity(new QTimer(this)), manager(new QNetworkAccessManager(this)) {
  ui->setupUi(this);
  this->installEventFilter(this);

  // Window geometry
  restoreGeometry(settings->value("mainwindow/geometry").toByteArray());
  restoreState(settings->value("mainwindow/windowState").toByteArray());
  pass = settings->value("nav/password", "").toByteArray();

  connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::getAssets);

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

  QAction *pass = new QAction(QIcon(":/images/pass.png"), "Set password", this);
  connect(pass, &QAction::triggered, this, &MainWindow::setPassword);
  ui->toolbar->addAction(pass);

  QKeySequence key = QKeySequence(Qt::ALT | Qt::Key_S);
  QAction *sound = new QAction(QIcon(), tr("Sound ") + key.toString(), this);
  sound->setShortcut(key);
  sound->setCheckable(true);
  connect(sound, &QAction::toggled, this, [this, sound](bool state) {
    if (state) {
      sound->setIcon(QIcon(":/images/soundOn.png"));
      sound->setText(tr("Click to mute"));
    }
    else {
      sound->setIcon(QIcon(":/images/soundOff.png"));
      sound->setText(tr("Click to unmute"));
    }
    isSound = state;
    ui->hidden->page()->setAudioMuted(!isSound);
  });
  isSound = settings->value("nav/sound", true).toBool();
  sound->toggled(isSound);  // Force update even is already in right state
  sound->setChecked(isSound);
  ui->toolbar->addAction(sound);

  key = QKeySequence(Qt::ALT | Qt::Key_Left);
  QAction *back = new QAction(QIcon(":/images/back.png"), tr("Go Back ") + key.toString(), this);
  back->setShortcut(key);
  connect(back, &QAction::triggered, ui->hidden, &QWebEngineView::back);
  ui->toolbar->addAction(back);

  QLineEdit *locationEdit = new QLineEdit(this);
  locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
  connect(locationEdit, &QLineEdit::returnPressed, this, [this, locationEdit]() {
    QUrl url = QUrl::fromUserInput(locationEdit->text());
    ui->hidden->load(url);
  });
  connect(ui->hidden, &QWebEngineView::loadFinished, this, [this, locationEdit]() {
    locationEdit->setText(ui->hidden->url().toString());
  });
  QAction *favicon = new QAction(this);
  connect(ui->hidden, &QWebEngineView::iconChanged, favicon, &QAction::setIcon);
  locationEdit->addAction(favicon, QLineEdit::LeadingPosition);
  ui->toolbar->addWidget(locationEdit);

  loadFav();
  QAction *favAction = new QAction(QIcon(":/images/favorite.png"), tr("Bookmarks"), this);
  connect(favAction, &QAction::triggered, this, [this]() {
    insertFav(ui->hidden->url().toEncoded());
  });
  favMenu = new QMenu(tr("Bookmarks"), this);
  QMapIterator<QString, QString> i(fav);
  while (i.hasNext()) {
    i.next();
    addToFavMenu(i.key(), i.value(), getFavicon(QUrl(i.value())));
  }
  favAction->setMenu(favMenu);
  ui->toolbar->addAction(favAction);

  QList<QString> names = {"Pornhub", "Youporn", "RedTube", "XHamster", "xnxx", "Spankbang", "HQporner", "XVideos", "EPorner", "DaftSex", "Beeg", "PornGo", "CumLouder", "PornTube", "4Tube"};
  for (auto const &i : names) {
    QUrl siteUrl(QString("https://%1.com").arg(i.toLower()));
    QString favPath = getFavicon(siteUrl);
    QAction *action = new QAction(QIcon(), i, this);
    QTimer::singleShot(2000, this, [=]() {
      action->setIcon(QIcon(favPath));
    });
    connect(action, &QAction::triggered, this, [=]() {
      ui->hidden->load(siteUrl);
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
  deleteAssets();
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
    addToFavMenu(text, link, getFavicon(QUrl(link)));
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

void MainWindow::getAssets(QNetworkReply *reply) {
  QRegularExpression re("url=http\\w://(.+)&");
  QRegularExpressionMatchIterator matches = re.globalMatch(reply->request().url().toString());
  while (matches.hasNext()) {
    QRegularExpressionMatch match = matches.next();
    QString host = match.captured(1);
    QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + host.toUtf8().toBase64();
    QByteArray data = reply->readAll();
    QFile file(path);
    if (!assets.contains(path) && file.open(QIODevice::WriteOnly)) {
      file.write(data);
      file.close();
      assets.append(path);
    }
  }
}

QString MainWindow::getFavicon(const QUrl &url) {
  QUrl faviconUrl = QUrl(QString("https://t3.gstatic.com/faviconV2?client=SOCIAL&type=FAVICON&fallback_opts=TYPE,SIZE,URL&url=https://%1&size=48").arg(url.host()));
  manager->get(QNetworkRequest(faviconUrl));
  return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + url.host().toUtf8().toBase64();
}

void MainWindow::addToFavMenu(const QString &key, const QString &value, const QString &path) {
  QAction *menuAction = new QAction(key, this);
  QTimer::singleShot(4000, this, [=]() {
    menuAction->setIcon(QIcon(path));
  });
  QMenu *subMenu = new QMenu(this);
  QAction *subGo = new QAction(tr("Go"), this);
  subMenu->addAction(subGo);
  connect(subGo, &QAction::triggered, this, [=]() {
    ui->hidden->setUrl(QUrl(value));
  });
  QAction *subChange = new QAction(tr("Change label"), this);
  subMenu->addAction(subChange);
  connect(subChange, &QAction::triggered, this, [=]() {
    insertFav(value);
    menuAction->deleteLater();
    fav.remove(key);
  });
  QAction *subDelete = new QAction(tr("Remove"), this);
  subMenu->addAction(subDelete);
  connect(subDelete, &QAction::triggered, this, [=]() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Remove"), tr("Confirm favorite removing?"), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      menuAction->deleteLater();
      fav.remove(key);
    }
  });
  menuAction->setMenu(subMenu);
  favMenu->addAction(menuAction);
}

void MainWindow::deleteAssets() {
  for (auto const &a : assets) {
    QFile::remove(a);
  }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::Enter) {
    ui->hidden->setGraphicsEffect(nullptr);
    return true;
  }
  else if (event->type() == QEvent::HoverLeave) {
    QGraphicsBlurEffect *effect = new QGraphicsBlurEffect(this);
    effect->setBlurRadius(40);
    ui->hidden->setGraphicsEffect(effect);
    inactivity->start(5000);
    return true;
  }
  else if (event->type() == QEvent::HoverMove) {
    inactivity->start(1000 * 120);
    return true;
  }
  else {
    return QMainWindow::eventFilter(obj, event);
  }
}

