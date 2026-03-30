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

bool DesktopFileHandler::save(const DesktopEntry &entry, const QString &filePath, QString *errorMsg) {
    qDebug() << "[DEBUG] ======== DesktopFileHandler::save ========";
    qDebug() << "[DEBUG] filePath arg:" << filePath << ", entry.filePath:" << entry.filePath;

    QString outPath = filePath.isEmpty() ? entry.filePath : filePath;
    if (outPath.isEmpty()) {
        qDebug() << "[DEBUG] Return false: outPath empty.";
        if (errorMsg) *errorMsg = QObject::tr("No destination path specified.");
        return false;
    }
    // Try normal save first
    QSaveFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "[DEBUG] QSaveFile open failed, error:" << file.errorString();
        qDebug() << "[DEBUG] QSaveFile error enum:" << file.error();
    }
    if (file.isOpen()) {
        QTextStream out(&file);
        out << "[Desktop Entry]\n";
        for (auto it = entry.keys.constBegin(); it != entry.keys.constEnd(); ++it)
            out << it.key() << '=' << it.value() << "\n";
        if (file.commit()) {
            qDebug() << "[DEBUG] QSaveFile commit success!";
            return true;
        } else {
            qDebug() << "[DEBUG] QSaveFile commit failed for:" << outPath << "err:" << file.errorString();
            if (errorMsg) *errorMsg = QObject::tr("Failed to commit to file: ") + file.errorString();
        }
    }
    // If failed and it's a system dir, try privilege escalation (pkexec cp)
    QFileInfo fi(outPath);
    if (file.error() != QFile::NoError) {
        qDebug() << "[DEBUG] Falha de permissão, tentando pkexec.";
        QTemporaryFile tmpFile(QDir::tempPath() + "/XXXXXX.desktop");
        if (!tmpFile.open()) {
            qDebug() << "[DEBUG] Falha ao criar arquivo temporário para pkexec save.";
            if (errorMsg) *errorMsg = QObject::tr("Could not create temporary file for privileged save.");
            return false;
        }
        QTextStream out(&tmpFile);
        out << "[Desktop Entry]\n";
        for (auto it = entry.keys.constBegin(); it != entry.keys.constEnd(); ++it)
            out << it.key() << '=' << it.value() << "\n";
        out.flush();
        tmpFile.flush();
        tmpFile.close();
        QString tmpPath = tmpFile.fileName();
        // Use pkexec env ... cp to target (for polkit compatibility)
        QString display = QString::fromLocal8Bit(qgetenv("DISPLAY"));
        QString xauth = QString::fromLocal8Bit(qgetenv("XAUTHORITY"));
        QString dbus = QString::fromLocal8Bit(qgetenv("DBUS_SESSION_BUS_ADDRESS"));
        QStringList args;
        args << "env";
        if (!display.isEmpty()) args << "DISPLAY=" + display;
        if (!xauth.isEmpty()) args << "XAUTHORITY=" + xauth;
        if (!dbus.isEmpty()) args << "DBUS_SESSION_BUS_ADDRESS=" + dbus;
        args << "cp" << tmpPath << outPath;
        QProcess proc;
        qDebug() << "[PKEXEC DEBUG] Will run: pkexec" << args;
        proc.start("pkexec", args);
        bool started = proc.waitForStarted(5000);
        if (!started) {
            qDebug() << "[PKEXEC DEBUG] Could not start pkexec. QProcess error:" << proc.error();
            if (errorMsg)
                *errorMsg = QObject::tr("Could not start pkexec process. Error code: %1").arg((int)proc.error());
            QFile::remove(tmpPath);
            qDebug() << "[DEBUG] Return false: pkexec did not start.";
            return false;
        }
        bool finished = proc.waitForFinished(-1);
        qDebug() << "[PKEXEC DEBUG] finished?" << finished << ", exitCode:" << proc.exitCode() << ", exitStatus:" << proc.exitStatus();
        qDebug() << "[PKEXEC DEBUG] stderr:" << QString::fromUtf8(proc.readAllStandardError());
        qDebug() << "[PKEXEC DEBUG] stdout:" << QString::fromUtf8(proc.readAllStandardOutput());
        QString stderrOut = QString::fromUtf8(proc.readAllStandardError());
        if (stderrOut.isEmpty()) stderrOut = QString::fromUtf8(proc.readAllStandardOutput());
        if (!(proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)) {
    qDebug() << "[DEBUG] Return false: unknown file save failure.";
    if (errorMsg)
        *errorMsg = QObject::tr("Unknown file save failure.");
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
bool DesktopFileHandler::remove(const QString &filePath, QString *errorMsg)
{
    qDebug() << "[DEBUG] ======== DesktopFileHandler::remove ========";
    qDebug() << "[DEBUG] filePath arg:" << filePath;

    if (filePath.isEmpty()) {
        qDebug() << "[DEBUG] Return false: filePath empty.";
        if (errorMsg) *errorMsg = QObject::tr("No file path specified.");
        return false;
    }

    QFileInfo fi(filePath);
    if (!fi.exists()) {
        qDebug() << "[DEBUG] File does not exist, consider removed.";
        return true; // Already gone
    }

    QFile f(filePath);
    if (f.remove()) {
        qDebug() << "[DEBUG] QFile::remove success!";
        return true;
    } else {
        qDebug() << "[DEBUG] QFile::remove failed, error:" << f.errorString();
        qDebug() << "[DEBUG] QFile error enum:" << f.error();
    }

    // If permission denied (system dir), try pkexec rm
    if (f.error() != QFile::NoError) {
        qDebug() << "[DEBUG] Falha de permissão, tentando pkexec para remoção.";
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
        qDebug() << "[PKEXEC DEBUG] Will run: pkexec" << args;
        proc.start("pkexec", args);
        bool started = proc.waitForStarted(5000);
        if (!started) {
            qDebug() << "[PKEXEC DEBUG] Could not start pkexec. QProcess error:" << proc.error();
            if (errorMsg)
                *errorMsg = QObject::tr("Could not start pkexec process. Error code: %1").arg((int)proc.error());
            qDebug() << "[DEBUG] Return false: pkexec did not start.";
            return false;
        }
        bool finished = proc.waitForFinished(-1);
        qDebug() << "[PKEXEC DEBUG] finished?" << finished << ", exitCode:" << proc.exitCode() << ", exitStatus:" << proc.exitStatus();
        qDebug() << "[PKEXEC DEBUG] stderr:" << QString::fromUtf8(proc.readAllStandardError());
        qDebug() << "[PKEXEC DEBUG] stdout:" << QString::fromUtf8(proc.readAllStandardOutput());
        if (!(proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)) {
            qDebug() << "[DEBUG] Return false: unknown file remove failure.";
            if (errorMsg)
                *errorMsg = QObject::tr("Unknown file remove failure.");
            return false;
        }
        return true;
    }
    return false;
}
