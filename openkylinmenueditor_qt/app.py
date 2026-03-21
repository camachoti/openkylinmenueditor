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
from .icons import get_icon

from .treeview import Treeview
from .toolbar import Toolbar
from .dialogs import AboutDialog, SaveDialog, show_error
from .file_handler import open_folder, copy_to_clipboard, open_editor
from .editor import ApplicationEditor
from .history import History

logger = logging.getLogger('openkylinmenueditor')


class MainWindow(QMainWindow):
    """OpenKylinMenuEditor main window."""

    def __init__(self):
        super().__init__()
        self.setWindowTitle('OpenKylinMenuEditor')
        self.setWindowIcon(get_icon('OpenKylinMenuEditor'))
        self.resize(1000, 640)

        self._current_filename: str = ''
        self._dirty: bool = False

        self._history = History(self)

        self._build_menubar()
        self._build_toolbar()
        self._build_central()
        self._build_statusbar()

        # Wire history signals now that actions exist
        self._history.undo_changed.connect(self._act_undo.setEnabled)
        self._history.redo_changed.connect(self._act_redo.setEnabled)
        self._history.revert_changed.connect(self._act_revert.setEnabled)

        self._load_menu()

    # ------------------------------------------------------------------
    # UI construction
    # ------------------------------------------------------------------

    def _build_menubar(self):
        mb = self.menuBar()

        # File menu
        file_menu = mb.addMenu(_('&File'))

        self._act_save = file_menu.addAction(
            get_icon('document-save'), _('&Save'))
        self._act_save.setShortcut(QKeySequence.Save)
        self._act_save.setEnabled(False)
        self._act_save.triggered.connect(self._on_save)

        file_menu.addSeparator()

        act_quit = file_menu.addAction(
            get_icon('application-exit'), _('&Quit'))
        act_quit.setShortcut(QKeySequence.Quit)
        act_quit.triggered.connect(self.close)

        # Edit menu
        edit_menu = mb.addMenu(_('&Edit'))

        self._act_undo = edit_menu.addAction(
            get_icon('edit-undo'), _('&Undo'))
        self._act_undo.setShortcut(QKeySequence.Undo)
        self._act_undo.setEnabled(False)
        self._act_undo.triggered.connect(self._on_undo)

        self._act_redo = edit_menu.addAction(
            get_icon('edit-redo'), _('&Redo'))
        self._act_redo.setShortcut(QKeySequence.Redo)
        self._act_redo.setEnabled(False)
        self._act_redo.triggered.connect(self._on_redo)

        edit_menu.addSeparator()

        self._act_revert = edit_menu.addAction(
            get_icon('document-revert'), _('Re&vert'))
        self._act_revert.setEnabled(False)
        self._act_revert.triggered.connect(self._on_revert)

        edit_menu.addSeparator()

        self._act_delete = edit_menu.addAction(
            get_icon('edit-delete'), _('&Delete'))
        self._act_delete.setEnabled(False)
        self._act_delete.triggered.connect(self._on_delete)

        # Launcher menu
        launcher_menu = mb.addMenu(_('&Launcher'))

        act_add_launcher = launcher_menu.addAction(
            get_icon('list-add'), _('Add &Launcher'))
        act_add_launcher.triggered.connect(self._on_add_launcher)

        act_add_dir = launcher_menu.addAction(
            get_icon('folder-new'), _('Add &Directory'))
        act_add_dir.triggered.connect(self._on_add_directory)

        act_add_sep = launcher_menu.addAction(
            get_icon('list-add'), _('Add &Separator'))
        act_add_sep.triggered.connect(self._on_add_separator)

        launcher_menu.addSeparator()

        self._act_execute = launcher_menu.addAction(
            get_icon('media-playback-start'), _('&Execute'))
        self._act_execute.triggered.connect(self._on_execute)

        # Help menu
        help_menu = mb.addMenu(_('&Help'))
        act_about = help_menu.addAction(
            get_icon('help-about'), _('&About'))
        act_about.triggered.connect(self._on_about)

    def _build_toolbar(self):
        tb = self.addToolBar(_('Main Toolbar'))
        tb.setIconSize(QSize(20, 20))
        tb.setMovable(False)
        tb.setObjectName('main_toolbar')

        # Add menu button
        add_menu = QMenu(self)
        add_menu.addAction(
            get_icon('list-add'), _('Add Launcher'),
            self._on_add_launcher)
        add_menu.addAction(
            get_icon('folder-new'), _('Add Directory'),
            self._on_add_directory)
        add_menu.addAction(
            get_icon('list-add'), _('Add Separator'),
            self._on_add_separator)

        add_btn = tb.addAction(get_icon('list-add'), _('Add'))
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

        # ── right: editor panel ──────────────────────────────────────
        self._editor = ApplicationEditor()
        self._editor.value_changed.connect(self._on_value_changed)

        self._editor_area = QScrollArea()
        self._editor_area.setWidgetResizable(True)
        self._editor_area.setFrameShape(QFrame.NoFrame)
        self._editor_area.setWidget(self._editor)

        splitter.addWidget(self._editor_area)
        splitter.setStretchFactor(0, 0)
        splitter.setStretchFactor(1, 1)
        splitter.setSizes([240, 760])

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

        if not filename:
            return

        # Load the desktop entry into the editor
        self._load_entry(filename, item_type)

    def _load_entry(self, filename: str, item_type: int):
        """Read filename and populate the editor panel."""
        try:
            from openkylinmenueditor.MenulibreXdg import MenulibreDesktopEntry
            from openkylinmenueditor.util import MenuItemKeys

            entry = MenulibreDesktopEntry(filename)
            self._history.clear()
            self._history.block()

            self._editor.set_value('Filename', filename)
            self._editor.set_value('Type', entry['Type'] or 'Application')

            for key_tuple in MenuItemKeys:
                key = key_tuple[0]  # MenuItemKeys entries are (name, type, required, item_types)
                try:
                    if key == 'Actions':
                        val = entry.get_actions()
                    else:
                        val = entry[key]
                    self._editor.set_value(key, val)
                    self._history.store(key, str(val) if val is not None else '')
                except Exception:
                    pass

            self._history.unblock()
            self._dirty = False
            self._act_save.setEnabled(False)
            self._act_revert.setEnabled(False)

        except Exception as e:
            logger.warning('Failed to load entry %s: %s', filename, e, exc_info=True)

    @pyqtSlot(str, str)
    def _on_value_changed(self, key: str, value: str):
        """Called when any editor widget changes a value."""
        if not self._current_filename:
            return
        before = self._editor.get_value(key)
        self._history.append(key, before, value)
        self._dirty = True
        self._act_save.setEnabled(True)

    # ------------------------------------------------------------------
    # Slots — edit actions
    # ------------------------------------------------------------------

    def _on_save(self):
        """Persist the current editor state to the .desktop file."""
        if not self._current_filename:
            return
        try:
            from openkylinmenueditor.MenulibreXdg import MenulibreDesktopEntry
            from openkylinmenueditor.util import MenuItemKeys

            entry = MenulibreDesktopEntry(self._current_filename)
            for key_tuple in MenuItemKeys:
                key = key_tuple[0]
                val = self._editor.get_value(key)
                if val is not None:
                    entry[key] = str(val) if not isinstance(val, str) else val

            # Write back to file
            data = entry.keyfile.to_data()
            with open(self._current_filename, 'w', encoding='utf-8') as f:
                f.write(data)

            self._dirty = False
            self._act_save.setEnabled(False)
            self._act_revert.setEnabled(False)
            self._history.clear()
            self._status_label.setText(_('Saved'))

        except Exception as e:
            logger.error('Save failed: %s', e)
            show_error(self, _('Save Error'), str(e))

    def _on_undo(self):
        if not self._history.can_undo():
            return
        key, value = self._history.undo()
        self._editor.set_value(key, value)
        self._act_undo.setEnabled(self._history.can_undo())
        self._act_redo.setEnabled(True)

    def _on_redo(self):
        if not self._history.can_redo():
            return
        key, value = self._history.redo()
        self._editor.set_value(key, value)
        self._act_redo.setEnabled(self._history.can_redo())
        self._act_undo.setEnabled(True)

    def _on_revert(self):
        restore = self._history.restore()
        self._history.block()
        for key, value in restore.items():
            self._editor.set_value(key, value)
        self._history.clear()
        self._history.unblock()
        self._dirty = False
        self._act_save.setEnabled(False)
        self._act_revert.setEnabled(False)

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
    app.setApplicationName('OpenKylinMenuEditor')
    app.setApplicationVersion('0.1')
    app.setOrganizationName('Tiago Camacho')

    # Apply basic stylesheet
    app.setStyleSheet(_get_stylesheet())

    window = MainWindow()
    window.show()
    return app.exec_()


