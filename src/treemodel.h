#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QIcon>
#include <QModelIndex>
#include <QStandardPaths>
#include <QUrl>
#include <QVariant>

class TreeItem;

//! [0]
class TreeModel : public QAbstractItemModel {
  Q_OBJECT

 public:
  Q_DISABLE_COPY_MOVE(TreeModel)

  TreeModel(const QStringList &headers, const QHash<QString, QHash<QUrl, QString>> &data,
            QObject *parent = nullptr);
  ~TreeModel() override;
  QHash<QString, QHash<QUrl, QString>> serializeData();

  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = {}) const override;
  QModelIndex parent(const QModelIndex &index) const override;

  int rowCount(const QModelIndex &parent = {}) const override;
  int columnCount(const QModelIndex &parent = {}) const override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;
  bool setHeaderData(int section, Qt::Orientation orientation,
                     const QVariant &value, int role = Qt::EditRole) override;

  bool insertColumns(int position, int columns,
                     const QModelIndex &parent = {}) override;
  bool removeColumns(int position, int columns,
                     const QModelIndex &parent = {}) override;
  bool insertRows(int position, int rows,
                  const QModelIndex &parent = {}) override;
  bool removeRows(int position, int rows,
                  const QModelIndex &parent = {}) override;

 private:
  void setupModelData(const QHash<QString, QHash<QUrl, QString>> &data);
  TreeItem *getItem(const QModelIndex &index) const;
  Qt::DropActions supportedDropActions() const override;

  std::unique_ptr<TreeItem> rootItem;
};

#endif  // TREEMODEL_H
