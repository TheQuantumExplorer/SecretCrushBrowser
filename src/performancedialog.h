#ifndef PERFORMANCEDIALOG_H
#define PERFORMANCEDIALOG_H

#include <QBarSeries>
#include <QBarSet>
#include <QCategoryAxis>
#include <QChart>
#include <QChartView>
#include <QDate>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QValueAxis>
#include <algorithm>

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
  QChartView *chartView;
  QTime overallStart;
  int fapStart;
  int fapStop;
  int fapCount;
  QHash<QString, QTime> session;
  QHash<QDate, QHash<QString, QTime>> history;
  void loadHistory();
  void computeDistribution();
  QString getEmoticonForTime(const QTime &time);
};

#endif  // PERFORMANCEDIALOG_H
