#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""Qt dialogs — replaces menulibre/Dialogs.py (Gtk.Dialog variants)."""

from locale import gettext as _

from PyQt5.QtWidgets import (QDialog, QDialogButtonBox, QVBoxLayout,
                              QHBoxLayout, QLabel, QTextEdit, QPushButton,
                              QMessageBox)
from PyQt5.QtGui import QIcon, QPixmap
from PyQt5.QtCore import Qt

import menulibre_lib


class AboutDialog(QDialog):
    """About dialog for KylinMenuEditor."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle(_('About Menu Editor'))
        self.setMinimumWidth(400)

        layout = QVBoxLayout(self)

        # Logo
        logo_label = QLabel()
        icon = QIcon.fromTheme('kylinmenueditor',
                               QIcon.fromTheme('preferences-system-windows'))
        logo_label.setPixmap(icon.pixmap(64, 64))
        logo_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(logo_label)

        # App name + version
        name_label = QLabel(
            '<b><big>KylinMenuEditor</big></b><br/>'
            'v%s' % menulibre_lib.get_version()
        )
        name_label.setAlignment(Qt.AlignCenter)
        name_label.setTextFormat(Qt.RichText)
        layout.addWidget(name_label)

        # Copyright
        copyright_label = QLabel(
            'Copyright © 2026 Tiago Camacho - Fork MenuLibre\n'
            'Copyright © 2012-2024 Sean Davis'
        )
        copyright_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(copyright_label)

        # Website
        link = QLabel(
            '<a href="https://github.com/camachoti/kylinmenueditor">'
            'github.com/camachoti/kylinmenueditor</a>'
        )
        link.setOpenExternalLinks(True)
        link.setAlignment(Qt.AlignCenter)
        layout.addWidget(link)

        # Close button
        buttons = QDialogButtonBox(QDialogButtonBox.Close)
        buttons.rejected.connect(self.accept)
        layout.addWidget(buttons)


class SaveDialog(QMessageBox):
    """Ask whether to save, discard or cancel before closing/switching."""

    SAVE = QMessageBox.Save
    DISCARD = QMessageBox.Discard
    CANCEL = QMessageBox.Cancel

    def __init__(self, parent=None, item_name: str = ''):
        super().__init__(parent)
        self.setIcon(QMessageBox.Question)
        self.setWindowTitle(_('Save Changes?'))
        msg = _('Save changes to launcher "%s" before closing?') % item_name
        self.setText(msg)
        self.setStandardButtons(
            QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
        self.setDefaultButton(QMessageBox.Save)


class ParsingErrorsDialog(QDialog):
    """Show desktop file parsing errors."""

    def __init__(self, errors: list, parent=None):
        super().__init__(parent)
        self.setWindowTitle(_('Parsing Errors'))
        self.setMinimumSize(600, 400)

        layout = QVBoxLayout(self)

        header = QLabel(
            _('The following desktop files contain errors and will not '
              'be shown in the menu:')
        )
        header.setWordWrap(True)
        layout.addWidget(header)

        text = QTextEdit()
        text.setReadOnly(True)
        text.setPlainText('\n'.join(errors))
        layout.addWidget(text)

        buttons = QDialogButtonBox(QDialogButtonBox.Close)
        buttons.rejected.connect(self.accept)
        layout.addWidget(buttons)


def show_error(parent, title: str, message: str) -> None:
    """Convenience: show a modal error message box."""
    QMessageBox.critical(parent, title, message)


def show_info(parent, title: str, message: str) -> None:
    """Convenience: show a modal info message box."""
    QMessageBox.information(parent, title, message)
