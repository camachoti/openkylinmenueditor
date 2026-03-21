#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""Qt toolbar — replaces openkylinmenueditor/Toolbar.py (Gtk.Toolbar)."""

from PyQt5.QtWidgets import (QToolBar, QToolButton, QAction, QMenu,
                              QSizePolicy, QWidget)
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import Qt, QSize
from .icons import get_icon


class Toolbar(QToolBar):
    """Application toolbar with helper methods matching the GTK original."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setIconSize(QSize(20, 20))
        self.setMovable(False)
        self.setFloatable(False)

    # ------------------------------------------------------------------
    # Factory helpers (API-compatible with the GTK Toolbar class)
    # ------------------------------------------------------------------

    def add_menu_button(self, icon_name: str, label: str,
                        menu: QMenu) -> QToolButton:
        """Add a QToolButton that shows a drop-down menu."""
        btn = QToolButton(self)
        btn.setIcon(get_icon(icon_name))
        btn.setText(label)
        btn.setToolTip(label)
        btn.setMenu(menu)
        btn.setPopupMode(QToolButton.InstantPopup)
        btn.setToolButtonStyle(Qt.ToolButtonIconOnly)
        self.addWidget(btn)
        return btn

    def add_button(self, icon_name: str, label: str) -> QAction:
        """Add a simple icon button and return its QAction."""
        action = QAction(get_icon(icon_name), label, self)
        action.setToolTip(label)
        self.addAction(action)
        return action

    def add_separator(self) -> None:
        self.addSeparator()

    def add_search(self, widget: QWidget) -> None:
        """Embed an arbitrary widget (e.g. QLineEdit) in the toolbar."""
        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        self.addWidget(spacer)
        self.addWidget(widget)

    def add_linked_buttons(self, buttons: list) -> list:
        """Add a group of visually linked QActions and return them."""
        actions = []
        for icon_name, label in buttons:
            action = self.add_button(icon_name, label)
            actions.append(action)
        return actions
