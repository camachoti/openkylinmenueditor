#include "menutreewidget.h"
#include "../backend/menuparser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStandardItem>
#include <QItemSelectionModel>
#include <QAction>
#include <QModelIndex>
#include <QHeaderView>

// ── MenuTreeWidget ────────────────────────────────────────────────────────────

MenuTreeWidget::MenuTreeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(220);
    buildUi();
}

void MenuTreeWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── search bar ──────────────────────────────────────────────────────
    m_search = new SearchBar(this);
    m_search->setObjectName(QStringLiteral("sidebar_search"));
    m_search->setPlaceholderText(tr("Search"));
    m_search->setToolTip(tr("Input the search key of files you would like to find."));
    m_search->setFixedHeight(36);
    m_search->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_search, &SearchBar::textChanged, this, &MenuTreeWidget::onSearchChanged);
    layout->addWidget(m_search);

    // ── proxy model for filtering ────────────────────────────────────────
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setRecursiveFilteringEnabled(true);
    m_proxy->setFilterRole(Qt::DisplayRole);

    // ── tree view ────────────────────────────────────────────────────────
    m_tree = new QTreeView(this);
    m_tree->setObjectName(QStringLiteral("sidebar_tree"));
    m_tree->setHeaderHidden(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setDragDropMode(QAbstractItemView::InternalMove);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree->setModel(m_proxy);
    connect(m_tree->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MenuTreeWidget::onSelectionChanged);
    layout->addWidget(m_tree, 1);

    // ── bottom toolbar ───────────────────────────────────────────────────
    m_toolbar = new QToolBar(this);
    m_toolbar->setObjectName(QStringLiteral("tree_toolbar"));
    m_toolbar->setIconSize(QSize(16, 16));
    m_toolbar->setMovable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    auto *actAddLauncher = m_toolbar->addAction(QIcon(":/icons/document-plus.svg"), "");
    actAddLauncher->setToolTip(tr("Add Launcher"));
    auto *actAddDir = m_toolbar->addAction(QIcon(":/icons/folder-plus.svg"), "");
    actAddDir->setToolTip(tr("Add Directory"));
    auto *actAddSep = m_toolbar->addAction(QIcon(":/icons/plus.svg"), "");
    actAddSep->setToolTip(tr("Add Separator"));
    m_toolbar->addSeparator();
    auto *actRemove = m_toolbar->addAction(QIcon(":/icons/minus.svg"), "");
    actRemove->setToolTip(tr("Remove"));
    m_toolbar->addSeparator();
    auto *actUp = m_toolbar->addAction(QIcon(":/icons/chevron-up.svg"), "");
    actUp->setToolTip(tr("Move Up"));
    auto *actDown = m_toolbar->addAction(QIcon(":/icons/chevron-down.svg"), "");
    actDown->setToolTip(tr("Move Down"));

    connect(actAddLauncher, &QAction::triggered,
            this, &MenuTreeWidget::addLauncherRequested);
    connect(actAddDir, &QAction::triggered,
            this, &MenuTreeWidget::addDirectoryRequested);
    connect(actAddSep, &QAction::triggered,
            this, &MenuTreeWidget::addSeparatorRequested);
    connect(actRemove, &QAction::triggered,
            this, &MenuTreeWidget::removeRequested);
    connect(actUp,   &QAction::triggered, this, &MenuTreeWidget::onMoveUp);
    connect(actDown, &QAction::triggered, this, &MenuTreeWidget::onMoveDown);

    layout->addWidget(m_toolbar);
}

void MenuTreeWidget::setMenuModel(QStandardItemModel *model)
{
    if (!model) return;
    // Check for a spurious root-item like kf5-applications.menu
    QStandardItem *root = model->invisibleRootItem();
    if (root->rowCount() == 1) {
        QStandardItem *pseudoRoot = root->child(0);
        // Heuristic: filename or text endsWith '.menu'
        QVariant filename = pseudoRoot->data(RoleFilename);
        QString name = pseudoRoot->text();
        if ((filename.isValid() && filename.toString().endsWith(".menu")) ||
            name.endsWith(".menu")) {
            // Move children of the menu node to the real root
            QList<QStandardItem*> children;
            while (pseudoRoot->rowCount() > 0) {
                children << pseudoRoot->takeRow(0).first();
            }
            for (QStandardItem* child : children) {
                root->appendRow(child);
            }
            // Remove the pseudo-root itself
            root->removeRow(0);
        }
    }
    m_model = model;
    m_proxy->setSourceModel(model);
    m_tree->expandAll();
}

