#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QSplashScreen>
#include <QTranslator>

int main(int argc, char *argv[]) {
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
  MainWindow w;
  w.show();
  splash.finish(&w);
  return a.exec();
}
