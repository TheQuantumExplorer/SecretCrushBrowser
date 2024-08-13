#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), settings(new QSettings(this)), inactivity(new QTimer(this)), manager(new QNetworkAccessManager(this)), performance(new PerformanceDialog(this)) {
  ui->setupUi(this);
  this->installEventFilter(this);

  // Window geometry
  restoreGeometry(settings->value("mainwindow/geometry").toByteArray());
  restoreState(settings->value("mainwindow/windowState").toByteArray());
  pass = settings->value("nav/password", "").toByteArray();

  connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::getAssets);

  // Performance
  QAction *perf = ui->toolbar->addAction(QIcon(":/images/perf.png"), "Show Performance");
  perf->setCheckable(true);
  connect(perf, &QAction::toggled, performance, &PerformanceDialog::setVisible);
  performance->installEventFilter(this);

  // Timer
  connect(inactivity, &QTimer::timeout, this, [this, perf]() {
    ui->hidden->page()->setAudioMuted(true);
    ui->stack->setCurrentIndex(0);
    ui->toolbar->setVisible(false);
    perf->setChecked(false);
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
  connect(hide, &QShortcut::activated, [this, perf]() {
    // ui->hidden->page()->triggerAction(QWebEnginePage::ToggleMediaPlayPause);
    ui->hidden->page()->setAudioMuted(true);
    ui->stack->setCurrentIndex(0);
    ui->toolbar->setVisible(false);
    perf->setChecked(false);
  });

  QAction *pass = ui->toolbar->addAction(QIcon(":/images/pass.png"), "Set password");
  connect(pass, &QAction::triggered, this, &MainWindow::setPassword);

  QKeySequence key = QKeySequence(Qt::ALT | Qt::Key_S);
  QAction *sound = ui->toolbar->addAction(QIcon(), tr("Sound ") + key.toString());
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

  key = QKeySequence(Qt::ALT | Qt::Key_Left);
  QAction *back = ui->toolbar->addAction(QIcon(":/images/back.png"), tr("Go Back ") + key.toString());
  back->setShortcut(key);
  connect(back, &QAction::triggered, ui->hidden, &QWebEngineView::back);

  key = QKeySequence(Qt::ALT | Qt::Key_Right);
  QAction *forward = ui->toolbar->addAction(QIcon(":/images/forward.png"), tr("Go Forward ") + key.toString());
  forward->setShortcut(key);
  connect(forward, &QAction::triggered, ui->hidden, &QWebEngineView::forward);

  QLineEdit *locationEdit = new QLineEdit(this);
  locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
  connect(locationEdit, &QLineEdit::returnPressed, this, [this, locationEdit]() {
    QUrl url = QUrl::fromUserInput(locationEdit->text());
    ui->hidden->load(url);
  });
  connect(ui->hidden, &QWebEngineView::loadFinished, this, [this, locationEdit]() {
    locationEdit->setText(ui->hidden->url().toString());
    auto a = ui->hidden->history()->currentItem();
    hist.insert(a.lastVisited().toString() + " " + a.url().toString(), a.url().toString());
    loadHistMenu();
  });
  QAction *favicon = new QAction(this);
  connect(ui->hidden, &QWebEngineView::iconChanged, favicon, &QAction::setIcon);
  locationEdit->addAction(favicon, QLineEdit::LeadingPosition);
  ui->toolbar->addWidget(locationEdit);

  loadHist();
  QAction *histAction = new QAction(QIcon(":/images/history.png"), tr("History"), this);
  histMenu = new QMenu(tr("History"), this);
  histMenu->setTearOffEnabled(true);
  histMenu->setStyleSheet("QMenu { menu-scrollable: 1;}");
  connect(histAction, &QAction::triggered, this, [this]() {
    histMenu->popup(QCursor::pos());
  });
  histAction->setMenu(histMenu);
  ui->toolbar->addAction(histAction);
  loadHistMenu();

  loadFav();
  QAction *favAction = new QAction(QIcon(":/images/favorite.png"), tr("Bookmarks"), this);
  connect(favAction, &QAction::triggered, this, [this]() {
    insertFav(ui->hidden->url().toEncoded());
  });
  favMenu = new QMenu(tr("Bookmarks"), this);
  favMenu->setStyleSheet("QMenu { menu-scrollable: 1; }");
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
    QAction *action = ui->toolbar->addAction(QIcon(), i);
    QTimer::singleShot(4000, this, [=]() {
      action->setIcon(QIcon(favPath));
    });
    connect(action, &QAction::triggered, this, [=]() {
      ui->hidden->load(siteUrl);
    });
  }

  ui->hidden->load(QUrl::fromEncoded(QByteArray::fromBase64(settings->value("nav/last", "").toByteArray())));

  if (settings->value("mainwindow/first", true).toBool()) {
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(QPixmap(":/images/intro.jpg"));
    msgBox.setTextFormat(Qt::MarkdownText);
    msgBox.setText(tr("### Important Notice\n\nPlease read this message carefully as it will not be displayed again.\n\nTo access the features, press **%1**. You will then have the option to set a password to secure the browser.\n\nIn case of an emergency, press **%2** to return to the Candy Crush page.\n\nBy proceeding, you confirm that you are of legal age in your country of residence.").arg(QKeySequence(Qt::CTRL | Qt::Key_P).toString(), QKeySequence(Qt::Key_Escape).toString()));
    msgBox.setStyleSheet("width: 800px; font-size: 20px; color: red;");
    msgBox.exec();
  }

  checkForUpdates();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  writeFav();
  writeHist();
  performance->saveHistory();
  settings->setValue("mainwindow/geometry", saveGeometry());
  settings->setValue("mainwindow/windowState", saveState());
  settings->setValue("mainwindow/first", false);
  settings->setValue("nav/sound", isSound);
  settings->setValue("nav/last", ui->hidden->url().toEncoded().toBase64());
  settings->setValue("nav/password", pass);
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
                                       tr("Password?"), QLineEdit::Password, QString(), &ok);
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

