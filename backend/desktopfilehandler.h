#pragma once
#include <QString>
#include <QStringList>
#include <QMap>

/**
 * DesktopEntry — in-memory representation of a .desktop or .directory file.
 *
 * Mirrors the fields used in openkylinmenueditor/util.py MenuItemKeys and
 * openkylinmenueditor/MenulibreXdg.py.
 *
 * All keys are stored as raw strings; typed accessors (bool, list) are
 * provided for convenience.
 */
struct DesktopEntry {
    // The [Desktop Entry] group key-value pairs, case-preserved
    QMap<QString, QString> keys;

    // Path on disk (may be empty for unsaved new entries)
    QString filePath;

    // Convenience typed accessors
    QString getString(const QString &key, const QString &defaultVal = {}) const;
    bool    getBool(const QString &key, bool defaultVal = false) const;
    QStringList getList(const QString &key) const;  // semicolon-separated

    void setString(const QString &key, const QString &value);
    void setBool(const QString &key, bool value);
    void setList(const QString &key, const QStringList &values);

    bool isValid() const;  // has at least Type and Name
};

/**
 * DesktopFileHandler — read and write .desktop / .directory files.
 *
 * Mirrors openkylinmenueditor/MenulibreXdg.py (_KeyFile) and the save
 * routines in openkylinmenueditor/MenulibreApplication.py.
 */
class DesktopFileHandler
{
public:
    /**
     * Load a .desktop or .directory file from disk.
     * Returns an empty DesktopEntry on error.
     */
    static DesktopEntry load(const QString &filePath);

    /**
     * Save a DesktopEntry to disk.
     * Creates parent directories as needed.
     * Returns true on success.
     */
    static bool save(const DesktopEntry &entry, const QString &filePath = {}, QString *errorMsg = nullptr);

    /**
     * Remove a .desktop or .directory file, using pkexec for system paths if needed.
     * Returns true on success.
     */
    static bool remove(const QString &filePath, QString *errorMsg = nullptr);

    /**
     * Return a new DesktopEntry pre-populated for a new Application launcher.
     */
    static DesktopEntry newApplication(const QString &name = {});

    /**
     * Return a new DesktopEntry pre-populated for a new Directory.
     */
    static DesktopEntry newDirectory(const QString &name = {});
};
