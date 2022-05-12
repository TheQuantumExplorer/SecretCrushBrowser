#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

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
  return a.exec();
}