def _get_stylesheet() -> str:
    return """
/* ── Global ──────────────────────────────────────────────────────── */
QMainWindow, QWidget {
    font-family: "Noto Sans", "HarmonyOS Sans SC", "Segoe UI", sans-serif;
    font-size: 9pt;
}

/* ── Menu bar ─────────────────────────────────────────────────────── */
QMenuBar {
    background: palette(window);
    border-bottom: 1px solid palette(mid);
    padding: 2px 4px;
}
QMenuBar::item {
    padding: 4px 10px;
    border-radius: 4px;
}
QMenuBar::item:selected {
    background: palette(highlight);
    color: palette(highlighted-text);
}
QMenu {
    border: 1px solid palette(mid);
    border-radius: 6px;
    padding: 4px;
}
QMenu::item {
    padding: 6px 20px;
    border-radius: 4px;
}
QMenu::item:selected {
    background: palette(highlight);
    color: palette(highlighted-text);
}
QMenu::separator {
    height: 1px;
    background: palette(mid);
    margin: 4px 8px;
}

/* ── Main toolbar ─────────────────────────────────────────────────── */
QToolBar#main_toolbar {
    background: palette(window);
    border-bottom: 1px solid palette(mid);
    padding: 4px 8px;
    spacing: 4px;
}
QToolBar#main_toolbar QToolButton {
    border-radius: 4px;
    padding: 4px;
    background: transparent;
}
QToolBar#main_toolbar QToolButton:hover {
    background: palette(midlight);
}
QToolBar#main_toolbar QToolButton:pressed {
    background: palette(mid);
}

/* ── Sidebar (left panel) ──────────────────────────────────────────── */
Treeview {
    background: palette(alternateBase);
}

/* ── Sidebar search ───────────────────────────────────────────────── */
QLineEdit#sidebar_search {
    border: 1px solid palette(mid);
    border-radius: 14px;
    padding: 4px 12px;
    background: palette(base);
    margin: 6px 8px 4px 8px;
}
QLineEdit#sidebar_search:focus {
    border: 1px solid palette(highlight);
}

/* ── Tree view ────────────────────────────────────────────────────── */
QTreeView#sidebar_tree {
    border: none;
    background: palette(alternateBase);
    outline: none;
    show-decoration-selected: 1;
}
QTreeView#sidebar_tree::item {
    min-height: 30px;
    padding-left: 4px;
    padding-right: 4px;
    border-radius: 4px;
}
QTreeView#sidebar_tree::item:selected {
    background: palette(highlight);
    color: palette(highlighted-text);
    border-radius: 4px;
}
QTreeView#sidebar_tree::item:hover:!selected {
    background: palette(midlight);
    border-radius: 4px;
}
QTreeView#sidebar_tree::branch {
    background: palette(alternateBase);
}
QTreeView#sidebar_tree::branch:has-children:!has-siblings:closed,
QTreeView#sidebar_tree::branch:closed:has-children:has-siblings {
    border-image: none;
    image: url(none);
}

/* ── Sidebar bottom toolbar ───────────────────────────────────────── */
Treeview QToolBar {
    background: palette(alternateBase);
    border-top: 1px solid palette(mid);
    spacing: 2px;
    padding: 2px 4px;
}
Treeview QToolBar QToolButton {
    border-radius: 4px;
    padding: 3px;
    background: transparent;
}
Treeview QToolBar QToolButton:hover {
    background: palette(midlight);
}
Treeview QToolBar QToolButton:pressed {
    background: palette(mid);
}
Treeview QToolBar QToolButton:checked {
    background: palette(highlight);
    color: palette(highlighted-text);
}

/* ── Splitter ─────────────────────────────────────────────────────── */
QSplitter::handle {
    background: palette(mid);
}
QSplitter::handle:horizontal {
    width: 1px;
}

/* ── Scroll area ─────────────────────────────────────────────────── */
QScrollArea {
    border: none;
    background: palette(base);
}

/* ── Editor header card ───────────────────────────────────────────── */
QFrame#editor_header {
    background: palette(base);
    border: 1px solid palette(mid);
    border-radius: 8px;
    padding: 4px;
}

/* ── Filename bar ─────────────────────────────────────────────────── */
QFrame#filename_bar {
    background: palette(alternateBase);
    border: 1px solid palette(mid);
    border-radius: 6px;
}

/* ── Editor scroll content (gray outer background) ───────────────── */
QWidget#editor_content {
    background: palette(window);
}

/* ── Section cards ────────────────────────────────────────────────── */
QLabel#section_title {
    color: palette(text);
    font-size: 9pt;
    font-weight: 500;
    padding: 0 2px;
}
QFrame#section_card {
    background: palette(base);
    border: 1px solid palette(base);
    border-radius: 10px;
}
QFrame#section_card QWidget {
    background: transparent;
}

/* ── Row separators inside cards ─────────────────────────────────── */
QFrame#row_separator {
    border: none;
    border-top: 1px solid palette(window);
    max-height: 1px;
    background: transparent;
}

/* ── Tabs ─────────────────────────────────────────────────────────── */
QTabWidget::pane {
    border: 1px solid palette(mid);
    border-radius: 0px 6px 6px 6px;
    background: palette(base);
}
QTabBar {
    background: transparent;
}
QTabBar::tab {
    background: palette(alternateBase);
    border: 1px solid palette(mid);
    border-bottom: none;
    border-radius: 6px 6px 0px 0px;
    padding: 6px 16px;
    margin-right: 2px;
    min-width: 60px;
}
QTabBar::tab:selected {
    background: palette(base);
    border-bottom: 1px solid palette(base);
    color: palette(highlight);
    font-weight: 600;
}
QTabBar::tab:hover:!selected {
    background: palette(midlight);
}

/* ── Line edit ────────────────────────────────────────────────────── */
QLineEdit {
    border: 1px solid palette(mid);
    border-radius: 4px;
    padding: 4px 8px;
    background: palette(base);
    selection-background-color: palette(highlight);
}
QLineEdit:focus {
    border: 1px solid palette(highlight);
}
QLineEdit:disabled {
    color: palette(mid);
}

/* ── Push button ─────────────────────────────────────────────────── */
QPushButton {
    border: 1px solid palette(mid);
    border-radius: 4px;
    padding: 4px 12px;
    background: palette(button);
    min-width: 60px;
}
QPushButton:hover {
    background: palette(midlight);
    border: 1px solid palette(highlight);
}
QPushButton:pressed {
    background: palette(mid);
}
QPushButton:flat {
    border: none;
    background: transparent;
    min-width: 0;
    padding: 2px;
    border-radius: 4px;
}
QPushButton:flat:hover {
    background: palette(midlight);
}

/* ── Checkbox ─────────────────────────────────────────────────────── */
QCheckBox {
    spacing: 8px;
}
QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid palette(mid);
    border-radius: 3px;
    background: palette(base);
}
QCheckBox::indicator:checked {
    background: palette(highlight);
    border: 1px solid palette(highlight);
}
QCheckBox::indicator:hover {
    border: 1px solid palette(highlight);
}

/* ── ComboBox ─────────────────────────────────────────────────────── */
QComboBox {
    border: 1px solid palette(mid);
    border-radius: 4px;
    padding: 4px 8px;
    background: palette(base);
}
QComboBox:focus {
    border: 1px solid palette(highlight);
}
QComboBox::drop-down {
    border: none;
    width: 20px;
}
QComboBox QAbstractItemView {
    border: 1px solid palette(mid);
    border-radius: 4px;
    selection-background-color: palette(highlight);
}

/* ── Table view ───────────────────────────────────────────────────── */
QTableWidget, QTableView {
    border: 1px solid palette(mid);
    border-radius: 6px;
    gridline-color: palette(mid);
    background: palette(base);
    alternate-background-color: palette(alternateBase);
    selection-background-color: palette(highlight);
    selection-color: palette(highlighted-text);
}
QHeaderView::section {
    background: palette(alternateBase);
    border: none;
    border-right: 1px solid palette(mid);
    border-bottom: 1px solid palette(mid);
    padding: 4px 8px;
    font-weight: 600;
}

/* ── Scrollbars ───────────────────────────────────────────────────── */
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 2px;
    border-radius: 4px;
}
QScrollBar::handle:vertical {
    background: palette(mid);
    border-radius: 4px;
    min-height: 20px;
}
QScrollBar::handle:vertical:hover {
    background: palette(shadow);
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}
QScrollBar:horizontal {
    background: transparent;
    height: 8px;
    margin: 2px;
    border-radius: 4px;
}
QScrollBar::handle:horizontal {
    background: palette(mid);
    border-radius: 4px;
    min-width: 20px;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0px;
}

/* ── Status bar ───────────────────────────────────────────────────── */
QStatusBar {
    background: palette(window);
    border-top: 1px solid palette(mid);
    padding: 2px 8px;
}
QStatusBar QLabel {
    color: palette(mid);
    font-size: 8pt;
}
"""
