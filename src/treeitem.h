#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>

class TreeItem {
 public:
  explicit TreeItem(QVariantList data, TreeItem *parent = nullptr);

  TreeItem *child(int number);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  bool insertChildren(int position, int count, int columns);
  bool insertColumns(int position, int columns);
  TreeItem *parent();
  bool removeChildren(int position, int count);
  bool removeColumns(int position, int columns);
  int row() const;
  bool setData(int column, const QVariant &value);

 private:
  std::vector<std::unique_ptr<TreeItem>> m_childItems;
  QVariantList itemData;
  TreeItem *m_parentItem;
};

#endif  // TREEITEM_H
