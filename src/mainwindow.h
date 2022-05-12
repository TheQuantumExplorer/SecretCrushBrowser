#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCryptographicHash>
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QSettings>
#include <QShortcut>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>
#include <QWebEngineView>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private:
  Ui::MainWindow *ui;
  QSettings *settings;
  bool isSound;
  void closeEvent(QCloseEvent *event);
  void setPassword();
  bool checkPassword();
  QByteArray pass;
};
#endif  // MAINWINDOW_H
