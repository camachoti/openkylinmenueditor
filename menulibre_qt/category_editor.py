#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
CategoryEditor — replaces menulibre/CategoryEditor.py (Gtk.Box + Gtk.TreeView).

Editable table of desktop entry Categories, with dropdown suggestions.
Pure data structures (category_groups, category_lookup, category_descriptions)
are imported directly from the GTK original — they have no GTK dependency.
"""

import re
from locale import gettext as _

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QTableWidget, QTableWidgetItem,
    QToolBar, QAction, QAbstractItemView, QHeaderView, QSizePolicy,
    QComboBox, QStyledItemDelegate, QStyleOptionViewItem
)
from PyQt5.QtCore import Qt, pyqtSignal, QSize, QModelIndex
from PyQt5.QtGui import QIcon
from .icons import get_icon

# ── Re-export pure-Python data from the GTK module (no GTK imported) ──────────
from menulibre.CategoryEditor import (  # noqa: F401
    category_groups,
    category_lookup,
    category_descriptions,
    lookup_category_description,
    lookup_section_description,
    lookup_vendor_category_description,
)

COL_CATEGORY = 0   # spec name  (e.g. "WebBrowser")
COL_SECTION = 1    # group name (e.g. "Network")
COL_DESC = 2       # display name (e.g. "Web Browser")


def _all_category_choices() -> list:
    """Sorted flat list of (spec_name, section, description) for the combobox."""
    choices = []
    for section, cats in category_groups.items():
        for cat in cats:
            desc = lookup_category_description(cat)
            choices.append((cat, section, desc))
    choices.sort(key=lambda x: x[2].lower())
    return choices


_CHOICES = _all_category_choices()
_CHOICE_LABELS = [c[2] for c in _CHOICES]


class _ComboDelegate(QStyledItemDelegate):
    """Drop-down combobox delegate for the Category column."""

    def createEditor(self, parent, option, index):
        if index.column() != COL_DESC:
            return super().createEditor(parent, option, index)
        combo = QComboBox(parent)
        combo.addItem(_('Select a category'), None)
        for spec_name, section, desc in _CHOICES:
            combo.addItem(desc, (spec_name, section))
        return combo

    def setEditorData(self, editor, index):
        if index.column() != COL_DESC:
            super().setEditorData(editor, index)
            return
        current = index.data(Qt.DisplayRole)
        for i in range(editor.count()):
            if editor.itemText(i) == current:
                editor.setCurrentIndex(i)
                return

    def setModelData(self, editor, model, index):
        if index.column() != COL_DESC:
            super().setModelData(editor, model, index)
            return
        data = editor.currentData()
        if data is None:
            return
        spec_name, section = data
        desc = lookup_category_description(spec_name)
        sec_desc = lookup_section_description(section)
        # Update all three columns in the same row
        row = index.row()
        model.setItem(row, COL_CATEGORY, QTableWidgetItem(spec_name))
        model.setItem(row, COL_SECTION, QTableWidgetItem(sec_desc))
        model.setItem(row, COL_DESC, QTableWidgetItem(desc))


class CategoryEditor(QWidget):
    """Editable list of desktop file categories."""

    value_changed = pyqtSignal(str, str)   # key='Categories', value

    def __init__(self, parent=None):
        super().__init__(parent)
        self._prefix = ''
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        # ── table ────────────────────────────────────────────────────
        self._table = QTableWidget(0, 3)
        self._table.setHorizontalHeaderLabels(
            [_('Category'), _('Section'), _('Display Name')])
        self._table.horizontalHeader().setSectionResizeMode(
            COL_CATEGORY, QHeaderView.ResizeToContents)
        self._table.horizontalHeader().setSectionResizeMode(
            COL_SECTION, QHeaderView.ResizeToContents)
        self._table.horizontalHeader().setSectionResizeMode(
            COL_DESC, QHeaderView.Stretch)
        self._table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self._table.setSelectionMode(QAbstractItemView.SingleSelection)
        self._table.verticalHeader().setVisible(False)
        self._table.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self._table.setItemDelegate(_ComboDelegate(self._table))
        self._table.itemChanged.connect(self._on_item_changed)
        self._table.selectionModel().selectionChanged.connect(
            self._update_buttons)
        layout.addWidget(self._table)

        # ── toolbar ──────────────────────────────────────────────────
        tb = QToolBar()
        tb.setIconSize(QSize(16, 16))
        tb.setMovable(False)

        self._act_add = tb.addAction(get_icon('list-add'), _('Add'))
        self._act_add.triggered.connect(self._on_add)

        self._act_remove = tb.addAction(
            get_icon('list-remove'), _('Remove'))
        self._act_remove.triggered.connect(self._on_remove)
        self._act_remove.setEnabled(False)

        self._act_clear = tb.addAction(
            get_icon('edit-clear'), _('Clear'))
        self._act_clear.triggered.connect(self._on_clear)
        self._act_clear.setEnabled(False)

        self._act_restore = tb.addAction(
            get_icon('document-revert'), _('Restore Defaults'))
        self._act_restore.triggered.connect(self._on_restore_defaults)

        tb.addSeparator()

        self._act_up = tb.addAction(get_icon('go-up'), _('Move Up'))
        self._act_up.triggered.connect(self._on_move_up)
        self._act_up.setEnabled(False)

        self._act_down = tb.addAction(
            get_icon('go-down'), _('Move Down'))
        self._act_down.triggered.connect(self._on_move_down)
        self._act_down.setEnabled(False)

        layout.addWidget(tb)

        self._inhibit = False
        self._parent_directory = None

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def set_prefix(self, prefix: str):
        self._prefix = prefix or ''

    def set_value(self, value: str):
        """Load semicolon-separated categories string."""
        cats = [c for c in (value or '').split(';') if c]
        self._load_categories(cats)

    def get_value(self) -> str:
        cats = []
        for r in range(self._table.rowCount()):
            item = self._table.item(r, COL_CATEGORY)
            if item and item.text():
                cats.append(item.text())
        return ';'.join(cats) + (';' if cats else '')

    def insert_required_categories(self, parent_directory: str):
        """Insert the default categories required by the parent directory."""
        self._parent_directory = parent_directory
        if not parent_directory:
            return
        existing = {self._table.item(r, COL_CATEGORY).text()
                    for r in range(self._table.rowCount())
                    if self._table.item(r, COL_CATEGORY)}
        # parent_directory is the spec name of the folder, e.g. 'Network'
        section = parent_directory
        if section in category_groups:
            # Insert the section category itself if not present
            if section not in existing:
                self._append_category_row(section)

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _load_categories(self, cats: list):
        self._inhibit = True
        self._table.setRowCount(0)
        for cat in cats:
            self._append_category_row(cat)
        self._inhibit = False
        self._update_buttons()

    def _append_category_row(self, spec_name: str):
        r = self._table.rowCount()
        self._table.insertRow(r)

        section = category_lookup.get(spec_name, '')
        desc = lookup_category_description(spec_name)
        sec_desc = lookup_section_description(section) if section else ''

        cat_item = QTableWidgetItem(spec_name)
        cat_item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
        self._table.setItem(r, COL_CATEGORY, cat_item)

        sec_item = QTableWidgetItem(sec_desc)
        sec_item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
        self._table.setItem(r, COL_SECTION, sec_item)

        desc_item = QTableWidgetItem(desc)
        self._table.setItem(r, COL_DESC, desc_item)

    def _on_item_changed(self, item):
        if self._inhibit:
            return
        self.value_changed.emit('Categories', self.get_value())

    def _update_buttons(self, *_):
        row = self._table.currentRow()
        has_sel = row >= 0
        has_rows = self._table.rowCount() > 0
        self._act_remove.setEnabled(has_sel)
        self._act_clear.setEnabled(has_rows)
        self._act_up.setEnabled(has_sel and row > 0)
        self._act_down.setEnabled(
            has_sel and row < self._table.rowCount() - 1)

    def _on_add(self):
        r = self._table.rowCount()
        self._table.insertRow(r)
        # Placeholder row — user picks from combobox in the Desc column
        cat_item = QTableWidgetItem('')
        cat_item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
        self._table.setItem(r, COL_CATEGORY, cat_item)
        sec_item = QTableWidgetItem('')
        sec_item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
        self._table.setItem(r, COL_SECTION, sec_item)
        self._table.setItem(r, COL_DESC, QTableWidgetItem(''))
        self._table.setCurrentCell(r, COL_DESC)
        self._table.editItem(self._table.item(r, COL_DESC))

    def _on_remove(self):
        row = self._table.currentRow()
        if row >= 0:
            self._table.removeRow(row)
            self.value_changed.emit('Categories', self.get_value())

    def _on_clear(self):
        self._table.setRowCount(0)
        self.value_changed.emit('Categories', self.get_value())

    def _on_restore_defaults(self):
        if self._parent_directory:
            self.set_value('')
            self.insert_required_categories(self._parent_directory)
            self.value_changed.emit('Categories', self.get_value())

    def _on_move_up(self):
        row = self._table.currentRow()
        if row <= 0:
            return
        self._swap_rows(row, row - 1)
        self._table.setCurrentCell(row - 1, self._table.currentColumn())
        self.value_changed.emit('Categories', self.get_value())

    def _on_move_down(self):
        row = self._table.currentRow()
        if row < 0 or row >= self._table.rowCount() - 1:
            return
        self._swap_rows(row, row + 1)
        self._table.setCurrentCell(row + 1, self._table.currentColumn())
        self.value_changed.emit('Categories', self.get_value())

    def _swap_rows(self, a: int, b: int):
        self._inhibit = True
        for col in range(self._table.columnCount()):
            item_a = self._table.takeItem(a, col)
            item_b = self._table.takeItem(b, col)
            # preserve flags
            if item_a:
                item_b_flags = item_b.flags() if item_b else Qt.ItemIsEnabled
                item_a.setFlags(item_b_flags)
            if item_b:
                item_a_flags = item_a.flags() if item_a else Qt.ItemIsEnabled
                item_b.setFlags(item_a_flags)
            self._table.setItem(a, col, item_b)
            self._table.setItem(b, col, item_a)
        self._inhibit = False