void MainWindow::loadHist() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QFile file(path + "/hist.candy");
  if (file.open(QIODevice::ReadOnly)) {
    QString decoded = QString(QByteArray::fromBase64(file.readAll()));
    QStringList lines = decoded.split("\n", Qt::SkipEmptyParts);
    for (const auto &a : lines) {
      QStringList h = a.split(";");
      hist.insert(h[0], h[1]);
    }
  }
}

void MainWindow::writeHist() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(path);
  QFile file(path + "/hist.candy");
  if (file.open(QIODevice::WriteOnly)) {
    QMapIterator<QString, QString> i(hist);
    QTextStream out(&file);
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
  QUrl faviconUrl = QUrl(QString("https://t1.gstatic.com/faviconV2?client=SOCIAL&type=FAVICON&fallback_opts=TYPE,SIZE,URL&url=https://%1&size=64").arg(url.host()));
  QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + url.host().toUtf8().toBase64();
  if (!assets.contains(path)) {
    manager->get(QNetworkRequest(faviconUrl));
  }
  return path;
}

void MainWindow::addToFavMenu(const QString &key, const QString &value, const QString &path) {
  QAction *menuAction = favMenu->addAction(key);
  QTimer::singleShot(1000, this, [=]() {
    menuAction->setIcon(QIcon(path));
  });
  QMenu *subMenu = new QMenu(this);
  QAction *subGo = subMenu->addAction(tr("Go"));
  connect(subGo, &QAction::triggered, this, [=]() {
    ui->hidden->setUrl(QUrl(value));
  });
  QAction *subChange = subMenu->addAction(tr("Change label"));
  connect(subChange, &QAction::triggered, this, [=]() {
    insertFav(value);
    menuAction->deleteLater();
    fav.remove(key);
  });
  QAction *subDelete = subMenu->addAction(tr("Remove"));
  connect(subDelete, &QAction::triggered, this, [=]() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Remove"), tr("Confirm favorite removing?"), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      menuAction->deleteLater();
      fav.remove(key);
    }
  });
  menuAction->setMenu(subMenu);
}

void MainWindow::loadHistMenu() {
  histMenu->clear();
  QMapIterator<QString, QString> j(hist);
  j.toBack();
  while (j.hasPrevious()) {
    j.previous();
    addToHistMenu(j.key(), j.value(), getFavicon(QUrl(j.value())));
  }
}

void MainWindow::addToHistMenu(const QString &key, const QString &value, const QString &path) {
  QAction *menuAction = histMenu->addAction(key);
  menuAction->setIcon(QIcon(getFavicon(value)));
  connect(menuAction, &QAction::triggered, this, [=]() {
    ui->hidden->setUrl(value);
  });
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
  else if (event->type() == QEvent::Leave) {
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

