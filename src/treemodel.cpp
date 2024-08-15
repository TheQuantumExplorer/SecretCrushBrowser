#include "treemodel.h"
#include "treeitem.h"

using namespace Qt::StringLiterals;

TreeModel::TreeModel(const QStringList &headers, const QHash<QString, QHash<QUrl, QString>> &data, QObject *parent)
    : QAbstractItemModel(parent) {
  QVariantList rootData;
  for (const QString &header : headers)
    rootData << header;

  rootItem = std::make_unique<TreeItem>(rootData);
  setupModelData(data);
}

TreeModel::~TreeModel() = default;

int TreeModel::columnCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};

  if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::DecorationRole)
    return {};

  TreeItem *item = getItem(index);

  return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
  else if (index.column() == 1)
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  else
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const {
  if (index.isValid()) {
    if (auto *item = static_cast<TreeItem *>(index.internalPointer()))
      return item;
  }
  return rootItem.get();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const {
  return (orientation == Qt::Horizontal && role == Qt::DisplayRole)
             ? rootItem->data(section)
             : QVariant{};
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
  if (parent.isValid() && parent.column() != 0)
    return {};

  TreeItem *parentItem = getItem(parent);
  if (!parentItem)
    return {};

  if (auto *childItem = parentItem->child(row))
    return createIndex(row, column, childItem);
  return {};
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent) {
  beginInsertColumns(parent, position, position + columns - 1);
  const bool success = rootItem->insertColumns(position, columns);
  endInsertColumns();

  return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent) {
  TreeItem *parentItem = getItem(parent);
  if (!parentItem)
    return false;

  beginInsertRows(parent, position, position + rows - 1);
  const bool success = parentItem->insertChildren(position,
                                                  rows,
                                                  rootItem->columnCount());
  endInsertRows();

  return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return {};

  TreeItem *childItem = getItem(index);
  TreeItem *parentItem = childItem ? childItem->parent() : nullptr;

  return (parentItem != rootItem.get() && parentItem != nullptr)
             ? createIndex(parentItem->row(), 0, parentItem)
             : QModelIndex{};
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent) {
  beginRemoveColumns(parent, position, position + columns - 1);
  const bool success = rootItem->removeColumns(position, columns);
  endRemoveColumns();

  if (rootItem->columnCount() == 0)
    removeRows(0, rowCount());

  return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent) {
  TreeItem *parentItem = getItem(parent);
  if (!parentItem)
    return false;

  beginRemoveRows(parent, position, position + rows - 1);
  const bool success = parentItem->removeChildren(position, rows);
  endRemoveRows();

  return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid() && parent.column() > 0)
    return 0;

  const TreeItem *parentItem = getItem(parent);

  return parentItem ? parentItem->childCount() : 0;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  TreeItem *item = getItem(index);
  bool result = item->setData(index.column(), value);

  if (result)
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

  return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role) {
  if (role != Qt::EditRole || orientation != Qt::Horizontal)
    return false;

  const bool result = rootItem->setData(section, value);

  if (result)
    emit headerDataChanged(orientation, section, section);

  return result;
}

void TreeModel::setupModelData(const QHash<QString, QHash<QUrl, QString>> &data) {
  if (data.isEmpty()) {
    rootItem->insertChildren(rootItem->childCount(), 1, rootItem->columnCount());
    auto *parent = rootItem->child(rootItem->childCount() - 1);
    parent->setData(1, "Default Category");
    return;
  }
  for (auto [key, value] : data.asKeyValueRange()) {
    rootItem->insertChildren(rootItem->childCount(), 1, rootItem->columnCount());
    auto *parent = rootItem->child(rootItem->childCount() - 1);
    parent->setData(1, key);
    for (auto [key, value] : value.asKeyValueRange()) {
      parent->insertChildren(parent->childCount(), 1, parent->columnCount());
      parent->child(parent->childCount() - 1)->setData(0, QIcon(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + key.host().toUtf8().toBase64()));
      parent->child(parent->childCount() - 1)->setData(1, value);
      parent->child(parent->childCount() - 1)->setData(2, key);
    }
  }
}

QHash<QString, QHash<QUrl, QString>> TreeModel::serializeData() {
  QHash<QString, QHash<QUrl, QString>> result;
  for (int i = 0; i < rootItem->childCount(); ++i) {
    auto *parent = rootItem->child(i);
    QString parentKey = parent->data(1).toString();
    QHash<QUrl, QString> childHash;

    for (int j = 0; j < parent->childCount(); ++j) {
      auto *child = parent->child(j);
      childHash.insert(child->data(2).toUrl(), child->data(1).toString());
    }
    result.insert(parentKey, childHash);
  }
  return result;
}

Qt::DropActions TreeModel::supportedDropActions() const {
  return Qt::CopyAction | Qt::MoveAction;
}
