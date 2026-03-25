#include "desktopfilehandler.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QProcess>
#include <QStringList>
#include <QDebug>

// --- DesktopEntry member implementations ---
QString DesktopEntry::getString(const QString &key, const QString &defaultVal) const {
    return keys.value(key, defaultVal);
}
bool DesktopEntry::getBool(const QString &key, bool defaultVal) const {
    QString v = keys.value(key).toLower();
    if (v == "true" || v == "1" || v == "yes" || v == "on") return true;
    if (v == "false" || v == "0" || v == "no" || v == "off") return false;
    return defaultVal;
}
QStringList DesktopEntry::getList(const QString &key) const {
    QString v = keys.value(key);
    if (v.isEmpty()) return {};
    QStringList list = v.split(';', Qt::SkipEmptyParts);
    for (auto &item : list) item = item.trimmed();
    return list;
}
void DesktopEntry::setString(const QString &key, const QString &value) {
    keys.insert(key, value);
}
void DesktopEntry::setBool(const QString &key, bool value) {
    keys.insert(key, value ? "true" : "false");
}
void DesktopEntry::setList(const QString &key, const QStringList &values) {
    keys.insert(key, values.join(";") + (values.isEmpty() ? "" : ";"));
}
bool DesktopEntry::isValid() const {
    return keys.contains("Type") && keys.contains("Name") && !keys.value("Type").isEmpty() && !keys.value("Name").isEmpty();
}

// --- DesktopFileHandler static methods ---

#include <QTextStream>

DesktopEntry DesktopFileHandler::load(const QString &filePath) {
    DesktopEntry entry;
    entry.filePath = filePath;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return entry;
    QTextStream in(&f);
    bool inDesktopEntry = false;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        if (line == "[Desktop Entry]") {
            inDesktopEntry = true;
            continue;
        }
        if (line.startsWith("[")) {
            if (inDesktopEntry) break; // Exits the [Desktop Entry] group
            continue;
        }
        if (inDesktopEntry) {
            int eq = line.indexOf('=');
            if (eq > 0) {
                QString k = line.left(eq).trimmed();
                QString v = line.mid(eq+1).trimmed();
                entry.keys.insert(k, v);
            }
        }
    }
    return entry;
}

bool DesktopFileHandler::save(const DesktopEntry &entry, const QString &filePath) {
    QString outPath = filePath.isEmpty() ? entry.filePath : filePath;
    if (outPath.isEmpty()) return false;
    // Try normal save first
    QSaveFile file(outPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "[Desktop Entry]\n";
        for (auto it = entry.keys.constBegin(); it != entry.keys.constEnd(); ++it)
            out << it.key() << '=' << it.value() << "\n";
        if (file.commit()) return true;
    }
    // If failed and it's a system dir, try privilege escalation (pkexec cp)
    QFileInfo fi(outPath);
    if (file.error() == QFile::PermissionsError || file.error() == QFileDevice::OpenError) {
        QTemporaryFile tmpFile(QDir::tempPath() + "/XXXXXX.desktop");
        if (!tmpFile.open()) return false;
        QTextStream out(&tmpFile);
        out << "[Desktop Entry]\n";
        for (auto it = entry.keys.constBegin(); it != entry.keys.constEnd(); ++it)
            out << it.key() << '=' << it.value() << "\n";
        out.flush();
        tmpFile.flush();
        tmpFile.close();
        QString tmpPath = tmpFile.fileName();
        // Use pkexec to copy to target
        QStringList args;
        args << "cp" << tmpPath << outPath;
        QProcess proc;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        if (!qgetenv("DISPLAY").isEmpty())
            env.insert("DISPLAY", QString::fromLocal8Bit(qgetenv("DISPLAY")));
        if (!qgetenv("XAUTHORITY").isEmpty())
            env.insert("XAUTHORITY", QString::fromLocal8Bit(qgetenv("XAUTHORITY")));
        proc.setProcessEnvironment(env);
        proc.start("pkexec", args);
        bool started = proc.waitForStarted(5000);
        if (!started) {
            qWarning("Could not start pkexec cp process");
            qWarning("QProcess error: %d", (int)proc.error());
            QFile::remove(tmpPath);
            return false;
        }
        proc.waitForFinished(-1);
        QString stderrOut = QString::fromUtf8(proc.readAllStandardError());
        if (stderrOut.isEmpty()) stderrOut = QString::fromUtf8(proc.readAllStandardOutput());
        if (!(proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)) {
            if (!stderrOut.isEmpty())
                qWarning() << "pkexec cp failed, stderr:" << stderrOut;
            QFile::remove(tmpPath);
            return false;
        }
        QFile::remove(tmpPath);
        return true;
    }
    return false;
}

DesktopEntry DesktopFileHandler::newApplication(const QString &name) {
    DesktopEntry entry;
    entry.setString("Type", "Application");
    entry.setString("Name", name);
    entry.setString("Exec", "");
    entry.setString("Icon", "application-default-icon");
    entry.setBool("Terminal", false);
    return entry;
}

DesktopEntry DesktopFileHandler::newDirectory(const QString &name) {
    DesktopEntry entry;
    entry.setString("Type", "Directory");
    entry.setString("Name", name);
    return entry;
}


// Remove a launcher file, with privilege fallback (pkexec) for system directories
bool DesktopFileHandler::remove(const QString &filePath)
{
    if (filePath.isEmpty()) return false;

    QFileInfo fi(filePath);
    if (!fi.exists())
        return true; // Already gone

    QFile f(filePath);
    if (f.remove())
        return true;

    // If permission denied (system dir), try pkexec rm
    if (f.error() == QFile::PermissionsError || f.error() == QFileDevice::OpenError) {
        // Compose env-invocation inline to pkexec for maximal session compatibility
        QString display = QString::fromLocal8Bit(qgetenv("DISPLAY"));
        QString xauth = QString::fromLocal8Bit(qgetenv("XAUTHORITY"));
        QString dbus = QString::fromLocal8Bit(qgetenv("DBUS_SESSION_BUS_ADDRESS"));
        QStringList args;
        args << "env";
        if (!display.isEmpty()) args << "DISPLAY=" + display;
        if (!xauth.isEmpty()) args << "XAUTHORITY=" + xauth;
        if (!dbus.isEmpty()) args << "DBUS_SESSION_BUS_ADDRESS=" + dbus;
        args << "rm" << "-f" << filePath;
        QProcess proc;
        proc.start("pkexec", args);
        bool started = proc.waitForStarted(5000);
        if (!started) {
            qWarning("Could not start pkexec process");
            qWarning("QProcess error: %d", (int)proc.error());
            return false;
        }
        proc.waitForFinished(-1);
        if (!(proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)) {
            QString stderrOut = QString::fromUtf8(proc.readAllStandardError());
            if (stderrOut.isEmpty()) stderrOut = QString::fromUtf8(proc.readAllStandardOutput());
            if (!stderrOut.isEmpty())
                qWarning() << "pkexec rm failed, stderr:" << stderrOut;
            return false;
        }
        return true;
    }
    return false;
}
