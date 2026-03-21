#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
Main application window — replaces MenulibreApplication.py (Gtk.ApplicationWindow).

Uses PyQt5 throughout. The GMenu/XDG parsing layer (MenuEditor, MenulibreXdg)
is kept as-is since it has no GUI dependency.
"""

import os
import sys
import logging
from locale import gettext as _

from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QSplitter, QVBoxLayout,
    QHBoxLayout, QToolBar, QAction, QMenu, QMenuBar, QStatusBar,
    QLabel, QMessageBox, QSizePolicy, QScrollArea, QFrame
)
from PyQt5.QtGui import QIcon, QKeySequence
from PyQt5.QtCore import Qt, QSize, pyqtSlot

from .treeview import Treeview
from .toolbar import Toolbar
from .dialogs import AboutDialog, SaveDialog, show_error
from .file_handler import open_folder, copy_to_clipboard, open_editor

logger = logging.getLogger('menulibre')


class MainWindow(QMainWindow):
    """KylinMenuEditor main window."""

    def __init__(self):
        super().__init__()
        self.setWindowTitle('KylinMenuEditor')
        self.setWindowIcon(QIcon.fromTheme('kylinmenueditor',
                                           QIcon.fromTheme('preferences-system-windows')))
        self.resize(900, 600)

        self._current_filename: str = ''
        self._dirty: bool = False

        self._build_menubar()
        self._build_toolbar()
        self._build_central()
        self._build_statusbar()

        self._load_menu()

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _build_menubar(self):
        mb = self.menuBar()

        # File menu
        file_menu = mb.addMenu(_('&File'))

        self._act_save = file_menu.addAction(
            QIcon.fromTheme('document-save'), _('&Save'))
        self._act_save.setShortcut(QKeySequence.Save)
        self._act_save.setEnabled(False)
        self._act_save.triggered.connect(self._on_save)

        file_menu.addSeparator()

        act_quit = file_menu.addAction(
            QIcon.fromTheme('application-exit'), _('&Quit'))
        act_quit.setShortcut(QKeySequence.Quit)
        act_quit.triggered.connect(self.close)

        # Edit menu
        edit_menu = mb.addMenu(_('&Edit'))

        self._act_undo = edit_menu.addAction(
            QIcon.fromTheme('edit-undo'), _('&Undo'))
        self._act_undo.setShortcut(QKeySequence.Undo)
        self._act_undo.setEnabled(False)
        self._act_undo.triggered.connect(self._on_undo)

        self._act_redo = edit_menu.addAction(
            QIcon.fromTheme('edit-redo'), _('&Redo'))
        self._act_redo.setShortcut(QKeySequence.Redo)
        self._act_redo.setEnabled(False)
        self._act_redo.triggered.connect(self._on_redo)

        edit_menu.addSeparator()

        self._act_revert = edit_menu.addAction(
            QIcon.fromTheme('document-revert'), _('Re&vert'))
        self._act_revert.setEnabled(False)
        self._act_revert.triggered.connect(self._on_revert)

        edit_menu.addSeparator()

        self._act_delete = edit_menu.addAction(
            QIcon.fromTheme('edit-delete'), _('&Delete'))
        self._act_delete.setEnabled(False)
        self._act_delete.triggered.connect(self._on_delete)

        # Launcher menu
        launcher_menu = mb.addMenu(_('&Launcher'))

        act_add_launcher = launcher_menu.addAction(
            QIcon.fromTheme('list-add'), _('Add &Launcher'))
        act_add_launcher.triggered.connect(self._on_add_launcher)

        act_add_dir = launcher_menu.addAction(
            QIcon.fromTheme('folder-new'), _('Add &Directory'))
        act_add_dir.triggered.connect(self._on_add_directory)

        act_add_sep = launcher_menu.addAction(
            QIcon.fromTheme('list-add'), _('Add &Separator'))
        act_add_sep.triggered.connect(self._on_add_separator)

        launcher_menu.addSeparator()

        self._act_execute = launcher_menu.addAction(
            QIcon.fromTheme('media-playback-start'), _('&Execute'))
        self._act_execute.triggered.connect(self._on_execute)

        # Help menu
        help_menu = mb.addMenu(_('&Help'))
        act_about = help_menu.addAction(
            QIcon.fromTheme('help-about'), _('&About'))
        act_about.triggered.connect(self._on_about)

    def _build_toolbar(self):
        tb = self.addToolBar(_('Main Toolbar'))
        tb.setIconSize(QSize(20, 20))
        tb.setMovable(False)
        tb.setObjectName('main_toolbar')

        # Add menu button
        add_menu = QMenu(self)
        add_menu.addAction(
            QIcon.fromTheme('list-add'), _('Add Launcher'),
            self._on_add_launcher)
        add_menu.addAction(
            QIcon.fromTheme('folder-new'), _('Add Directory'),
            self._on_add_directory)
        add_menu.addAction(
            QIcon.fromTheme('list-add'), _('Add Separator'),
            self._on_add_separator)

        add_btn = tb.addAction(QIcon.fromTheme('list-add'), _('Add'))
        add_btn.setMenu(add_menu)
        add_btn.triggered.connect(lambda: add_menu.exec_(
            self.mapToGlobal(self.rect().bottomLeft())))

        tb.addSeparator()
        tb.addAction(self._act_save)
        tb.addSeparator()
        tb.addAction(self._act_undo)
        tb.addAction(self._act_redo)
        tb.addAction(self._act_revert)
        tb.addSeparator()
        tb.addAction(self._act_execute)
        tb.addAction(self._act_delete)

    def _build_central(self):
        splitter = QSplitter(Qt.Horizontal)
        splitter.setHandleWidth(1)

        # ── left: tree sidebar ───────────────────────────────────────
        self._treeview = Treeview()
        self._treeview.item_selected.connect(self._on_item_selected)
        self._treeview.add_launcher_requested.connect(self._on_add_launcher)
        self._treeview.add_directory_requested.connect(self._on_add_directory)
        self._treeview.add_separator_requested.connect(self._on_add_separator)
        self._treeview.remove_requested.connect(self._on_delete)
        self._treeview.reload_required.connect(self._on_reload_required)

        splitter.addWidget(self._treeview)

        # ── right: editor placeholder ────────────────────────────────
        self._editor_area = QScrollArea()
        self._editor_area.setWidgetResizable(True)
        self._editor_area.setFrameShape(QFrame.NoFrame)

        placeholder = QLabel(_('Select a launcher or directory to edit.'))
        placeholder.setAlignment(Qt.AlignCenter)
        placeholder.setObjectName('editor_placeholder')
        self._editor_area.setWidget(placeholder)

        splitter.addWidget(self._editor_area)
        splitter.setStretchFactor(0, 0)
        splitter.setStretchFactor(1, 1)
        splitter.setSizes([220, 680])

        self.setCentralWidget(splitter)

    def _build_statusbar(self):
        self._status = QStatusBar()
        self.setStatusBar(self._status)
        self._status_label = QLabel()
        self._status.addWidget(self._status_label)

    # ------------------------------------------------------------------
    # Menu loading
    # ------------------------------------------------------------------

    def _load_menu(self):
        self._status_label.setText(_('Loading menu…'))
        QApplication.processEvents()
        try:
            self._treeview.load_menu()
            self._status_label.setText(_('Ready'))
        except Exception as e:
            logger.error('Menu load failed: %s', e)
            self._status_label.setText(_('Failed to load menu'))
            show_error(self, _('Error'), _('Failed to load menu: %s') % e)

    # ------------------------------------------------------------------
    # Slots — item selection
    # ------------------------------------------------------------------

    @pyqtSlot(str, int)
    def _on_item_selected(self, filename: str, item_type: int):
        self._current_filename = filename
        self._act_delete.setEnabled(bool(filename))
        self._act_execute.setEnabled(bool(filename))
        self._status_label.setText(filename or _('Ready'))
        # TODO: load filename into editor panel (Phase 5)

    # ------------------------------------------------------------------
    # Slots — edit actions
    # ------------------------------------------------------------------

    def _on_save(self):
        logger.debug('save: %s', self._current_filename)
        self._dirty = False
        self._act_save.setEnabled(False)
        # TODO: persist changes via MenulibreXdg (Phase 5)

    def _on_undo(self):
        logger.debug('undo')
        # TODO: hook into MenulibreHistory (Phase 5)

    def _on_redo(self):
        logger.debug('redo')

    def _on_revert(self):
        logger.debug('revert')

    def _on_delete(self):
        if not self._current_filename:
            return
        ret = QMessageBox.question(
            self, _('Delete'),
            _('Delete "%s"?') % os.path.basename(self._current_filename),
            QMessageBox.Yes | QMessageBox.No)
        if ret == QMessageBox.Yes:
            logger.debug('delete: %s', self._current_filename)
            # TODO: implement deletion (Phase 5)

    def _on_execute(self):
        if self._current_filename:
            import subprocess
            subprocess.Popen(['xdg-open', self._current_filename])

    # ------------------------------------------------------------------
    # Slots — add items
    # ------------------------------------------------------------------

    def _on_add_launcher(self):
        logger.debug('add launcher')
        # TODO: Phase 5

    def _on_add_directory(self):
        logger.debug('add directory')

    def _on_add_separator(self):
        logger.debug('add separator')

    def _on_reload_required(self):
        self._load_menu()

    # ------------------------------------------------------------------
    # About
    # ------------------------------------------------------------------

    def _on_about(self):
        dlg = AboutDialog(self)
        dlg.exec_()

    # ------------------------------------------------------------------
    # Close event
    # ------------------------------------------------------------------

    def closeEvent(self, event):
        if self._dirty:
            dlg = SaveDialog(self, os.path.basename(self._current_filename))
            result = dlg.exec_()
            if result == SaveDialog.SAVE:
                self._on_save()
            elif result == SaveDialog.CANCEL:
                event.ignore()
                return
        event.accept()


def run():
    """Entry point — create QApplication and show the main window."""
    app = QApplication.instance() or QApplication(sys.argv)
    app.setApplicationName('KylinMenuEditor')
    app.setApplicationVersion('0.1')
    app.setOrganizationName('Tiago Camacho')

    # Apply basic stylesheet
    app.setStyleSheet(_get_stylesheet())

    window = MainWindow()
    window.show()
    return app.exec_()


def _get_stylesheet() -> str:
    return """
QMainWindow, QWidget {
    font-family: "Noto Sans", "Segoe UI", sans-serif;
}
QTreeView {
    border: none;
    background: palette(base);
}
QTreeView::item:selected {
    background: palette(highlight);
    color: palette(highlighted-text);
}
QToolBar {
    border: none;
    spacing: 2px;
}
QScrollArea {
    border: none;
}
"""
