#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""File operations — replaces menulibre/FileHandler.py (Gio/Gdk)."""

import os
import subprocess
import logging

from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QUrl
from PyQt5.QtGui import QDesktopServices

logger = logging.getLogger('menulibre')


def open_folder(path: str, parent=None) -> bool:
    """Open *path* in the system file manager."""
    url = QUrl.fromLocalFile(path)
    return QDesktopServices.openUrl(url)


def copy_to_clipboard(text: str) -> None:
    """Copy *text* to the system clipboard."""
    QApplication.clipboard().setText(text)


def _is_writable(path: str) -> bool:
    return os.access(path, os.W_OK)


def _get_text_editor() -> list:
    """Return a command list for a suitable text editor."""
    candidates = [
        ['xdg-open'],
        ['gedit'],
        ['kate'],
        ['mousepad'],
        ['xed'],
        ['featherpad'],
        ['nano'],          # fallback terminal editor
    ]
    import shutil
    for cmd in candidates:
        if shutil.which(cmd[0]):
            return cmd
    return ['xdg-open']


def open_editor(path: str, parent=None) -> None:
    """Open *path* in a text editor, elevating with pkexec if needed."""
    if _is_writable(path):
        cmd = _get_text_editor() + [path]
    else:
        # Try admin-capable editor or fall back to pkexec
        import shutil
        if shutil.which('pkexec'):
            editor = _get_text_editor()
            cmd = ['pkexec'] + editor + [path]
        else:
            cmd = _get_text_editor() + [path]
    try:
        subprocess.Popen(cmd)
    except Exception as e:
        logger.error('Failed to open editor: %s', e)
