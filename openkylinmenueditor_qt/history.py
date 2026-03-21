#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
History — replaces MenulibreHistory.py (GObject.GObject).

Pure Python, no GTK dependency. Uses pyqtSignal instead of GObject signals.
"""

import logging
from PyQt5.QtCore import QObject, pyqtSignal

logger = logging.getLogger('openkylinmenueditor')


class History(QObject):
    """Stores all undo/redo/revert history for the editor."""

    undo_changed = pyqtSignal(bool)
    redo_changed = pyqtSignal(bool)
    revert_changed = pyqtSignal(bool)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._undo = []
        self._redo = []
        self._restore = {}
        self._block = False

    def append(self, key, before, after):
        if self._block:
            return
        self._append_undo(key, before, after)
        self._clear_redo()
        self._check_revert()

    def store(self, key, value):
        self._restore[key] = value

    def restore(self):
        return self._restore.copy()

    def undo(self):
        key, before, after = self._pop_undo()
        self._append_redo(key, before, after)
        self._check_revert()
        return (key, before)

    def redo(self):
        key, before, after = self._pop_redo()
        self._append_undo(key, before, after)
        self._check_revert()
        return (key, after)

    def clear(self):
        self._clear_undo()
        self._clear_redo()
        self._restore.clear()
        self._check_revert()

    def block(self):
        logger.debug('Blocking history updates')
        self._block = True

    def unblock(self):
        logger.debug('Unblocking history updates')
        self._block = False

    def is_blocked(self):
        return self._block

    def can_undo(self):
        return len(self._undo) > 0

    def can_redo(self):
        return len(self._redo) > 0

    def _append_undo(self, key, before, after):
        self._undo.append((key, before, after))
        if len(self._undo) == 1:
            self.undo_changed.emit(True)

    def _pop_undo(self):
        history = self._undo.pop()
        if len(self._undo) == 0:
            self.undo_changed.emit(False)
        return history

    def _clear_undo(self):
        had = len(self._undo) > 0
        self._undo.clear()
        if had:
            self.undo_changed.emit(False)

    def _clear_redo(self):
        had = len(self._redo) > 0
        self._redo.clear()
        if had:
            self.redo_changed.emit(False)

    def _append_redo(self, key, before, after):
        self._redo.append((key, before, after))
        if len(self._redo) == 1:
            self.redo_changed.emit(True)

    def _pop_redo(self):
        history = self._redo.pop()
        if len(self._redo) == 0:
            self.redo_changed.emit(False)
        return history

    def _check_revert(self):
        if len(self._undo) == 0 and len(self._redo) == 0:
            self.revert_changed.emit(False)
        elif len(self._undo) == 1 or len(self._redo) == 1:
            self.revert_changed.emit(True)
