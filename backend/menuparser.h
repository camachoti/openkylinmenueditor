#pragma once
#include <QStandardItemModel>
#include <QString>
#include <QObject>

/**
 * MenuItemType — mirrors openkylinmenueditor/util.py MenuItemTypes enum.
 */
enum class MenuItemType : int {
    Separator   = -1,
    Application =  0,
    Link        =  1,
    Directory   =  2
};

/**
 * Column indices — match COL_* constants in menu_model.py / MenuEditor.py.
 * A single-column model is used; extra data is stored in Qt::UserRole + N.
 */
enum MenuModelColumn { COL_NAME = 0 };

/**
 * Qt::UserRole offsets used for metadata (match ROLE_* in menu_model.py).
 */
enum MenuModelRole {
    RoleType       = Qt::UserRole + 1,
    RoleFilename   = Qt::UserRole + 2,
    RoleShow       = Qt::UserRole + 3,
    RoleExec       = Qt::UserRole + 4,
    RoleCategories = Qt::UserRole + 5,
    RoleComment    = Qt::UserRole + 6,
    RoleIconName   = Qt::UserRole + 7,
};

/**
 * MenuParser — loads the system application menu via GMenu (GLib C API)
 * and exposes it as a QStandardItemModel.
 *
 * Mirrors openkylinmenueditor/MenuEditor.py: get_menus() + get_submenus()
 * and openkylinmenueditor_qt/menu_model.py: get_model() + build_model().
 *
 * No GTK widgets are used; only GMenu / Gio via their C API.
 */
class MenuParser
{
public:
    /**
     * Load the system application menu and return a new QStandardItemModel.
     * The caller owns the returned model.
     * Returns nullptr on hard failure.
     */
    static QStandardItemModel *loadModel(QObject *parent = nullptr);

    /**
     * Return the filename of the default applications.menu file,
     * e.g. "kf5-applications.menu" or "applications.menu".
     * Mirrors MenuEditor.py get_default_menu().
     */
    static QString defaultMenuFile();

    /**
     * Load all .desktop files from all XDG application directories (ignoring NoDisplay/Hidden).
     * Used for diagnostic "show all launchers" mode.
     */
    static QStandardItemModel *loadAllLaunchersModel(QObject *parent = nullptr);
    // Novo: Monta árvore completa, incluindo ocultos
    static QStandardItemModel *loadMenuIncludingHiddenModel(QObject *parent = nullptr);
};
