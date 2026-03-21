#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
Qt data model for the application menu tree.

Replaces Gtk.TreeStore + MenuEditor.get_treestore().
Uses GMenu (via gi.repository) only for reading the system menu — no GTK widgets.
"""

import os
import logging

from PyQt5.QtCore import Qt, QVariant
from PyQt5.QtGui import QIcon, QStandardItem, QStandardItemModel

logger = logging.getLogger('openkylinmenueditor')

# Column indices — kept identical to MenuEditor.py for easy cross-referencing
COL_NAME = 0
COL_DISPLAY_NAME = 1
COL_COMMENT = 2
COL_EXEC = 3
COL_CATEGORIES = 4
COL_TYPE = 5
COL_ICON_NAME = 6
COL_FILENAME = 7
COL_EXPAND = 8
COL_SHOW = 9

# Extra Qt roles
ROLE_TYPE = Qt.UserRole + 1
ROLE_FILENAME = Qt.UserRole + 2
ROLE_SHOW = Qt.UserRole + 3
ROLE_EXEC = Qt.UserRole + 4
ROLE_CATEGORIES = Qt.UserRole + 5
ROLE_COMMENT = Qt.UserRole + 6
ROLE_ICON_NAME = Qt.UserRole + 7


def _load_icon(icon_name: str) -> QIcon:
    """Return a QIcon for an icon name or file path."""
    if icon_name and os.path.isabs(icon_name) and os.path.isfile(icon_name):
        return QIcon(icon_name)
    if icon_name:
        icon = QIcon.fromTheme(icon_name)
        if not icon.isNull():
            return icon
    return QIcon.fromTheme('application-x-executable')


def _build_item(name, display_name, comment, executable, categories,
                item_type, icon_name, filename, show):
    """Create a QStandardItem row with all metadata stored in UserRoles."""
    item = QStandardItem(display_name or name)
    item.setIcon(_load_icon(icon_name))
    item.setEditable(False)

    item.setData(name, Qt.DisplayRole)
    item.setData(item_type, ROLE_TYPE)
    item.setData(filename or '', ROLE_FILENAME)
    item.setData(show, ROLE_SHOW)
    item.setData(executable or '', ROLE_EXEC)
    item.setData(categories or '', ROLE_CATEGORIES)
    item.setData(comment or '', ROLE_COMMENT)
    item.setData(icon_name or '', ROLE_ICON_NAME)
    return item


def build_model_from_structure(menu_structure) -> QStandardItemModel:
    """
    Convert the list returned by MenuEditor.get_menus() into a
    QStandardItemModel.

    menu_structure is a list of lists returned by get_menus() — each entry
    is [item_type, entry_id, details_dict, submenus_or_None].
    """
    from openkylinmenueditor.util import MenuItemTypes, escapeText  # noqa

    model = QStandardItemModel()
    model.setHorizontalHeaderLabels(['Name'])

    def _fill(parent_item, items):
        for entry in items:
            item_type = entry[0]
            if item_type == MenuItemTypes.SEPARATOR:  # type: ignore
                sep_item = QStandardItem('─────')
                sep_item.setEditable(False)
                sep_item.setData(item_type, ROLE_TYPE)
                sep_item.setData('', ROLE_FILENAME)
                sep_item.setData(False, ROLE_SHOW)
                parent_item.appendRow(sep_item)
                continue

            details = entry[2]
            submenus = entry[3]

            row_item = _build_item(
                name=details.get('display_name', ''),
                display_name=escapeText(details.get('display_name', '')),
                comment=details.get('comment', ''),
                executable=details.get('executable', ''),
                categories=details.get('categories', ''),
                item_type=item_type,
                icon_name=details.get('icon_name', ''),
                filename=details.get('filename', ''),
                show=details.get('show', True),
            )
            parent_item.appendRow(row_item)

            if item_type == MenuItemTypes.DIRECTORY and submenus:  # type: ignore
                _fill(row_item, submenus)

    root = model.invisibleRootItem()
    _fill(root, menu_structure)
    return model


def get_model() -> QStandardItemModel:
    """Load the system application menu and return a QStandardItemModel."""
    try:
        from openkylinmenueditor.MenuEditor import get_menus
        menus = get_menus()
        if not menus:
            logger.warning('No menus found')
            return QStandardItemModel()
        # get_menus() returns a list of toplevel submenu lists
        all_items = []
        for toplevel in menus:
            all_items.extend(toplevel)
        return build_model_from_structure(all_items)
    except Exception as e:
        logger.error('Failed to load menu model: %s', e)
        return QStandardItemModel()
