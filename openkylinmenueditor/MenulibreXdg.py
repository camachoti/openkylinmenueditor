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

import locale
import os
import json
import configparser
from locale import gettext as _

import subprocess

locale.textdomain('openkylinmenueditor')


sudo = os.getuid() == 0
default_locale = locale.getdefaultlocale()[0]


def _get_user_data_dir():
    return os.environ.get('XDG_DATA_HOME', os.path.expanduser('~/.local/share'))


def _get_system_config_dirs():
    dirs = os.environ.get('XDG_CONFIG_DIRS', '/etc/xdg')
    return [d for d in dirs.split(':') if d]


class _KeyFile:
    """Thin configparser wrapper that mimics the GLib.KeyFile API used here."""

    def __init__(self):
        self._cp = configparser.RawConfigParser(strict=False)
        self._cp.optionxform = str  # preserve case

    def load_from_file(self, filename, _flags=None):
        self._cp = configparser.RawConfigParser(strict=False)
        self._cp.optionxform = str
        self._cp.read(filename, encoding='utf-8')

    # --- read helpers ---

    def get_string(self, group, key):
        try:
            return self._cp.get(group, key)
        except (configparser.NoSectionError, configparser.NoOptionError):
            return None

    def get_value(self, group, key):
        return self.get_string(group, key)

    def get_locale_string(self, group, key, locale_str):
        """Return localised value for key[locale_str] or plain key."""
        locale_key = '%s[%s]' % (key, locale_str)
        val = self.get_string(group, locale_key)
        if val is None:
            val = self.get_string(group, key)
        return val

    def get_string_list(self, group, key):
        val = self.get_string(group, key)
        if val is None:
            return []
        # Desktop-spec lists are semicolon-separated, trailing semicolon allowed
        return [v for v in val.split(';') if v]

    def get_groups(self):
        return (self._cp.sections(), len(self._cp.sections()))

    def get_keys(self, group):
        try:
            keys = list(self._cp.options(group))
            return (keys, len(keys))
        except configparser.NoSectionError:
            return ([], 0)

    def get_start_group(self):
        sections = self._cp.sections()
        return sections[0] if sections else None

    # --- write helpers ---

    def set_value(self, group, key, value):
        if not self._cp.has_section(group):
            self._cp.add_section(group)
        self._cp.set(group, key, value)

    def set_locale_string(self, group, key, locale_str, value):
        locale_key = '%s[%s]' % (key, locale_str)
        self.set_value(group, locale_key, value)

    def to_data(self):
        """Serialise back to .desktop file string."""
        import io
        buf = io.StringIO()
        self._cp.write(buf, space_around_delimiters=False)
        # configparser writes ' = ', strip spaces around '='
        raw = buf.getvalue()
        # configparser lowercases option names — we preserved case via optionxform
        # also add trailing newline after each section as spec requires
        return raw


class MenulibreDesktopEntry:

    """Basic class for Desktop Entry files"""

    def __init__(self, filename=None):
        """Initialize the MenulibreDesktopEntry instance."""
        self.keyfile = _KeyFile()
        if filename is not None and os.path.isfile(filename):
            self.load_properties(filename)
        else:
            self['Version'] = '1.1'
            self['Type'] = 'Application'
            self['Name'] = _('New Launcher')
            self['Comment'] = ''
            self['Icon'] = 'applications-other'
            self['Exec'] = ''
            self['Path'] = ''
            self['Terminal'] = 'false'
            self['StartupNotify'] = 'false'
            self['Categories'] = ''

    def __getitem__(self, prop_name):
        """Get property from this object like a dictionary."""
        return self.get_property('Desktop Entry', prop_name, default_locale)

    def __setitem__(self, key, value):
        """Set property to this object like a dictionary."""
        self._set_value("Desktop Entry", key, value)
        if key in ["Name", "GenericName", "Comment", "Keywords"]:
            self._set_locale_string("Desktop Entry", key, default_locale,
                                    value)

    def load_properties(self, filename):
        """Load the properties."""
        self.keyfile = _KeyFile()
        self.keyfile.load_from_file(filename)

    def get_property(self, category, prop_name, locale_str=default_locale):
        """Return the value of the specified property."""
        prop = self.get_named_property(category, prop_name, locale_str)
        if prop in ['true', 'false']:
            return prop == 'true'
        if prop_name in ['Hidden', 'NoDisplay', 'Terminal', 'StartupNotify']:
            return False
        return prop

    def get_named_property(self, group, key, locale_str=None):
        """Return the value of the specified named property."""
        if key in ["Name", "GenericName", "Comment", "Keywords"]:
            if locale_str is not None:
                return self._get_locale_string(group, key, locale_str)

        value = self._get_value(group, key)

        if value is not None:
            return value

        return ""

    def get_actions(self):
        """Return a list of the Unity action groups."""
        if "Actions" in self._get_keys("Desktop Entry"):
            action_key = "Actions"
        elif "X-Ayatana-Desktop-Shortcuts" in self._get_keys("Desktop Entry"):
            action_key = "X-Ayatana-Desktop-Shortcuts"
        else:
            return json.dumps([])

        enabled_quicklists = self._get_string_list("Desktop Entry", action_key)

        quicklists = []

        for group in self._get_groups():
            name = self._get_action_group_name(group)
            if name is None:
                continue
            displayed_name = self.get_property(group, "Name")
            command = self.get_property(group, "Exec")
            enabled = name in enabled_quicklists
            quicklists.append([enabled, name, displayed_name, command])

        return json.dumps(quicklists)

    def _get_action_group_name(self, group):
        if group.startswith("Desktop Action "):
            name = group.replace("Desktop Action", "")
        elif group.endswith(" Shortcut Group"):
            name = group.replace("Shortcut Group", "")
        else:
            return None
        name = name.strip()
        if len(name) > 0:
            return name
        return None

    def _get_locale_string(self, group, key, locale_str):
        value = self.keyfile.get_locale_string(group, key, locale_str)

        if value is not None:
            return value

        if '_' in locale_str:
            locale_str = locale_str.split("_")[0]
            return self._get_locale_string(group, key, locale_str)

        return self._get_string(group, key)

    def _set_locale_string(self, group, key, locale_str, value):
        self.keyfile.set_locale_string(group, key, locale_str, value)

    def _get_string(self, group, key):
        value = self.keyfile.get_string(group, key)
        if value is not None:
            return value
        return ""

    def _get_value(self, group, key):
        return self.keyfile.get_value(group, key)

    def _set_value(self, group, key, value):
        self.keyfile.set_value(group, key, value)

    def _get_string_list(self, group, key):
        return self.keyfile.get_string_list(group, key)

    def _get_groups(self):
        return self.keyfile.get_groups()[0]

    def _get_keys(self, group):
        return self.keyfile.get_keys(group)[0]