QString MenuTreeWidget::selectedFilename() const
{
    QModelIndex idx = m_tree->currentIndex();
    if (!idx.isValid()) return {};
    QModelIndex src = m_proxy->mapToSource(idx);
    if (!src.isValid()) return {};
    auto *item = m_model ? m_model->itemFromIndex(src) : nullptr;
    if (!item) return {};
    return item->data(RoleFilename).toString();
}

int MenuTreeWidget::selectedItemType() const
{
    QModelIndex idx = m_tree->currentIndex();
    if (!idx.isValid()) return -1;
    QModelIndex src = m_proxy->mapToSource(idx);
    if (!src.isValid() || !m_model) return -1;
    auto *item = m_model->itemFromIndex(src);
    if (!item) return -1;
    return item->data(RoleType).toInt();
}

QModelIndex MenuTreeWidget::currentProxyIndex() const
{
    return m_tree->currentIndex();
}

QModelIndex MenuTreeWidget::proxyToSource(const QModelIndex &proxyIdx) const
{
    return m_proxy->mapToSource(proxyIdx);
}

void MenuTreeWidget::expandAll()
{
    m_tree->expandAll();
}

void MenuTreeWidget::selectByFilename(const QString &filePath)
{
    if (!m_model) return;

    std::function<QModelIndex(QStandardItem*)> findItem =
        [&](QStandardItem *parent) -> QModelIndex {
        for (int r = 0; r < parent->rowCount(); ++r) {
            QStandardItem *child = parent->child(r);
            if (child->data(RoleFilename).toString() == filePath)
                return child->index();
            QModelIndex found = findItem(child);
            if (found.isValid()) return found;
        }
        return {};
    };

    QModelIndex srcIdx = findItem(m_model->invisibleRootItem());
    if (!srcIdx.isValid()) return;
    QModelIndex proxyIdx = m_proxy->mapFromSource(srcIdx);
    m_tree->setCurrentIndex(proxyIdx);
    m_tree->scrollTo(proxyIdx);
}

void MenuTreeWidget::onSearchChanged(const QString &text)
{
    m_proxy->setFilterFixedString(text);
    if (!text.isEmpty())
        m_tree->expandAll();
}

void MenuTreeWidget::onSelectionChanged()
{
    QModelIndex idx = m_tree->currentIndex();
    if (!idx.isValid()) { emit itemSelected({}, -1); return; }
    QModelIndex src = m_proxy->mapToSource(idx);
    if (!src.isValid() || !m_model) { emit itemSelected({}, -1); return; }
    auto *item = m_model->itemFromIndex(src);
    if (!item) { emit itemSelected({}, -1); return; }
    QString filename = item->data(RoleFilename).toString();
    int type = item->data(RoleType).toInt();
    emit itemSelected(filename, type);
}

void MenuTreeWidget::onMoveUp()
{
    QModelIndex proxyIdx = m_tree->currentIndex();
    if (!proxyIdx.isValid() || proxyIdx.row() == 0) return;
    QModelIndex srcIdx = m_proxy->mapToSource(proxyIdx);
    auto *parent = srcIdx.parent().isValid()
        ? m_model->itemFromIndex(srcIdx.parent())
        : m_model->invisibleRootItem();
    int row = srcIdx.row();
    QList<QStandardItem*> taken = parent->takeRow(row);
    parent->insertRow(row - 1, taken);
    QModelIndex newProxy = m_proxy->mapFromSource(
        m_model->index(row - 1, 0, srcIdx.parent()));
    m_tree->setCurrentIndex(newProxy);
    emit reloadRequired();
}

void MenuTreeWidget::onMoveDown()
{
    QModelIndex proxyIdx = m_tree->currentIndex();
    if (!proxyIdx.isValid()) return;
    QModelIndex srcIdx = m_proxy->mapToSource(proxyIdx);
    auto *parent = srcIdx.parent().isValid()
        ? m_model->itemFromIndex(srcIdx.parent())
        : m_model->invisibleRootItem();
    int row = srcIdx.row();
    if (row >= parent->rowCount() - 1) return;
    QList<QStandardItem*> taken = parent->takeRow(row);
    parent->insertRow(row + 1, taken);
    QModelIndex newProxy = m_proxy->mapFromSource(
        m_model->index(row + 1, 0, srcIdx.parent()));
    m_tree->setCurrentIndex(newProxy);
    emit reloadRequired();
}
