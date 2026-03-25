#pragma once
#include <QWidget>
#include <QTreeView>
#include "searchbar.h"
#include <QToolBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

/**
 * MenuTreeWidget — left-panel menu tree with search bar and edit toolbar.
 *
 * Mirrors openkylinmenueditor_qt/treeview.py: class Treeview.
 *
 * Signals
 * -------
 * itemSelected(filename, itemType) — user selected a row
 * addLauncherRequested()           — "+" toolbar button
 * addDirectoryRequested()          — folder-add toolbar button
 * addSeparatorRequested()          — separator toolbar button
 * removeRequested()                — "-" toolbar button
 * moveUpRequested()                — "up" toolbar button
 * moveDownRequested()              — "down" toolbar button
 * reloadRequired()                 — menu was modified externally
 */
class MenuTreeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MenuTreeWidget(QWidget *parent = nullptr);

    /** Replace the current model (takes ownership). */
    void setMenuModel(QStandardItemModel *model);

    QStandardItemModel *menuModel() const { return m_model; }

    /** Return the filename stored in the currently selected item, or empty. */
    QString selectedFilename() const;

    /** Return the MenuItemType of the currently selected item, or -1. */
    int selectedItemType() const;

    /** Return the proxy index of the current item. */
    QModelIndex currentProxyIndex() const;

    /** Map a proxy index to its source index. */
    QModelIndex proxyToSource(const QModelIndex &proxyIdx) const;

    /** Expand all nodes in the tree view. */
    void expandAll();

    /** Select the item whose RoleFilename matches @p filePath. */
    void selectByFilename(const QString &filePath);

signals:
    void itemSelected(const QString &filename, int itemType);
    void addLauncherRequested();
    void addDirectoryRequested();
    void addSeparatorRequested();
    void removeRequested();
    void moveUpRequested();
    void moveDownRequested();
    void reloadRequired();

private slots:
    void onSearchChanged(const QString &text);
    void onSelectionChanged();
    void onMoveUp();
    void onMoveDown();

private:
    void buildUi();

    QStandardItemModel       *m_model  = nullptr;
    QSortFilterProxyModel    *m_proxy  = nullptr;
    QTreeView                *m_tree   = nullptr;
    SearchBar                *m_search = nullptr;
    QToolBar                 *m_toolbar = nullptr;
};
