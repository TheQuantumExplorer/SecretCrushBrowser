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
#include "favwindow.h"
#include "performancedialog.h"

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
  PerformanceDialog *performance;
  FavWindow *favWindow;
  bool isSound;
  void closeEvent(QCloseEvent *event) override;
  void setPassword();
  bool checkPassword();
  QByteArray pass;
  bool eventFilter(QObject *obj, QEvent *event) override;
  QTimer *inactivity;
  QHash<QString, QHash<QUrl, QString>> loadFav();
  QHash<QString, QHash<QUrl, QString>> loadFavLegacy();
  void writeFav(const QHash<QString, QHash<QUrl, QString>> &data);
  void loadHist();
  void writeHist();
  QMap<QString, QString> hist;
  QMenu *histMenu;
  void checkForUpdates();
  QNetworkAccessManager *manager;
  void getAssets(QNetworkReply *reply);
  QString getFavicon(const QUrl &url);
  QString getFaviconBlocking(const QUrl &url);
  QStringList assets;
  void addToHistMenu(const QString &key, const QString &value, const QString &path);
  void loadHistMenu();
  void deleteAssets();
};
#endif  // MAINWINDOW_H
