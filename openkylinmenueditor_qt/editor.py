#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
ApplicationEditor — replaces openkylinmenueditor/ApplicationEditor.py (Gtk.Box).

Right-panel property editor for a single launcher or directory entry.
Reads/writes all standard Desktop Entry spec keys via MenulibreXdg.
"""

import os
from locale import gettext as _

from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QGridLayout,
    QLabel, QLineEdit, QPushButton, QCheckBox, QTabWidget,
    QScrollArea, QFrame, QSizePolicy, QSpacerItem,
    QFileDialog, QApplication
)
from PyQt5.QtCore import Qt, pyqtSignal, QSize
from PyQt5.QtGui import QFont, QIcon
from .icons import get_icon

from .icon_entry import IconEntry
from .action_editor import ActionEditor
from .category_editor import CategoryEditor

# Advanced keys shown in the "Advanced" tab
ADVANCED_KEYS = [
    'GenericName', 'Keywords', 'MimeType', 'OnlyShowIn',
    'NotShowIn', 'StartupWMClass', 'DBusActivatable', 'TryExec',
    'Hidden', 'Version',
]


class _SectionCard(QWidget):
    """Title label above + white rounded card frame containing rows."""

    def __init__(self, title: str, parent=None):
        super().__init__(parent)
        outer = QVBoxLayout(self)
        outer.setContentsMargins(0, 0, 0, 0)
        outer.setSpacing(6)

        title_lbl = QLabel(title)
        title_lbl.setObjectName('section_title')
        outer.addWidget(title_lbl)

        self._card = QFrame()
        self._card.setObjectName('section_card')
        self._card_layout = QVBoxLayout(self._card)
        self._card_layout.setContentsMargins(0, 0, 0, 0)
        self._card_layout.setSpacing(0)
        outer.addWidget(self._card)

    def add_row(self, widget):
        if self._card_layout.count() > 0:
            sep = QFrame()
            sep.setObjectName('row_separator')
            sep.setFrameShape(QFrame.HLine)
            self._card_layout.addWidget(sep)
        self._card_layout.addWidget(widget)


class _FieldRow(QWidget):
    """Label + editor widget pair for a single desktop entry key."""

    value_changed = pyqtSignal(str, str)

    def __init__(self, label: str, key: str, tooltip: str = '',
                 multiline: bool = False, parent=None):
        super().__init__(parent)
        self._key = key
        layout = QHBoxLayout(self)
        layout.setContentsMargins(16, 10, 16, 10)

        lbl = QLabel(label)
        lbl.setMinimumWidth(150)
        lbl.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        if tooltip:
            lbl.setToolTip(tooltip)
        layout.addWidget(lbl)

        self._edit = QLineEdit()
        if tooltip:
            self._edit.setToolTip(tooltip)
        self._edit.textEdited.connect(
            lambda text: self.value_changed.emit(self._key, text))
        layout.addWidget(self._edit)

    def set_value(self, value):
        v = str(value) if value is not None else ''
        self._edit.blockSignals(True)
        self._edit.setText(v)
        self._edit.blockSignals(False)

    def get_value(self) -> str:
        return self._edit.text()


class _SwitchRow(QWidget):
    """Label + QCheckBox pair for boolean desktop entry keys."""

    value_changed = pyqtSignal(str, str)

    def __init__(self, label: str, key: str, tooltip: str = '', parent=None):
        super().__init__(parent)
        self._key = key
        layout = QHBoxLayout(self)
        layout.setContentsMargins(16, 10, 16, 10)

        lbl = QLabel(label)
        lbl.setMinimumWidth(150)
        lbl.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        if tooltip:
            lbl.setToolTip(tooltip)
        layout.addWidget(lbl)

        self._check = QCheckBox()
        if tooltip:
            self._check.setToolTip(tooltip)
        self._check.stateChanged.connect(
            lambda s: self.value_changed.emit(
                self._key, 'true' if s == Qt.Checked else 'false'))
        layout.addWidget(self._check)
        layout.addStretch()

    def set_value(self, value):
        checked = value in (True, 'true', '1', 'True')
        self._check.blockSignals(True)
        self._check.setChecked(checked)
        self._check.blockSignals(False)

    def get_value(self) -> str:
        return 'true' if self._check.isChecked() else 'false'


class ApplicationEditor(QWidget):
    """Full property editor for a desktop entry (application or directory)."""

    value_changed = pyqtSignal(str, str)   # key, value

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumWidth(400)
        self._internal = {'Type': '', 'Version': ''}
        self._directory_hide = []
        self._fields = {}   # key -> widget

        self._build_ui()

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _build_ui(self):
        outer = QVBoxLayout(self)
        outer.setContentsMargins(16, 16, 16, 16)
        outer.setSpacing(10)

        # ── header card: icon + name + comment ────────────────────────
        header = QFrame()
        header.setObjectName('editor_header')
        header.setFrameShape(QFrame.StyledPanel)
        header_layout = QHBoxLayout(header)
        header_layout.setContentsMargins(16, 16, 16, 16)
        header_layout.setSpacing(10)

        self._icon_entry = IconEntry()
        self._icon_entry.value_changed.connect(self._on_changed)
        header_layout.addWidget(self._icon_entry)

        name_box = QVBoxLayout()
        name_box.setSpacing(2)

        self._name_edit = QLineEdit()
        self._name_edit.setPlaceholderText(_('Application name'))
        font = self._name_edit.font()
        font.setBold(True)
        font.setPointSize(font.pointSize() + 2)
        self._name_edit.setFont(font)
        self._name_edit.textEdited.connect(
            lambda t: self._on_changed('Name', t))
        name_box.addWidget(self._name_edit)

        self._comment_edit = QLineEdit()
        self._comment_edit.setPlaceholderText(_('Short description'))
        self._comment_edit.textEdited.connect(
            lambda t: self._on_changed('Comment', t))
        name_box.addWidget(self._comment_edit)
        header_layout.addLayout(name_box)

        outer.addWidget(header)

        # ── scroll area for the rest ───────────────────────────────────
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QFrame.NoFrame)
        content = QWidget()
        content.setObjectName('editor_content')
        scroll_layout = QVBoxLayout(content)
        scroll_layout.setSpacing(20)
        scroll_layout.setContentsMargins(0, 8, 0, 8)
        scroll.setWidget(content)
        outer.addWidget(scroll)

        # ── Application Details ────────────────────────────────────────
        app_card = _SectionCard(_('Application Details'))

        self._exec_row = _FieldRow(
            _('Command'), 'Exec',
            _('Program to execute with arguments.'))
        self._exec_row.value_changed.connect(self._on_changed)
        self._fields['Exec'] = self._exec_row
        app_card.add_row(self._exec_row)

        self._path_row = _FieldRow(
            _('Working Directory'), 'Path',
            _('The working directory.'))
        self._path_row.value_changed.connect(self._on_changed)
        self._fields['Path'] = self._path_row
        app_card.add_row(self._path_row)

        self._directory_hide.append(app_card)
        scroll_layout.addWidget(app_card)

        # ── Options ───────────────────────────────────────────────────
        opts_card = _SectionCard(_('Options'))

        self._terminal_row = _SwitchRow(
            _('Run in terminal'), 'Terminal',
            _('Run in a terminal window.'))
        self._terminal_row.value_changed.connect(self._on_changed)
        self._fields['Terminal'] = self._terminal_row
        opts_card.add_row(self._terminal_row)

        self._notify_row = _SwitchRow(
            _('Use startup notification'), 'StartupNotify',
            _('Send a startup notification (busy cursor).'))
        self._notify_row.value_changed.connect(self._on_changed)
        self._fields['StartupNotify'] = self._notify_row
        opts_card.add_row(self._notify_row)

        self._nodisplay_row = _SwitchRow(
            _('Hide from menus'), 'NoDisplay',
            _('Do not show in application menus.'))
        self._nodisplay_row.value_changed.connect(self._on_changed)
        self._fields['NoDisplay'] = self._nodisplay_row
        opts_card.add_row(self._nodisplay_row)

        self._directory_hide.append(opts_card)
        scroll_layout.addWidget(opts_card)

        # ── Tabs: Categories / Actions / Advanced ─────────────────────
        self._tabs = QTabWidget()
        self._directory_hide.append(self._tabs)

        self._category_editor = CategoryEditor()
        self._category_editor.value_changed.connect(self._on_changed)
        self._tabs.addTab(self._category_editor, _('Categories'))

        self._action_editor = ActionEditor()
        self._action_editor.value_changed.connect(self._on_changed)
        self._tabs.addTab(self._action_editor, _('Actions'))

        adv_card = _SectionCard(_('Advanced'))
        for key in ADVANCED_KEYS:
            row = _FieldRow(key, key)
            row.value_changed.connect(self._on_changed)
            self._fields[key] = row
            adv_card.add_row(row)
        self._tabs.addTab(adv_card, _('Advanced'))

        scroll_layout.addWidget(self._tabs)

        # ── Filename bar ───────────────────────────────────────────────
        filename_bar = QFrame()
        filename_bar.setObjectName('filename_bar')
        filename_bar.setFrameShape(QFrame.StyledPanel)
        filename_layout = QHBoxLayout(filename_bar)
        filename_layout.setContentsMargins(6, 4, 6, 4)

        self._filename_label = QLabel()
        self._filename_label.setStyleSheet(
            'color: palette(mid); font-size: small;')
        self._filename_label.setTextInteractionFlags(
            Qt.TextSelectableByMouse)
        filename_layout.addWidget(self._filename_label)

        copy_btn = QPushButton(get_icon('edit-copy'), '')
        copy_btn.setToolTip(_('Copy path to clipboard'))
        copy_btn.setFlat(True)
        copy_btn.setFixedSize(24, 24)
        copy_btn.clicked.connect(
            lambda: QApplication.clipboard().setText(
                self._filename_label.text()))
        filename_layout.addWidget(copy_btn)

        outer.addWidget(filename_bar)

    # ------------------------------------------------------------------
    # Public API (mirrors GTK ApplicationEditor)
    # ------------------------------------------------------------------

    def set_value(self, key: str, value):
        """Set the desktop entry key in the editor UI."""
        if key == 'Name':
            self._name_edit.blockSignals(True)
            self._name_edit.setText(str(value or ''))
            self._name_edit.blockSignals(False)
        elif key == 'Comment':
            self._comment_edit.blockSignals(True)
            self._comment_edit.setText(str(value or ''))
            self._comment_edit.blockSignals(False)
        elif key == 'Icon':
            self._icon_entry.set_value(str(value or ''))
        elif key == 'Categories':
            self._category_editor.set_value(str(value or ''))
        elif key == 'Actions':
            self._action_editor.set_value(value)
        elif key == 'Filename':
            self._filename_label.setText(str(value or ''))
        elif key == 'Type':
            self._internal['Type'] = str(value or '')
            is_dir = str(value) == 'Directory'
            for w in self._directory_hide:
                w.setVisible(not is_dir)
        elif key in self._internal:
            self._internal[key] = str(value or '')
        elif key in self._fields:
            self._fields[key].set_value(value)

    def get_value(self, key: str):
        """Return the current UI value for the given desktop entry key."""
        if key == 'Name':
            return self._name_edit.text()
        elif key == 'Comment':
            return self._comment_edit.text()
        elif key == 'Icon':
            return self._icon_entry.get_value()
        elif key == 'Categories':
            return self._category_editor.get_value()
        elif key == 'Actions':
            return self._action_editor.get_value()
        elif key == 'Filename':
            return self._filename_label.text()
        elif key in self._internal:
            return self._internal[key]
        elif key in self._fields:
            return self._fields[key].get_value()
        return None

    def remove_incomplete_actions(self):
        self._action_editor.remove_incomplete_actions()

    def get_actions(self) -> list:
        return self._action_editor.get_actions()

    def clear_categories(self):
        self._category_editor.set_value('')

    def insert_required_categories(self, parent_directory: str):
        self._category_editor.insert_required_categories(parent_directory)

    def take_focus(self):
        self._icon_entry.grab_focus()

    # ------------------------------------------------------------------

    def _on_changed(self, key: str, value: str):
        self.value_changed.emit(key, value)
