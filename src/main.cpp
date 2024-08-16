#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QProcessEnvironment>
#include <QSplashScreen>
#include <QTranslator>

QString loadStyleSheet(const QString &fileName) {
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << "Unable to open file:" << fileName;
    return "";
  }

  QTextStream in(&file);
  return in.readAll();
}

int main(int argc, char *argv[]) {
  QProcess process;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("QTWEBENGINE_CHROMIUM_FLAGS", "--force-dark-mode --disable-logging");
  process.setProcessEnvironment(env);

  QApplication a(argc, argv);
  QPixmap pixmap(QStringLiteral(":/images/icon.png"));
  QSplashScreen splash(pixmap);
  splash.show();

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
    const QString baseName = "SecretCrush_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      a.installTranslator(&translator);
      break;
    }
  }
  a.setOrganizationName("SecretCrushOrg");
  a.setApplicationName("SecretCrush");
  a.setApplicationVersion("0.0.1");
  a.setWindowIcon(QIcon(":/images/icon.png"));
  a.setStyleSheet(loadStyleSheet(":/dark-style.css"));
  MainWindow w;
  w.show();
  splash.finish(&w);
  return a.exec();
}
