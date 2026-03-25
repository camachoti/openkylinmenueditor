#ifndef INTERFACE_H
#define INTERFACE_H

#include <QIcon>
#include <QWidget>
#include <QString>

enum FunType {
    MENUEDITOR = 0,
    TOTALMODULES,
};

class CommonInterface {
public:
    virtual ~CommonInterface() {}
    virtual QString plugini18nName() = 0;      // Plugin display name
    virtual int pluginTypes() = 0;             // Plugin type (FunType)
    virtual QWidget *pluginUi() = 0;           // Main plugin widget
    virtual bool isShowOnHomePage() const = 0; // Show on home page
    virtual QIcon icon() const = 0;            // Plugin icon
    virtual bool isEnable() const = 0;         // Plugin enabled
    virtual const QString name() const = 0;    // Module name

    virtual void plugin_leave() { return; }
};

#define CommonInterface_iid "org.ukmge.CommonInterface"

Q_DECLARE_INTERFACE(CommonInterface, CommonInterface_iid)

#endif // INTERFACE_H
