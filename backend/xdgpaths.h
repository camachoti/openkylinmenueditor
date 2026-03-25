#pragma once
#include <QString>
#include <QStringList>

/**
 * XdgPaths — XDG base-directory helpers.
 *
 * Mirrors the utility functions from openkylinmenueditor/util.py:
 *   _xdg_user_data_dir, _xdg_user_config_dir,
 *   _xdg_system_data_dirs, _xdg_system_config_dirs,
 *   getUserApplicationsDirectory, getUserDirectoriesDirectory,
 *   getUserMenusDirectory, getSystemMenuPath, …
 */
class XdgPaths
{
public:
    // ── User directories ──────────────────────────────────────────────
    static QString userDataDir();           // $XDG_DATA_HOME (~/.local/share)
    static QString userConfigDir();         // $XDG_CONFIG_HOME (~/.config)
    static QString userApplicationsDir();   // userDataDir/applications   (created if absent)
    static QString userDirectoriesDir();    // userDataDir/desktop-directories (created if absent)
    static QString userMenusDir();          // userConfigDir/menus         (created if absent)

    // ── System directories ────────────────────────────────────────────
    static QStringList systemDataDirs();    // $XDG_DATA_DIRS
    static QStringList systemConfigDirs();  // $XDG_CONFIG_DIRS

    static QStringList itemSearchPaths();        // systemDataDirs + /applications
    static QStringList directorySearchPaths();   // systemDataDirs + /desktop-directories

    // ── File lookup ───────────────────────────────────────────────────
    static QString findSystemItem(const QString &fileId);        // .desktop file
    static QString findSystemDirectory(const QString &fileId);   // .directory file
    static QString findSystemMenu(const QString &fileId);        // .menu file

    static QString findUserItem(const QString &fileId);
    static QString findUserDirectory(const QString &fileId);

    // ── Save-path generation ──────────────────────────────────────────
    /**
     * Return the writable path where a launcher should be saved.
     * @param name        Human-readable name (used to build a filename when unique=true).
     * @param existingPath  Current file path, or empty if creating new.
     * @param type        "Application" | "Directory"
     * @param forceUpdate If true, return the derived path even if it already exists.
     */
    static QString getSavePath(const QString &name,
                               const QString &existingPath,
                               const QString &type,
                               bool forceUpdate = false);

    // ── Menu prefix ───────────────────────────────────────────────────
    static QString defaultMenuPrefix();   // e.g. "ukui-", "gnome-", "kf5-"
    static QString defaultMenuName();     // prefix without trailing "-"

private:
    static QString ensureDir(const QString &path);
};
