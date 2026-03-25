/*
 * Theme and Icon Detection Logic for OpenKylinMenuEditor
 * -----------------------------------------------------
 * This application tries to automatically detect the system's dark/light mode
 * and icon theme settings. It uses GSettings queries with the following priorities:
 *  - UKUI: org.ukui.style theme, org.ukui.interface color-scheme, org.ukui.icon-theme name
 *  - GNOME: org.gnome.desktop.interface gtk-theme, color-scheme, icon-theme
 * The app also logs every detected value for debugging purposes. If the desktop environment
 * is not UKUI or GNOME, a fallback tries to read at least the GNOME icon theme and any available color-scheme.
 *
 * Only if dark mode is detected is a custom "dark palette" applied (Fusion). Otherwise the system palette is used.
 * The QSS (stylesheet) is currently disabled to avoid interfering with system palette for debugging purposes.
 */

#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QStyle>

QString runGsettings(const QString &schema, const QString &key) {
    QProcess process;
    process.start("gsettings", QStringList() << "get" << schema << key);
    if (!process.waitForFinished(1000)) {
        qDebug() << "[GSETTINGS] Timeout querying" << schema << key;
        return QString();
    }
    QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    QString error = QString::fromUtf8(process.readAllStandardError()).trimmed();
    if (!error.isEmpty()) {
        qDebug() << "[GSETTINGS][ERROR]" << error;
    }
    if (output.startsWith("'") && output.endsWith("'")) {
        output = output.mid(1, output.length() - 2);
    }
    return output;
}

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    a.setApplicationName("openkylinmenueditor");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("OpenKylin");

    // DETECÇÃO DE DARK MODE E TEMA DE ÍCONES (UKUI e fallback GNOME)
    QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower();
    QString theme, iconTheme;
    bool isDark = false;

    QString colorScheme;
// Always use GNOME keys for theme/color-scheme/icon-theme because UKUI doesn't provide them
// (Note: UKUI schemas do not offer usable theme information on this system)
theme = runGsettings("org.gnome.desktop.interface", "gtk-theme");
qDebug() << "[GSETTINGS] org.gnome.desktop.interface.gtk-theme:" << theme;
colorScheme = runGsettings("org.gnome.desktop.interface", "color-scheme");
qDebug() << "[GSETTINGS] org.gnome.desktop.interface.color-scheme:" << colorScheme;
isDark = theme.contains("dark", Qt::CaseInsensitive) || theme.contains("black", Qt::CaseInsensitive) || colorScheme.contains("prefer-dark", Qt::CaseInsensitive);
iconTheme = runGsettings("org.gnome.desktop.interface", "icon-theme");
qDebug() << "[GSETTINGS] org.gnome.desktop.interface.icon-theme:" << iconTheme;
if (desktop.contains("ukui")) {
    qDebug() << "[THEME DETECTION] XDG_CURRENT_DESKTOP is 'ukui', but using GNOME keys for theme sync.";
} else if (desktop.contains("gnome")) {
    qDebug() << "[THEME DETECTION] XDG_CURRENT_DESKTOP is 'gnome', using GNOME keys.";
} else {
    qDebug() << "[THEME DETECTION] Unknown desktop environment (" << desktop << "). Using GNOME keys as fallback.";
}
iconTheme = iconTheme.remove("'", Qt::CaseInsensitive);

    qDebug() << "[DETECTED] Desktop:" << desktop << "Theme:" << theme << "ColorScheme:" << colorScheme << "IconTheme:" << iconTheme << "isDark:" << isDark;
    qDebug() << "[ENV CHECK] XDG_CURRENT_DESKTOP=" << qgetenv("XDG_CURRENT_DESKTOP");

    if (!iconTheme.isEmpty()) {
        QIcon::setThemeName(iconTheme);
        qDebug() << "[QIcon] Set theme name to" << iconTheme;
    } else {
        qDebug() << "[QIcon] No icon theme could be set (empty string).";
    }

    // Remove Fusion to inherit system visual style (QStyle) from desktop/Qt platform plugin
    // Always inherit system visual style and palette
    // Never force custom palette or style; let qt5-ukui-platformtheme handle dark/light
    // Removed color/palette overrides for full native appearance

    // Load stylesheet from embedded resources (DESABILITADO TEMPORARIAMENTE)
    /*
    QFile qss(QStringLiteral(":/qss/openkylinmenueditor.qss"));
    if (qss.open(QFile::ReadOnly)) {
        a.setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }
    */

    // Load translations
    QTranslator translator;
    if (translator.load(QStringLiteral(":/i18n/") + QLocale::system().name())) {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
