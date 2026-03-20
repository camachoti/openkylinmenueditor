#!/usr/bin/python3
# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#   MenuLibre - Advanced fd.o Compliant Menu Editor
#   Copyright (C) 2012-2024 Sean Davis <sean@bluesabre.org>
#
#   This program is free software: you can redistribute it and/or modify it
#   under the terms of the GNU General Public License version 3, as published
#   by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranties of
#   MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
#   PURPOSE.  See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Heroicons-inspired modern SVG icons for MenuLibre toolbar buttons.

Icons are based on Heroicons v2 outline style (MIT License, Tailwind Labs).
https://heroicons.com
"""

import logging

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('GdkPixbuf', '2.0')
from gi.repository import Gtk, GdkPixbuf  # type: ignore

logger = logging.getLogger('menulibre')

# Maps GTK system icon names → our custom SVG keys
_ICON_MAP = {
    "list-add-symbolic":               "add",
    "list-add":                        "add",
    "document-save-symbolic":          "save",
    "document-save":                   "save",
    "edit-undo-symbolic":              "undo",
    "edit-undo":                       "undo",
    "edit-redo-symbolic":              "redo",
    "edit-redo":                       "redo",
    "document-revert-symbolic":        "revert",
    "document-revert":                 "revert",
    "media-playback-start-symbolic":   "play",
    "system-run":                      "play",
    "edit-delete-symbolic":            "delete",
    "edit-delete":                     "delete",
    "open-menu-symbolic":              "settings",
    "emblem-system-symbolic":          "settings",
}

# Heroicons v2 outline paths (24×24 viewBox, stroke-width 1.5)
# Stroke color placeholder {c} is replaced at render time.
_SVG_TEMPLATE = (
    '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none">'
    '<path stroke="{c}" stroke-linecap="round" stroke-linejoin="round"'
    ' stroke-width="1.5" d="{d}"/>'
    '</svg>'
)

_PATHS = {
    # plus-circle
    "add":    "M12 9v6m3-3H9m12 0a9 9 0 1 1-18 0 9 9 0 0 1 18 0Z",
    # arrow-down-tray
    "save":   ("M3 16.5v2.25A2.25 2.25 0 0 0 5.25 21h13.5"
               "A2.25 2.25 0 0 0 21 18.75V16.5"
               "M16.5 12 12 16.5m0 0L7.5 12m4.5 4.5V3"),
    # arrow-uturn-left
    "undo":   "M9 15 3 9m0 0 6-6M3 9h12a6 6 0 0 1 0 12h-3",
    # arrow-uturn-right
    "redo":   "m15 15 6-6m0 0-6-6m6 6H9a6 6 0 0 0 0 12h3",
    # arrow-path
    "revert": ("M16.023 9.348h4.992v-.001"
               "M2.985 19.644v-4.992m0 0h4.992"
               "m-4.993 0 3.181 3.183a8.25 8.25 0 0 0 13.803-3.7"
               "M4.031 9.865a8.25 8.25 0 0 1 13.803-3.7"
               "l3.181 3.182m0-4.991v4.99"),
    # play
    "play":   ("M5.25 5.653c0-.856.917-1.398 1.667-.986"
               "l11.54 6.347a1.125 1.125 0 0 1 0 1.972"
               "l-11.54 6.347a1.125 1.125 0 0 1-1.667-.986V5.653Z"),
    # cog-6-tooth (gear + inner circle as two subpaths in one d attribute)
    "settings": (
        "M9.594 3.94c.09-.542.56-.94 1.11-.94h2.593c.55 0 1.02.398 1.11.94"
        "l.213 1.281c.063.374.313.686.645.87.074.04.147.083.22.127"
        "c.325.196.72.257 1.075.124l1.217-.456a1.125 1.125 0 0 1 1.37.49"
        "l1.296 2.247a1.125 1.125 0 0 1-.26 1.431l-1.003.827"
        "c-.293.241-.438.613-.43.992a7.723 7.723 0 0 1 0 .255"
        "c-.008.378.137.75.43.991l1.004.827c.424.35.534.955.26 1.43"
        "l-1.298 2.247a1.125 1.125 0 0 1-1.369.491l-1.217-.456"
        "c-.355-.133-.75-.072-1.076.124a6.47 6.47 0 0 1-.22.128"
        "c-.331.183-.581.495-.644.869l-.213 1.281c-.09.543-.56.94-1.11.94"
        "h-2.594c-.55 0-1.019-.398-1.11-.94l-.213-1.281"
        "c-.062-.374-.312-.686-.644-.87a6.52 6.52 0 0 1-.22-.127"
        "c-.325-.196-.72-.257-1.076-.124l-1.217.456"
        "a1.125 1.125 0 0 1-1.369-.49l-1.297-2.247"
        "a1.125 1.125 0 0 1 .26-1.431l1.004-.827"
        "c.292-.24.437-.613.43-.991a6.932 6.932 0 0 1 0-.255"
        "c.007-.38-.138-.751-.43-.992l-1.004-.827"
        "a1.125 1.125 0 0 1-.26-1.43l1.297-2.247"
        "a1.125 1.125 0 0 1 1.37-.491l1.216.456"
        "c.356.133.751.072 1.076-.124.072-.044.146-.086.22-.128"
        "c.332-.183.582-.495.644-.869l.214-1.28Z"
        " M15 12a3 3 0 1 1-6 0 3 3 0 0 1 6 0Z"
    ),
    # trash
    "delete": ("m14.74 9-.346 9m-4.788 0L9.26 9"
               "m9.968-3.21c.342.052.682.107 1.022.166"
               "m-1.022-.165L18.16 19.673"
               "a2.25 2.25 0 0 1-2.244 2.077H8.084"
               "a2.25 2.25 0 0 1-2.244-2.077L4.772 5.79"
               "m14.456 0a48.108 48.108 0 0 0-3.478-.397"
               "m-12 .562c.34-.059.68-.114 1.022-.165"
               "m0 0a48.11 48.11 0 0 1 3.478-.397"
               "m7.5 0v-.916c0-1.18-.91-2.164-2.09-2.201"
               "a51.964 51.964 0 0 0-3.32 0"
               "c-1.18.037-2.09 1.022-2.09 2.201v.916"
               "m7.5 0a48.667 48.667 0 0 0-7.5 0"),
}

_svg_supported = None


def _is_svg_supported():
    global _svg_supported
    if _svg_supported is not None:
        return _svg_supported
    try:
        loader = GdkPixbuf.PixbufLoader.new_with_type("svg")
        loader.write(b'<svg xmlns="http://www.w3.org/2000/svg"/>')
        loader.close()
        _svg_supported = True
    except Exception:
        _svg_supported = False
    return _svg_supported


def _get_stroke_color():
    """Return a stroke color appropriate for the current GTK theme."""
    settings = Gtk.Settings.get_default()
    if settings is not None:
        prefer_dark = settings.get_property(
            "gtk-application-prefer-dark-theme")
        if prefer_dark:
            return "#d4d4d4"
    return "#2d2d2d"


def _render_svg(key, pixel_size):
    path = _PATHS.get(key)
    if path is None:
        return None
    svg = _SVG_TEMPLATE.format(c=_get_stroke_color(), d=path).encode()
    try:
        loader = GdkPixbuf.PixbufLoader.new_with_type("svg")
        loader.set_size(pixel_size, pixel_size)
        loader.write(svg)
        loader.close()
        return loader.get_pixbuf()
    except Exception as e:
        logger.debug("icons: SVG render failed for %r: %s", key, e)
        return None


def make_image(icon_name, pixel_size=16):
    """Return a Gtk.Image for icon_name.

    Uses a custom Heroicons-style SVG when available; falls back to the
    system icon theme otherwise.
    """
    key = _ICON_MAP.get(icon_name)
    if key and _is_svg_supported():
        pixbuf = _render_svg(key, pixel_size)
        if pixbuf is not None:
            image = Gtk.Image.new_from_pixbuf(pixbuf)
            image.set_pixel_size(pixel_size)
            return image

    # Fallback: system icon
    image = Gtk.Image.new_from_icon_name(icon_name, Gtk.IconSize.BUTTON)
    image.set_pixel_size(pixel_size)
    image.set_property("use-fallback", True)
    return image
