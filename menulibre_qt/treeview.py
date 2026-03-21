#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
Sidebar tree view — replaces menulibre/MenulibreTreeview.py (Gtk.TreeView).

Shows the application menu hierarchy with folder expand/collapse,
add/remove/move/sort toolbar, and a "Launchers Only" flat-list toggle.
"""

import logging
from locale import gettext as _

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QTreeView, QToolBar, QAction,
    QAbstractItemView, QSizePolicy, QLineEdit, QToolButton, QSizePolicy
)
from PyQt5.QtGui import QIcon, QStandardItemModel, QStandardItem
from PyQt5.QtCore import (
    Qt, pyqtSignal, QSortFilterProxyModel, QModelIndex, QSize
)
from .icons import get_icon

from .menu_model import (
    ROLE_TYPE, ROLE_FILENAME, ROLE_SHOW, ROLE_ICON_NAME,
    get_model
)

logger = logging.getLogger('menulibre')


class _SearchProxy(QSortFilterProxyModel):
    """Case-insensitive name filter proxy."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setFilterCaseSensitivity(Qt.CaseInsensitive)
        self.setRecursiveFilteringEnabled(True)
        self.setFilterRole(Qt.DisplayRole)


class Treeview(QWidget):
    """
    Left-panel menu tree widget.

    Signals
    -------
    item_selected(filename, item_type)  — emitted when the user picks a row
    add_launcher_requested()            — "+" button
    add_directory_requested()           — folder "+" button
    add_separator_requested()           — separator "+" button
    remove_requested()                  — "-" button
    reload_required()                   — menu needs reloading from disk
    """

    item_selected = pyqtSignal(str, int)   # filename, item_type
    add_launcher_requested = pyqtSignal()
    add_directory_requested = pyqtSignal()
    add_separator_requested = pyqtSignal()
    remove_requested = pyqtSignal()
    reload_required = pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumWidth(220)
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)

        self._model: QStandardItemModel | None = None
        self._proxy = _SearchProxy(self)
        self._launchers_only = False

        self._build_ui()

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _build_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        # ── search bar ──────────────────────────────────────────────
        self._search = QLineEdit()
        self._search.setPlaceholderText(_('Search…'))
        self._search.setClearButtonEnabled(True)
        self._search.textChanged.connect(self._on_search_changed)
        layout.addWidget(self._search)

        # ── tree view ────────────────────────────────────────────────
        self._tree = QTreeView()
        self._tree.setHeaderHidden(True)
        self._tree.setSelectionMode(QAbstractItemView.SingleSelection)
        self._tree.setDragDropMode(QAbstractItemView.InternalMove)
        self._tree.setAnimated(True)
        self._tree.setIconSize(QSize(16, 16))
        self._tree.selectionModel  # lazy; will be set after model
        layout.addWidget(self._tree)

        # ── bottom toolbar ────────────────────────────────────────────
        self._toolbar = QToolBar()
        self._toolbar.setIconSize(QSize(16, 16))
        self._toolbar.setMovable(False)

        self._act_add_launcher = self._toolbar.addAction(
            get_icon('list-add'), _('Add Launcher'))
        self._act_add_launcher.triggered.connect(self.add_launcher_requested)

        self._act_add_dir = self._toolbar.addAction(
            get_icon('folder-new'), _('Add Directory'))
        self._act_add_dir.triggered.connect(self.add_directory_requested)

        self._act_add_sep = self._toolbar.addAction(
            get_icon('list-add'), _('Add Separator'))
        self._act_add_sep.triggered.connect(self.add_separator_requested)

        self._act_remove = self._toolbar.addAction(
            get_icon('list-remove'), _('Remove'))
        self._act_remove.triggered.connect(self.remove_requested)

        self._toolbar.addSeparator()

        self._act_move_up = self._toolbar.addAction(
            get_icon('go-up'), _('Move Up'))
        self._act_move_up.triggered.connect(self._move_up)

        self._act_move_down = self._toolbar.addAction(
            get_icon('go-down'), _('Move Down'))
        self._act_move_down.triggered.connect(self._move_down)

        self._act_sort = self._toolbar.addAction(
            get_icon('view-sort-ascending'), _('Sort Alphabetically'))
        self._act_sort.triggered.connect(self._sort_children)

        self._toolbar.addSeparator()

        self._act_launchers_only = QAction(
            get_icon('view-list-symbolic'), _('Show Launchers Only'),
            self._toolbar)
        self._act_launchers_only.setCheckable(True)
        self._act_launchers_only.toggled.connect(self._on_launchers_only_toggled)
        self._toolbar.addAction(self._act_launchers_only)

        layout.addWidget(self._toolbar)

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def load_menu(self):
        """Load (or reload) the system menu into the model."""
        self._model = get_model()
        self._proxy.setSourceModel(self._model)
        self._tree.setModel(self._proxy)
        self._tree.selectionModel().selectionChanged.connect(
            self._on_selection_changed)
        self._tree.expandToDepth(0)

    def set_move_up_enabled(self, enabled: bool):
        self._act_move_up.setEnabled(enabled)

    def set_move_down_enabled(self, enabled: bool):
        self._act_move_down.setEnabled(enabled)

    def get_selected_filename(self) -> str:
        idx = self._current_source_index()
        if not idx.isValid():
            return ''
        item = self._model.itemFromIndex(idx)
        return item.data(ROLE_FILENAME) if item else ''

    def get_selected_type(self) -> int:
        idx = self._current_source_index()
        if not idx.isValid():
            return -1
        item = self._model.itemFromIndex(idx)
        return item.data(ROLE_TYPE) if item else -1

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _current_source_index(self) -> QModelIndex:
        proxy_idx = self._tree.currentIndex()
        if not proxy_idx.isValid():
            return QModelIndex()
        return self._proxy.mapToSource(proxy_idx)

    def _on_selection_changed(self, selected, _deselected):
        indexes = selected.indexes()
        if not indexes:
            return
        src_idx = self._proxy.mapToSource(indexes[0])
        item = self._model.itemFromIndex(src_idx)
        if item is None:
            return
        filename = item.data(ROLE_FILENAME) or ''
        item_type = item.data(ROLE_TYPE) or -1
        self.item_selected.emit(filename, item_type)

    def _on_search_changed(self, text: str):
        self._proxy.setFilterFixedString(text)
        if text:
            self._tree.expandAll()

    def _on_launchers_only_toggled(self, checked: bool):
        self._launchers_only = checked
        if checked:
            self._enable_launchers_only()
        else:
            self._disable_launchers_only()

    def _enable_launchers_only(self):
        """Show only APPLICATION/LINK items in a flat list."""
        from menulibre.util import MenuItemTypes  # type: ignore

        flat = QStandardItemModel()
        flat.setHorizontalHeaderLabels(['Name'])

        def _collect(parent_item: QStandardItem):
            for row in range(parent_item.rowCount()):
                child = parent_item.child(row)
                if child is None:
                    continue
                t = child.data(ROLE_TYPE)
                if t in (MenuItemTypes.APPLICATION,  # type: ignore
                         MenuItemTypes.LINK):  # type: ignore
                    clone = child.clone()
                    flat.invisibleRootItem().appendRow(clone)
                elif t == MenuItemTypes.DIRECTORY:  # type: ignore
                    _collect(child)

        if self._model:
            _collect(self._model.invisibleRootItem())

        self._proxy.setSourceModel(flat)
        self._tree.setRootIsDecorated(False)
        self._act_move_up.setEnabled(False)
        self._act_move_down.setEnabled(False)
        self._act_sort.setEnabled(False)

    def _disable_launchers_only(self):
        """Restore the full tree."""
        if self._model:
            self._proxy.setSourceModel(self._model)
        self._tree.setRootIsDecorated(True)
        self._tree.expandToDepth(0)
        self._act_move_up.setEnabled(True)
        self._act_move_down.setEnabled(True)
        self._act_sort.setEnabled(True)

    def _get_current_item(self):
        idx = self._current_source_index()
        if not idx.isValid() or self._model is None:
            return None, None
        item = self._model.itemFromIndex(idx)
        return item, idx

    def _move_up(self):
        item, idx = self._get_current_item()
        if item is None:
            return
        parent = item.parent() or self._model.invisibleRootItem()
        row = item.row()
        if row == 0:
            return
        taken = parent.takeRow(row)
        parent.insertRow(row - 1, taken)
        new_idx = self._proxy.mapFromSource(
            self._model.indexFromItem(parent.child(row - 1)))
        self._tree.setCurrentIndex(new_idx)
        self.reload_required.emit()

    def _move_down(self):
        item, idx = self._get_current_item()
        if item is None:
            return
        parent = item.parent() or self._model.invisibleRootItem()
        row = item.row()
        if row >= parent.rowCount() - 1:
            return
        taken = parent.takeRow(row)
        parent.insertRow(row + 1, taken)
        new_idx = self._proxy.mapFromSource(
            self._model.indexFromItem(parent.child(row + 1)))
        self._tree.setCurrentIndex(new_idx)
        self.reload_required.emit()

    def _sort_children(self):
        item, idx = self._get_current_item()
        if item is None:
            return
        parent = item.parent() or self._model.invisibleRootItem()
        # Collect and sort by display text
        rows = []
        for r in range(parent.rowCount()):
            rows.append(parent.takeRow(0))
        rows.sort(key=lambda r: r[0].text().lower())
        for r in rows:
            parent.appendRow(r)
        self.reload_required.emit()
