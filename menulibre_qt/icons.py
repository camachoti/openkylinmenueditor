"""
Heroicons-based icon provider for KylinMenuEditor.

Heroicons are MIT-licensed SVG icons by Tailwind Labs.
https://heroicons.com

Usage:
    from menulibre_qt.icons import get_icon
    action.setIcon(get_icon('list-add'))
"""

from PyQt5.QtCore import QByteArray
from PyQt5.QtGui import QIcon, QPixmap

# ---------------------------------------------------------------------------
# Heroicons SVG paths (24px outline style, stroke="currentColor")
# ---------------------------------------------------------------------------

_HEROICONS: dict[str, str] = {
    # plus / list-add
    'list-add': '<path stroke-linecap="round" stroke-linejoin="round" d="M12 4.5v15m7.5-7.5h-15"/>',

    # minus / list-remove
    'list-remove': '<path stroke-linecap="round" stroke-linejoin="round" d="M5 12h14"/>',

    # trash / edit-delete
    'edit-delete': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M14.74 9l-.346 9m-4.788 0L9.26 9m9.968-3.21c.342.052.682.107 1.022.166m-1.022-.165L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12 .562c.34-.059.68-.114 1.022-.165m0 0a48.11 48.11 0 013.478-.397m7.5 0v-.916c0-1.18-.91-2.164-2.09-2.201a51.964 51.964 0 00-3.32 0c-1.18.037-2.09 1.022-2.09 2.201v.916m7.5 0a48.667 48.667 0 00-7.5 0"/>'
    ),

    # arrow-down-tray / document-save
    'document-save': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5M16.5 12L12 16.5m0 0L7.5 12m4.5 4.5V3"/>'
    ),

    # arrow-right-on-rectangle / application-exit
    'application-exit': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M15.75 9V5.25A2.25 2.25 0 0013.5 3h-6a2.25 2.25 0 00-2.25 2.25v13.5A2.25 2.25 0 007.5 21h6a2.25 2.25 0 002.25-2.25V15M12 9l-3 3m0 0l3 3m-3-3h12.75"/>'
    ),

    # arrow-uturn-left / edit-undo
    'edit-undo': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M9 15L3 9m0 0l6-6M3 9h12a6 6 0 010 12h-3"/>'
    ),

    # arrow-uturn-right / edit-redo
    'edit-redo': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M15 15l6-6m0 0l-6-6m6 6H9a6 6 0 000 12h3"/>'
    ),

    # arrow-path / document-revert
    'document-revert': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M16.023 9.348h4.992v-.001M2.985 19.644v-4.992m0 0h4.992m-4.993 0l3.181 3.183a8.25 8.25 0 0013.803-3.7M4.031 9.865a8.25 8.25 0 0113.803-3.7l3.181 3.182m0-4.991v4.99"/>'
    ),

    # folder-plus / folder-new
    'folder-new': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M12 10.5v6m3-3H9m4.06-7.19l-2.12-2.12a1.5 1.5 0 00-1.061-.44H4.5A2.25 2.25 0 002.25 6v12a2.25 2.25 0 002.25 2.25h15A2.25 2.25 0 0021.75 18V9a2.25 2.25 0 00-2.25-2.25h-5.379a1.5 1.5 0 01-1.06-.44z"/>'
    ),

    # play / media-playback-start
    'media-playback-start': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M5.25 5.653c0-.856.917-1.398 1.667-.986l11.54 6.348a1.125 1.125 0 010 1.971l-11.54 6.347a1.125 1.125 0 01-1.667-.985V5.653z"/>'
    ),

    # information-circle / help-about
    'help-about': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M11.25 11.25l.041-.02a.75.75 0 011.063.852l-.708 2.836a.75.75 0 001.063.853l.041-.021M21 12a9 9 0 11-18 0 9 9 0 0118 0zm-9-3.75h.008v.008H12V8.25z"/>'
    ),

    # x-mark / edit-clear
    'edit-clear': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M6 18L18 6M6 6l12 12"/>'
    ),

    # arrow-up / go-up
    'go-up': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M4.5 10.5L12 3m0 0l7.5 7.5M12 3v18"/>'
    ),

    # arrow-down / go-down
    'go-down': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M19.5 13.5L12 21m0 0l-7.5-7.5M12 21V3"/>'
    ),

    # bars-arrow-up / view-sort-ascending
    'view-sort-ascending': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M3 4.5h14.25M3 9h9.75M3 13.5h5.25m5.25-.75L17.25 9m0 0L21 12.75M17.25 9v12"/>'
    ),

    # bars-3 / view-list-symbolic
    'view-list-symbolic': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M3.75 6.75h16.5M3.75 12h16.5m-16.5 5.25h16.5"/>'
    ),

    # document-duplicate / edit-copy
    'edit-copy': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M15.75 17.25v3.375c0 .621-.504 1.125-1.125 1.125h-9.75a1.125 1.125 0 01-1.125-1.125V7.875c0-.621.504-1.125 1.125-1.125H6.75a9.06 9.06 0 011.5.124m7.5 10.376h3.375c.621 0 1.125-.504 1.125-1.125V11.25c0-4.46-3.243-8.161-7.5-8.876a9.06 9.06 0 00-1.5-.124H9.375c-.621 0-1.125.504-1.125 1.125v3.5m7.5 10.375H9.375a1.125 1.125 0 01-1.125-1.125v-9.25m12 6.625v-1.875a3.375 3.375 0 00-3.375-3.375h-1.5a1.125 1.125 0 01-1.125-1.125v-1.5a3.375 3.375 0 00-3.375-3.375H9.75"/>'
    ),

    # cog-6-tooth / preferences-system-windows
    'preferences-system-windows': (
        '<path stroke-linecap="round" stroke-linejoin="round" '
        'd="M9.594 3.94c.09-.542.56-.94 1.11-.94h2.593c.55 0 1.02.398 1.11.94l.213 1.281c.063.374.313.686.645.87.074.04.147.083.22.127.324.196.72.257 1.075.124l1.217-.456a1.125 1.125 0 011.37.49l1.296 2.247a1.125 1.125 0 01-.26 1.431l-1.003.827c-.293.24-.438.613-.431.992a6.759 6.759 0 010 .255c-.007.378.138.75.43.99l1.005.828c.424.35.534.954.26 1.43l-1.298 2.247a1.125 1.125 0 01-1.369.491l-1.217-.456c-.355-.133-.75-.072-1.076.124a6.57 6.57 0 01-.22.128c-.331.183-.581.495-.644.869l-.213 1.28c-.09.543-.56.941-1.11.941h-2.594c-.55 0-1.02-.398-1.11-.94l-.213-1.281c-.062-.374-.312-.686-.644-.87a6.52 6.52 0 01-.22-.127c-.325-.196-.72-.257-1.076-.124l-1.217.456a1.125 1.125 0 01-1.369-.49l-1.297-2.247a1.125 1.125 0 01.26-1.431l1.004-.827c.292-.24.437-.613.43-.992a6.932 6.932 0 010-.255c.007-.378-.138-.75-.43-.99l-1.004-.828a1.125 1.125 0 01-.26-1.43l1.297-2.247a1.125 1.125 0 011.37-.491l1.216.456c.356.133.751.072 1.076-.124.072-.044.146-.087.22-.128.332-.183.582-.495.644-.869l.214-1.281z"/>'
        '<path stroke-linecap="round" stroke-linejoin="round" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"/>'
    ),

    # squares-plus / list-add (alias used as window icon fallback)
    'kylinmenueditor': (
        '<rect x="3" y="3" width="8" height="8" rx="1"/>'
        '<rect x="13" y="3" width="8" height="8" rx="1"/>'
        '<rect x="3" y="13" width="8" height="8" rx="1"/>'
        '<path stroke-linecap="round" stroke-linejoin="round" d="M17 13v8M13 17h8"/>'
    ),
}

