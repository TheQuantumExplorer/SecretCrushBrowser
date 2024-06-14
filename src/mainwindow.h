#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QGraphicsBlurEffect>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineHistory>
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
  void closeEvent(QCloseEvent *event) override;
  void setPassword();
  bool checkPassword();
  QByteArray pass;
  bool eventFilter(QObject *obj, QEvent *event) override;
  QTimer *inactivity;
  void loadFav();
  void writeFav();
  void loadHist();
  void writeHist();
  void insertFav(const QString &fav);
  QMap<QString, QString> fav;
  QMap<QString, QString> hist;
  QMenu *favMenu;
  QMenu *histMenu;
  void checkForUpdates();
  QNetworkAccessManager *manager;
  void getAssets(QNetworkReply *reply);
  QString getFavicon(const QUrl &url);
  QStringList assets;
  void addToFavMenu(const QString &key, const QString &value, const QString &path);
  void addToHistMenu(const QString &key, const QString &value, const QString &path);
  void loadHistMenu();
  void deleteAssets();
};
#endif  // MAINWINDOW_H
