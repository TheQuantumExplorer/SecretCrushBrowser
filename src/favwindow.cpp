#include "favwindow.h"
#include "treemodel.h"

using namespace Qt::StringLiterals;

FavWindow::FavWindow(const QHash<QString, QHash<QUrl, QString>> &data, QWidget *parent)
    : QMainWindow(parent) {
  setupUi(this);
  this->setWindowModality(Qt::ApplicationModal);
  this->resize(parent->size() * 0.75);

  const QStringList headers({tr("Icon"), tr("Description"), tr("URL")});

  auto *model = new TreeModel(headers, data, this);
  view->setModel(model);
  for (int column = 0; column < model->columnCount(); ++column)
    view->resizeColumnToContents(column);
  view->expandAll();
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  view->setDragEnabled(true);
  view->viewport()->setAcceptDrops(true);
  view->setDropIndicatorShown(true);
  view->setDragDropMode(QAbstractItemView::InternalMove);
  view->resizeColumnToContents(0);
  view->resizeColumnToContents(1);

  connect(view, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
    if (index.isValid() && index.column() == 2) {
      QUrl url = index.data().toUrl();
      emit(loadFav(url));
    }
  });

  connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &FavWindow::updateActions);

  connect(actionsMenu, &QMenu::aboutToShow, this, &FavWindow::updateActions);
  connect(insertRowAction, &QAction::triggered, this, &FavWindow::insertFav);
  connect(removeRowAction, &QAction::triggered, this, &FavWindow::removeRow);
  connect(insertChildAction, &QAction::triggered, this, &FavWindow::insertCategory);

  updateActions();
}

QHash<QString, QHash<QUrl, QString>> FavWindow::getData() {
  TreeModel *model = qobject_cast<TreeModel *>(view->model());
  return model->serializeData();
}

void FavWindow::insertCategory() {
  // For the moment, sub category are not supported due to export in json file
  // The model should be ready except that it cannot accept more than 3 sub levels TOFIX
  // const QModelIndex parent = view->selectionModel()->currentIndex().siblingAtColumn(0);
  const QModelIndex parent = view->model()->index(-1, 0);

  QAbstractItemModel *model = view->model();

  // If the parent is a favorite
  if (!parent.siblingAtColumn(2).data().isNull()) {
    return;
  }

  if (!model->insertRow(0, parent))
    return;

  model->setData(model->index(0, 1, parent), QVariant("Category Label"), Qt::EditRole);

  updateActions();
}

void FavWindow::insertFav() {
  // Insert Fav
  QModelIndex index = view->selectionModel()->currentIndex().siblingAtColumn(0);
  QAbstractItemModel *model = view->model();

  // If the parent is a favorite select it category
  if (!index.siblingAtColumn(2).data().isNull()) {
    index = index.parent();
  }

  if (!model->insertRow(0, index))
    return;

  updateActions();

  model->setData(model->index(0, 0, index), QVariant(currentIcon), Qt::DecorationRole);
  model->setData(model->index(0, 1, index), QVariant("Label"), Qt::EditRole);
  model->setData(model->index(0, 2, index), QVariant(currentUrl), Qt::DisplayRole);
}

void FavWindow::removeRow() {
  const QModelIndex index = view->selectionModel()->currentIndex();
  QAbstractItemModel *model = view->model();
  if (model->removeRow(index.row(), index.parent()))
    updateActions();
}

void FavWindow::updateActions() {
  const bool hasSelection = !view->selectionModel()->selection().isEmpty();
  removeRowAction->setEnabled(hasSelection);
  removeColumnAction->setEnabled(hasSelection);

  const bool hasCurrent = view->selectionModel()->currentIndex().isValid();
  insertRowAction->setEnabled(hasCurrent);
  insertColumnAction->setEnabled(hasCurrent);

  if (hasCurrent) {
    view->closePersistentEditor(view->selectionModel()->currentIndex());
  }
}

void FavWindow::contextMenuEvent(QContextMenuEvent *event) {
  QMenu contextMenu(this);
  contextMenu.addAction(insertChildAction);
  contextMenu.addAction(insertRowAction);
  contextMenu.addAction(insertColumnAction);
  contextMenu.addAction(removeRowAction);
  contextMenu.addAction(removeColumnAction);
  contextMenu.exec(event->globalPos());
}

void FavWindow::setCurrentUrl(const QUrl &url) {
  currentUrl = url;
}

void FavWindow::setCurrentIcon(const QIcon &icon) {
  currentIcon = icon;
}

void FavWindow::hideEvent(QHideEvent *event) {
  emit(visibilityChanged(false));
}