# ---------------------------------------------------------------------------
# Cache
# ---------------------------------------------------------------------------

_cache: dict[str, QIcon] = {}


def _build_svg(paths: str, color: str, size: int = 24) -> bytes:
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" '
        f'fill="none" stroke="{color}" stroke-width="1.5">'
        f'{paths}'
        f'</svg>'
    ).encode('utf-8')


def _auto_color() -> str:
    """Return a text color that works for the current palette."""
    try:
        from PyQt5.QtWidgets import QApplication
        palette = QApplication.palette()
        c = palette.text().color()
        return c.name()
    except Exception:
        return '#374151'


def get_icon(theme_name: str, color: str | None = None, size: int = 24) -> QIcon:
    """Return a QIcon for *theme_name*, using Heroicons if available.

    Falls back to ``QIcon.fromTheme(theme_name)`` for unknown names.
    The icon is rendered at *size*×*size* pixels. If *color* is not
    provided, the current application text-color is used, giving
    automatic dark-mode support.
    """
    if color is None:
        color = _auto_color()

    cache_key = f'{theme_name}:{color}:{size}'
    if cache_key in _cache:
        return _cache[cache_key]

    paths = _HEROICONS.get(theme_name)
    if paths is None:
        icon = QIcon.fromTheme(theme_name)
        _cache[cache_key] = icon
        return icon

    svg_bytes = _build_svg(paths, color, size)
    pm = QPixmap(size, size)
    from PyQt5.QtCore import Qt
    pm.fill(Qt.transparent)
    ok = pm.loadFromData(QByteArray(svg_bytes), 'SVG')
    if not ok or pm.isNull():
        icon = QIcon.fromTheme(theme_name)
    else:
        icon = QIcon(pm)

    _cache[cache_key] = icon
    return icon


def clear_cache() -> None:
    """Invalidate the icon cache (call after palette change)."""
    _cache.clear()
