#ifndef FAVWINDOW_H
#define FAVWINDOW_H

#include "ui_favwindow.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QHash>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QUrl>
#include <QWidget>

class FavWindow : public QMainWindow, private Ui::FavWindow {
  Q_OBJECT

 public:
  FavWindow(const QHash<QString, QHash<QUrl, QString>> &data, QWidget *parent = nullptr);
  QUrl currentUrl;
  QIcon currentIcon;
  void setCurrentUrl(const QUrl &url);
  void setCurrentIcon(const QIcon &icon);

 public slots:
  void updateActions();
  QHash<QString, QHash<QUrl, QString>> getData();

 private slots:
  void insertCategory();
  void insertFav();
  void removeRow();

 protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void hideEvent(QHideEvent *event) override;

 signals:
  void loadFav(const QUrl &url);
  void visibilityChanged(bool);
};

#endif  // FAVWINDOW_H
