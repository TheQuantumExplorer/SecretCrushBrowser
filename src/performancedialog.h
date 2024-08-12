#ifndef PERFORMANCEDIALOG_H
#define PERFORMANCEDIALOG_H

#include <QDate>
#include <QDialog>
#include <QElapsedTimer>
#include <QHash>
#include <QPushButton>
#include <QTime>
#include <QTimer>

namespace Ui {
class PerformanceDialog;
}

class PerformanceDialog : public QDialog {
  Q_OBJECT

 public:
  explicit PerformanceDialog(QWidget *parent = nullptr);
  ~PerformanceDialog();
  void saveHistory();

 private:
  Ui::PerformanceDialog *ui;
  QTimer *activity;
  QElapsedTimer *timer;
  QTime overallStart;
  int fapStart;
  int fapStop;
  int fapCount;
  QHash<QString, QTime> session;
  QHash<QDate, QHash<QString, QTime>> history;
  void loadHistory();
  QString getEmoticonForTime(const QTime &time);
};

#endif  // PERFORMANCEDIALOG_H
