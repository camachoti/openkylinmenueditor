#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
ActionEditor — replaces menulibre/ActionEditor.py (Gtk.Box + Gtk.TreeView).

Editable table of desktop entry Actions (shortcuts).
Each row: [Show (checkbox) | Internal Name | Display Name | Command]
"""

import json
from locale import gettext as _

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QTableWidget, QTableWidgetItem,
    QToolBar, QAction, QAbstractItemView, QHeaderView, QSizePolicy
)
from PyQt5.QtCore import Qt, pyqtSignal, QSize
from PyQt5.QtGui import QIcon
from .icons import get_icon

COL_SHOW = 0
COL_NAME = 1
COL_DISPLAYED = 2
COL_COMMAND = 3


class ActionEditor(QWidget):
    """Editable list of desktop file shortcut actions."""

    value_changed = pyqtSignal(str, str)   # key='Actions', value=json

    def __init__(self, parent=None):
        super().__init__(parent)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        # ── table ────────────────────────────────────────────────────
        self._table = QTableWidget(0, 4)
        self._table.setHorizontalHeaderLabels(
            [_('Show'), _('Name'), _('Display Name'), _('Command')])
        self._table.horizontalHeader().setSectionResizeMode(
            COL_SHOW, QHeaderView.ResizeToContents)
        self._table.horizontalHeader().setSectionResizeMode(
            COL_NAME, QHeaderView.ResizeToContents)
        self._table.horizontalHeader().setSectionResizeMode(
            COL_DISPLAYED, QHeaderView.Stretch)
        self._table.horizontalHeader().setSectionResizeMode(
            COL_COMMAND, QHeaderView.Stretch)
        self._table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self._table.setSelectionMode(QAbstractItemView.SingleSelection)
        self._table.verticalHeader().setVisible(False)
        self._table.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self._table.itemChanged.connect(self._on_item_changed)
        self._table.selectionModel().selectionChanged.connect(
            self._update_button_states)
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

    # ------------------------------------------------------------------
    # Public API (mirrors GTK ActionEditor)
    # ------------------------------------------------------------------

    def set_value(self, value):
        """Load actions from JSON string."""
        if value is None:
            rows = []
        else:
            try:
                rows = json.loads(value)
            except (json.JSONDecodeError, TypeError):
                rows = []
        self._load_rows(rows)

    def get_value(self) -> str:
        """Return current actions as JSON string."""
        return json.dumps(self.get_actions())

    def get_actions(self) -> list:
        rows = []
        for r in range(self._table.rowCount()):
            show_item = self._table.item(r, COL_SHOW)
            show = (show_item.checkState() == Qt.Checked) if show_item else False
            name = self._cell_text(r, COL_NAME)
            displayed = self._cell_text(r, COL_DISPLAYED)
            command = self._cell_text(r, COL_COMMAND)
            rows.append([show, name, displayed, command])
        return rows

    def remove_incomplete_actions(self):
        """Remove rows where Name or Command are empty."""
        self._inhibit = True
        rows = [row for row in self.get_actions()
                if len(row[COL_NAME]) > 0 and len(row[COL_COMMAND]) > 0]
        self._load_rows(rows)
        self._inhibit = False

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _cell_text(self, row: int, col: int) -> str:
        item = self._table.item(row, col)
        return item.text() if item else ''

    def _load_rows(self, rows: list):
        self._inhibit = True
        self._table.setRowCount(0)
        for row in rows:
            self._append_row(row[COL_SHOW], row[COL_NAME],
                             row[COL_DISPLAYED], row[COL_COMMAND])
        self._inhibit = False
        self._update_button_states()

    def _append_row(self, show: bool, name: str, displayed: str, command: str):
        r = self._table.rowCount()
        self._table.insertRow(r)

        show_item = QTableWidgetItem()
        show_item.setFlags(Qt.ItemIsUserCheckable | Qt.ItemIsEnabled |
                           Qt.ItemIsSelectable)
        show_item.setCheckState(Qt.Checked if show else Qt.Unchecked)
        self._table.setItem(r, COL_SHOW, show_item)

        # Internal name (read-only — editing it via the table could break
        # existing .desktop file references)
        name_item = QTableWidgetItem(name)
        name_item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
        self._table.setItem(r, COL_NAME, name_item)

        self._table.setItem(r, COL_DISPLAYED, QTableWidgetItem(displayed))
        self._table.setItem(r, COL_COMMAND, QTableWidgetItem(command))

    def _on_item_changed(self, item):
        if self._inhibit:
            return
        self.value_changed.emit('Actions', self.get_value())

    def _update_button_states(self, *_):
        row = self._table.currentRow()
        has_sel = row >= 0
        has_rows = self._table.rowCount() > 0
        self._act_remove.setEnabled(has_sel)
        self._act_clear.setEnabled(has_rows)
        self._act_up.setEnabled(has_sel and row > 0)
        self._act_down.setEnabled(
            has_sel and row < self._table.rowCount() - 1)

    def _on_add(self):
        existing = {self._cell_text(r, COL_NAME)
                    for r in range(self._table.rowCount())}
        name = 'NewShortcut'
        n = 1
        while name in existing:
            name = 'NewShortcut%i' % n
            n += 1
        self._append_row(True, name, _('New Shortcut'), '')
        new_row = self._table.rowCount() - 1
        self._table.setCurrentCell(new_row, COL_DISPLAYED)
        self._table.editItem(self._table.item(new_row, COL_DISPLAYED))
        self.value_changed.emit('Actions', self.get_value())

    def _on_remove(self):
        row = self._table.currentRow()
        if row >= 0:
            self._table.removeRow(row)
            self.value_changed.emit('Actions', self.get_value())

    def _on_clear(self):
        self._table.setRowCount(0)
        self.value_changed.emit('Actions', self.get_value())

    def _on_move_up(self):
        row = self._table.currentRow()
        if row <= 0:
            return
        self._swap_rows(row, row - 1)
        self._table.setCurrentCell(row - 1, self._table.currentColumn())
        self.value_changed.emit('Actions', self.get_value())

    def _on_move_down(self):
        row = self._table.currentRow()
        if row < 0 or row >= self._table.rowCount() - 1:
            return
        self._swap_rows(row, row + 1)
        self._table.setCurrentCell(row + 1, self._table.currentColumn())
        self.value_changed.emit('Actions', self.get_value())

    def _swap_rows(self, a: int, b: int):
        self._inhibit = True
        for col in range(self._table.columnCount()):
            item_a = self._table.takeItem(a, col)
            item_b = self._table.takeItem(b, col)
            self._table.setItem(a, col, item_b)
            self._table.setItem(b, col, item_a)
        self._inhibit = False