def desktop_menu_update():
    subprocess.call(["xdg-desktop-menu", "forceupdate"])


def desktop_menu_install(directory_files, desktop_files):
    """Install one or more applications in a submenu of the desktop menu
    system.  If multiple directory files are provided each file will represent
    a submenu within the menu that precedes it, creating a nested menu
    hierarchy (sub-sub-menus). The menu entries themselves will be added to
    the last submenu. """
    # Check for the minimum required arguments
    if len(directory_files) == 0 or len(desktop_files) == 0:
        return True

    # Do not install to system paths.
    for path in _get_system_config_dirs():
        for filename in directory_files:
            if filename.startswith(path):
                return True

    # xdg-desktop-menu doesn't behave nicely with vendor- directories
    # without vendor- applications.
    directory_dir = os.path.join(
        _get_user_data_dir(),
        "desktop-directories")
    for filename in directory_files:
        if not filename.startswith(directory_dir):
            continue
        if "/" not in filename:
            continue
        basename = os.path.basename(filename)
        relative = filename.split("desktop-directories/")[1]
        vendor_path = relative.replace("/%s" % basename, "")
        if len(vendor_path) == 0:
            continue
        vendor_prefix = vendor_path.replace("/", "-") + "-"
        if basename.startswith(vendor_prefix):
            return False

    cmd_list = ["xdg-desktop-menu", "install", "--novendor"]
    cmd_list = cmd_list + directory_files + desktop_files
    subprocess.call(cmd_list)

    return True


def desktop_menu_uninstall(directory_files, desktop_files):  # noqa
    """Remove applications or submenus from the desktop menu system
    previously installed with xdg-desktop-menu install."""
    # Check for the minimum required arguments
    if len(directory_files) == 0 or len(desktop_files) == 0:
        return

    # Do not uninstall from system paths.
    for path in _get_system_config_dirs():
        for filename in directory_files:
            if filename.startswith(path):
                return

    # xdg-desktop-menu uninstall does not work... implement ourselves.
    basenames = []
    for filename in directory_files:
        basenames.append(os.path.basename(filename))
    basenames.sort()
    base_filename = os.path.basename(desktop_files[0])

    # Find the file with all the details to remove the filename.
    merged_dir = os.path.join(os.environ.get('XDG_CONFIG_HOME', os.path.expanduser('~/.config')),
                              "menus", "applications-merged")

    for filename in os.listdir(merged_dir):
        filename = os.path.join(merged_dir, filename)
        found_directories = []
        filename_found = False
        with open(filename, 'r') as open_file:
            write_contents = ""
            for line in open_file:
                if "<Filename>" in line:
                    if base_filename in line:
                        filename_found = True
                    else:
                        write_contents += line
                else:
                    write_contents += line
                if "<Directory>" in line:
                    line = line.split("<Directory>")[1]
                    line = line.split("</Directory>")[0]
                    found_directories.append(line)
        if filename_found:
            found_directories.sort()
            if basenames == found_directories:
                with open(filename, 'w') as open_file:
                    open_file.write(write_contents)
                return
