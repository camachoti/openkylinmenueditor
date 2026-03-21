#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
IconEntry — replaces openkylinmenueditor/IconEntry.py (Gtk.MenuButton + dialogs).

A clickable icon button that opens a menu with "Browse Icons…" and
"Browse Files…" options. Shows a warning badge when the icon is missing.
"""

import os
from locale import gettext as _

from PyQt5.QtWidgets import (
    QWidget, QToolButton, QMenu, QAction, QDialog, QVBoxLayout,
    QHBoxLayout, QLineEdit, QListWidget, QListWidgetItem, QDialogButtonBox,
    QLabel, QFileDialog, QSizePolicy, QAbstractItemView
)
from PyQt5.QtCore import Qt, pyqtSignal, QSize, QThread, pyqtSlot
from PyQt5.QtGui import QIcon, QPixmap, QPainter


def _icon_exists(name: str) -> bool:
    """Return True if *name* resolves to a usable icon."""
    if name and os.path.isabs(name):
        return os.path.isfile(name)
    if not name:
        return False
    return not QIcon.fromTheme(name).isNull()


def _load_icon(name: str, size: int = 48) -> QIcon:
    if name and os.path.isabs(name) and os.path.isfile(name):
        return QIcon(name)
    if name:
        icon = QIcon.fromTheme(name)
        if not icon.isNull():
            return icon
    return QIcon.fromTheme('application-x-executable')


class _IconBrowser(QDialog):
    """Browse and search all icons available in the current theme."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle(_('Select Icon'))
        self.setMinimumSize(500, 500)
        self._selected = None

        layout = QVBoxLayout(self)

        self._search = QLineEdit()
        self._search.setPlaceholderText(_('Search icons…'))
        self._search.setClearButtonEnabled(True)
        self._search.textChanged.connect(self._filter)
        layout.addWidget(self._search)

        self._list = QListWidget()
        self._list.setViewMode(QListWidget.IconMode)
        self._list.setIconSize(QSize(32, 32))
        self._list.setResizeMode(QListWidget.Adjust)
        self._list.setSelectionMode(QAbstractItemView.SingleSelection)
        self._list.itemDoubleClicked.connect(self._accept)
        layout.addWidget(self._list)

        buttons = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self._accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

        self._all_items = []
        self._load_icons()

    def _load_icons(self):
        # Collect icon names from known categories
        from PyQt5.QtGui import QIconEnginePlugin
        import subprocess
        try:
            # Use the GTK icon theme list if available, otherwise fallback
            result = subprocess.run(
                ['find', '/usr/share/icons', '-name', '*.png',
                 '-not', '-path', '*/cursors/*'],
                capture_output=True, text=True, timeout=5)
            names = set()
            for line in result.stdout.splitlines():
                base = os.path.basename(line)
                name = os.path.splitext(base)[0]
                # strip size suffix like -16, -32
                if name.endswith(('-16', '-32', '-48', '-64', '-128', '-256')):
                    name = name.rsplit('-', 1)[0]
                names.add(name)
        except Exception:
            names = set()

        # Always add a base set of common icon names
        base_names = [
            'application-x-executable', 'applications-other',
            'text-x-generic', 'folder', 'folder-open',
            'utilities-terminal', 'system-file-manager',
            'preferences-system', 'help-browser',
            'internet-web-browser', 'multimedia-player',
            'accessories-text-editor', 'accessories-calculator',
        ]
        names.update(base_names)

        self._all_items = sorted(names)
        self._fill_list(self._all_items)

    def _fill_list(self, names: list):
        self._list.clear()
        for name in names:
            icon = QIcon.fromTheme(name)
            if icon.isNull():
                continue
            item = QListWidgetItem(icon, name)
            item.setToolTip(name)
            self._list.addItem(item)

    def _filter(self, text: str):
        if text:
            filtered = [n for n in self._all_items
                        if text.lower() in n.lower()]
        else:
            filtered = self._all_items
        self._fill_list(filtered)

    def _accept(self, *_):
        items = self._list.selectedItems()
        if items:
            self._selected = items[0].text()
            self.accept()

    def get_icon(self) -> str:
        return self._selected or ''


class IconEntry(QWidget):
    """Icon button widget — displays current icon, opens picker on click."""

    value_changed = pyqtSignal(str, str)   # key='Icon', value

    def __init__(self, parent=None):
        super().__init__(parent)
        self._value = ''

        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)

        self._btn = QToolButton()
        self._btn.setIconSize(QSize(48, 48))
        self._btn.setFixedSize(56, 56)
        self._btn.setToolTip(_('Click to change icon'))
        self._btn.setPopupMode(QToolButton.InstantPopup)
        layout.addWidget(self._btn)

        menu = QMenu(self._btn)
        menu.addAction(_('Browse Icons…'), self._browse_icons)
        menu.addAction(_('Browse Files…'), self._browse_files)
        self._btn.setMenu(menu)

        # Warning overlay label (shown when icon is missing)
        self._warning = QLabel('⚠')
        self._warning.setStyleSheet('color: orange; font-size: 12px;')
        self._warning.setFixedSize(16, 16)
        self._warning.hide()
        layout.addWidget(self._warning)

        self._set_image('applications-other')

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def set_value(self, value: str):
        self._set_image(value)

    def get_value(self) -> str:
        return self._value

    def grab_focus(self):
        self._btn.setFocus()

    # ------------------------------------------------------------------
    # Internal
    # ------------------------------------------------------------------

    def _set_image(self, icon_name: str):
        self._value = icon_name or ''

        if _icon_exists(icon_name):
            icon = _load_icon(icon_name, 48)
            self._btn.setIcon(icon)
            self._warning.hide()
        else:
            # Show fallback + warning badge
            self._btn.setIcon(QIcon.fromTheme('image-missing',
                                               QIcon.fromTheme('applications-other')))
            self._warning.show()
            self._warning.setToolTip(
                _('Icon "%s" not found in current theme') % icon_name)

        self.value_changed.emit('Icon', self._value)

    def _browse_icons(self):
        dlg = _IconBrowser(self.window())
        if dlg.exec_() == QDialog.Accepted:
            icon = dlg.get_icon()
            if icon:
                self._set_image(icon)

    def _browse_files(self):
        path, _ = QFileDialog.getOpenFileName(
            self.window(),
            _('Select Icon File'),
            os.path.expanduser('~'),
            _('Images (*.png *.svg *.xpm *.ico *.jpg *.jpeg)')
        )
        if path:
            self._set_image(path)
