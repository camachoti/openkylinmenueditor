#include "xdgpaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

// ── helpers ───────────────────────────────────────────────────────────────────

static QString envOr(const char *var, const QString &fallback)
{
    QString v = QProcessEnvironment::systemEnvironment().value(QLatin1String(var));
    return v.isEmpty() ? fallback : v;
}

static QStringList envListOr(const char *var, const QString &fallback)
{
    QString v = QProcessEnvironment::systemEnvironment().value(QLatin1String(var));
    if (v.isEmpty()) v = fallback;
    QStringList result;
    for (const QString &p : v.split(QLatin1Char(':'), Qt::SkipEmptyParts))
        result << p;
    return result;
}

// ── XdgPaths implementation ───────────────────────────────────────────────────

QString XdgPaths::ensureDir(const QString &path)
{
    QDir().mkpath(path);
    return path;
}

QString XdgPaths::userDataDir()
{
    return envOr("XDG_DATA_HOME",
                 QDir::homePath() + QStringLiteral("/.local/share"));
}

QString XdgPaths::userConfigDir()
{
    return envOr("XDG_CONFIG_HOME",
                 QDir::homePath() + QStringLiteral("/.config"));
}

QString XdgPaths::userApplicationsDir()
{
    return ensureDir(userDataDir() + QStringLiteral("/applications"));
}

QString XdgPaths::userDirectoriesDir()
{
    return ensureDir(userDataDir() + QStringLiteral("/desktop-directories"));
}

QString XdgPaths::userMenusDir()
{
    return ensureDir(userConfigDir() + QStringLiteral("/menus"));
}

QStringList XdgPaths::systemDataDirs()
{
    return envListOr("XDG_DATA_DIRS",
                     QStringLiteral("/usr/local/share:/usr/share"));
}

QStringList XdgPaths::systemConfigDirs()
{
    return envListOr("XDG_CONFIG_DIRS", QStringLiteral("/etc/xdg"));
}

QStringList XdgPaths::itemSearchPaths()
{
    QStringList result;
    for (const QString &d : systemDataDirs())
        result << d + QStringLiteral("/applications");
    return result;
}

QStringList XdgPaths::directorySearchPaths()
{
    QStringList result;
    for (const QString &d : systemDataDirs())
        result << d + QStringLiteral("/desktop-directories");
    return result;
}

QString XdgPaths::findSystemItem(const QString &fileId)
{
    for (const QString &dir : itemSearchPaths()) {
        QString p = dir + QLatin1Char('/') + fileId;
        if (QFile::exists(p)) return p;
    }
    return {};
}

QString XdgPaths::findSystemDirectory(const QString &fileId)
{
    for (const QString &dir : directorySearchPaths()) {
        QString p = dir + QLatin1Char('/') + fileId;
        if (QFile::exists(p)) return p;
    }
    return {};
}

QString XdgPaths::findSystemMenu(const QString &fileId)
{
    for (const QString &dir : systemConfigDirs()) {
        QString p = dir + QStringLiteral("/menus/") + fileId;
        if (QFile::exists(p)) return p;
    }
    return {};
}

QString XdgPaths::findUserItem(const QString &fileId)
{
    QString p = userApplicationsDir() + QLatin1Char('/') + fileId;
    return QFile::exists(p) ? p : QString{};
}

QString XdgPaths::findUserDirectory(const QString &fileId)
{
    QString p = userDirectoriesDir() + QLatin1Char('/') + fileId;
    return QFile::exists(p) ? p : QString{};
}

QString XdgPaths::defaultMenuPrefix()
{
    QString prefix = QProcessEnvironment::systemEnvironment()
                         .value(QStringLiteral("XDG_MENU_PREFIX"));
    if (!prefix.isEmpty())
        return prefix;

    // Fallback: detect from XDG_CURRENT_DESKTOP
    QString desktop = QProcessEnvironment::systemEnvironment()
                          .value(QStringLiteral("XDG_CURRENT_DESKTOP"))
                          .toLower();
    if (desktop.contains(QStringLiteral("ukui")))
        return QStringLiteral("kf5-");
    if (desktop.contains(QStringLiteral("kde"))) {
        QString ver = QProcessEnvironment::systemEnvironment()
                          .value(QStringLiteral("KDE_SESSION_VERSION"), QStringLiteral("4"));
        return ver.toInt() >= 5 ? QStringLiteral("kf5-") : QStringLiteral("kde4-");
    }
    if (desktop.contains(QStringLiteral("gnome")))
        return QStringLiteral("gnome-");
    if (desktop.contains(QStringLiteral("mate")))
        return QStringLiteral("mate-");
    if (desktop.contains(QStringLiteral("xfce")))
        return QStringLiteral("xfce-");
    if (desktop.contains(QStringLiteral("cinnamon")))
        return QStringLiteral("cinnamon-");
    return {};
}

QString XdgPaths::defaultMenuName()
{
    QString prefix = defaultMenuPrefix();
    if (prefix.endsWith(QLatin1Char('-')))
        return prefix.left(prefix.length() - 1);
    return prefix;
}

QString XdgPaths::getSavePath(const QString &name,
                               const QString &existingPath,
                               const QString &type,
                               bool forceUpdate)
{
    bool isNew = existingPath.isEmpty();
    QString userAppDir = QDir::homePath() + "/.local/share/applications/";

    // Se o arquivo já está no usuário (personalizado), sobrescreve
    if (!isNew && existingPath.startsWith(userAppDir)) {
        return existingPath;
    }
    // Se é arquivo de sistema: salva uma cópia personalizada no home
    if (!isNew) {
        QString ext = type == QStringLiteral("Application") ? ".desktop" : ".directory";
        QString dir = type == QStringLiteral("Application") ? userAppDir : userDirectoriesDir();
        QFileInfo fi(existingPath);
        QString base = fi.completeBaseName();
        QString candidate = dir + QLatin1Char('/') + base + ext;
        int count = 1;
        while (QFile::exists(candidate)) {
            candidate = dir + QLatin1Char('/') + base + QString::number(count) + ext;
            ++count;
        }
        return candidate;
    }

    // Novo arquivo: usa padrão
    QString ext, dir;
    if (type == QStringLiteral("Application")) {
        ext = QStringLiteral(".desktop");
        dir = userApplicationsDir();
    } else {
        ext = QStringLiteral(".directory");
        dir = userDirectoriesDir();
    }

    QString base;
    base = QStringLiteral("openkylinmenueditor-") + name.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
    if (base.endsWith(ext))
        base = base.left(base.length() - ext.length());
    QString candidate = dir + QLatin1Char('/') + base + ext;
    if (forceUpdate) return candidate;
    int count = 1;
    while (QFile::exists(candidate)) {
        candidate = dir + QLatin1Char('/') + base + QString::number(count) + ext;
        ++count;
    }
    return candidate;

}
