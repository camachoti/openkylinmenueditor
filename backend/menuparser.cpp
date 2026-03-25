#include "menuparser.h"
#include "xdgpaths.h"
#include "desktopfilehandler.h"

#include <QStandardItem>
#include <QDir>
#include <QFileInfoList>
#include <QDebug>
#include <QDirIterator>
#include <QBrush>

QStandardItemModel *MenuParser::loadAllLaunchersModel(QObject *parent)
{
    QStringList paths = XdgPaths::itemSearchPaths();
    paths.prepend(XdgPaths::userApplicationsDir());
    QSet<QString> seen; // Avoid duplicates
    QList<QString> desktopFiles;
    
    // Collect all .desktop files recursively
    for (const QString &dirPath : paths) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;
        QFileInfoList files = dir.entryInfoList(QStringList() << "*.desktop", QDir::Files | QDir::NoSymLinks, QDir::Name);
        for (const QFileInfo &fi : files) {
            QString abs = fi.absoluteFilePath();
            if (!seen.contains(abs)) {
                desktopFiles << abs;
                seen.insert(abs);
            }
        }
        // Find recursively
        QDirIterator it(dirPath, QStringList() << "*.desktop", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString abs = it.next();
            if (!seen.contains(abs)) {
                desktopFiles << abs;
                seen.insert(abs);
            }
        }
    }

    QStandardItemModel *model = new QStandardItemModel(parent);
    model->setHorizontalHeaderLabels({QStringLiteral("Name"), "Exec", "Path", "NoDisplay", "Hidden", "OnlyShowIn"});

    for (const QString &filePath : desktopFiles) {
        DesktopEntry entry = DesktopFileHandler::load(filePath);
        if (!entry.isValid()) continue;
        const bool nodisplay = entry.getBool("NoDisplay");
        const bool hidden = entry.getBool("Hidden");
        QList<QStandardItem *> row;
        auto *nameItem = new QStandardItem(entry.getString("Name"));
        if (nodisplay || hidden) {
            nameItem->setForeground(QBrush(Qt::gray));
            nameItem->setToolTip("Normally hidden (NoDisplay/Hidden)");
        }
        row << nameItem
            << new QStandardItem(entry.getString("Exec"))
            << new QStandardItem(filePath)
            << new QStandardItem(nodisplay ? "true" : "false")
            << new QStandardItem(hidden ? "true" : "false")
            << new QStandardItem(entry.getList("OnlyShowIn").join(";"));
        model->appendRow(row);
    }
    return model;
}

QString MenuParser::defaultMenuFile()
{
    // Use prefix if set (e.g., "kf5-", "gnome-"), else standard fallback
    QString prefix = XdgPaths::defaultMenuPrefix();
    if (!prefix.isEmpty())
        return prefix + QStringLiteral("applications.menu");
    return QStringLiteral("applications.menu");
}

QStandardItemModel *MenuParser::loadModel(QObject *parent)
{
    // Resolve the menu filename
    QString menuFile = defaultMenuFile();
    const QString menuPath = XdgPaths::findSystemMenu(menuFile);
    if (menuPath.isEmpty())
        return nullptr;
    // === MINIMAL PLACEHOLDER: Model with menuFile as root node ===
    // Replace with full parser if/when one exists
    auto *model = new QStandardItemModel(parent);
    QStandardItem *root = new QStandardItem(menuFile);
    model->appendRow(root);
    // Optionally: parse XML menu and populate tree with categories/
    // applications (left as stub for now)
    return model;
}

#include <QDomDocument>
#include <QFile>
#include <QIcon>

// Aux: Carrega todos os launchers referenciados (inclusive ocultos) sob um nó/categoria
static void parseMenuRecursive(QDomElement elem, QStandardItem *parentItem, QSet<QString> *includedFiles)
{
    QDomNode node = elem.firstChild();
    while (!node.isNull()) {
        if (node.isElement()) {
            QDomElement e = node.toElement();
            if (e.tagName() == QLatin1String("Menu")) {
                // Recursivo: subdiretório/categoria
                QString catName = e.firstChildElement("Name").text();
                auto *catItem = new QStandardItem(catName);
                parentItem->appendRow(catItem);
                parseMenuRecursive(e, catItem, includedFiles);
            } else if (e.tagName() == QLatin1String("AppDir")) {
                // Directory de .desktop pode ser referenciada
                QDir d(e.text());
                QFileInfoList files = d.entryInfoList(QStringList() << "*.desktop", QDir::Files);
                for (auto &fi : files) {
                    DesktopEntry entry = DesktopFileHandler::load(fi.absoluteFilePath());
                    if (!entry.isValid()) continue;
                    QStandardItem *item = new QStandardItem(entry.getString("Name"));
                    // Icone
                    QString icon = entry.getString("Icon");
                    if (!icon.isEmpty()) item->setIcon(QIcon::fromTheme(icon));
                    // Marcação de oculto
                    bool nodisplay = entry.getBool("NoDisplay");
                    bool hidden = entry.getBool("Hidden");
                    if (nodisplay || hidden) {
                        item->setForeground(QBrush(Qt::gray));
                        item->setToolTip(QStringLiteral("Normalmente oculto (NoDisplay/Hidden)"));
                    }
                    // Coloca infos extras como dados
                    item->setData(fi.absoluteFilePath(), RoleFilename);
                    item->setData(entry.getString("Exec"), RoleExec);
                    item->setData((int)MenuItemType::Application, RoleType);
parentItem->appendRow(item);
if (includedFiles) includedFiles->insert(fi.absoluteFilePath());
                }
            } else if (e.tagName() == QLatin1String("Include")) {
                // Lista arquivos .desktop explicitamente
                QDomElement fElem = e.firstChildElement("Filename");
                while (!fElem.isNull()) {
                    QString file = fElem.text();
                    // Busca em todos os dirs XDG possíveis para mostrar TODOS os .desktop
                    QStringList allDirs = XdgPaths::itemSearchPaths();
                    allDirs.prepend(XdgPaths::userApplicationsDir());
                    QSet<QString> shownAbs;
                    for (const QString &dpath : allDirs) {
                        QString absPath = dpath + QLatin1Char('/') + file;
                        if (!QFileInfo::exists(absPath)) continue;
                        if (shownAbs.contains(absPath)) continue;
                        shownAbs.insert(absPath);
                        DesktopEntry entry = DesktopFileHandler::load(absPath);
                        if (!entry.isValid()) continue;
                        QStandardItem *item = new QStandardItem(entry.getString("Name"));
                        QString icon = entry.getString("Icon");
                        if (!icon.isEmpty()) item->setIcon(QIcon::fromTheme(icon));
                        bool nodisplay = entry.getBool("NoDisplay");
                        bool hidden = entry.getBool("Hidden");
                        if (nodisplay || hidden) {
                            item->setForeground(QBrush(Qt::gray));
                            item->setToolTip(QStringLiteral("Normalmente oculto (NoDisplay/Hidden)") + "\n" + absPath);
                        }
                        else {
                            item->setToolTip(absPath);
                        }
                        item->setData(absPath, RoleFilename);
                        item->setData(entry.getString("Exec"), RoleExec);
                        item->setData((int)MenuItemType::Application, RoleType);
parentItem->appendRow(item);
if (includedFiles) includedFiles->insert(absPath);
                    }
                    fElem = fElem.nextSiblingElement("Filename");
                }
                // Suporte básico para <Category>
                QDomElement catElem = e.firstChildElement("Category");
                while (!catElem.isNull()) {
                    QString cat = catElem.text();
                    // Para cada .desktop nos XDG paths, se Categories contém a categoria, inclui
                    QStringList itemPaths = XdgPaths::itemSearchPaths();
                    itemPaths.prepend(XdgPaths::userApplicationsDir());
                    QSet<QString> already;
                    for (const QString& dirPath : itemPaths) {
                        QDir dir(dirPath);
                        QFileInfoList files = dir.entryInfoList(QStringList() << "*.desktop", QDir::Files | QDir::NoSymLinks | QDir::Readable);
                        for (const QFileInfo& fi : files) {
                            if (already.contains(fi.absoluteFilePath())) continue;
                            DesktopEntry entry = DesktopFileHandler::load(fi.absoluteFilePath());
                            if (!entry.isValid()) continue;
                            QStringList cats = entry.getList("Categories");
                            if (!cats.contains(cat)) continue;
                            QStandardItem *item = new QStandardItem(entry.getString("Name"));
                            QString icon = entry.getString("Icon");
                            if (!icon.isEmpty()) item->setIcon(QIcon::fromTheme(icon));
                            bool nodisplay = entry.getBool("NoDisplay");
                            bool hidden = entry.getBool("Hidden");
                            if (nodisplay || hidden) {
                                item->setForeground(QBrush(Qt::gray));
                                item->setToolTip(QStringLiteral("Normalmente oculto (NoDisplay/Hidden)"));
                            }
                            item->setData(fi.absoluteFilePath(), RoleFilename);
                            item->setData(entry.getString("Exec"), RoleExec);
                            item->setData((int)MenuItemType::Application, RoleType);
parentItem->appendRow(item);
if (includedFiles) includedFiles->insert(fi.absoluteFilePath());
                            already.insert(fi.absoluteFilePath());
                        }
                    }
                    catElem = catElem.nextSiblingElement("Category");
                }
            }
        }
        node = node.nextSibling();
    }
}

QStandardItemModel *MenuParser::loadMenuIncludingHiddenModel(QObject *parent)
{
    QString menuFile = defaultMenuFile();
    QString menuPath = XdgPaths::findSystemMenu(menuFile);
    if (menuPath.isEmpty()) return nullptr;
    QFile file(menuPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return nullptr;
    QDomDocument doc;
    if (!doc.setContent(&file)) return nullptr;
    file.close();
    // Busca nó <Menu> raiz
    QDomElement rootElem = doc.documentElement();
    if (rootElem.tagName() != QLatin1String("Menu")) return nullptr;
    QStandardItemModel *model = new QStandardItemModel(parent);
    model->setHorizontalHeaderLabels({QStringLiteral("Menu")});
    auto *rootItem = new QStandardItem(menuFile);
    model->appendRow(rootItem);
    QSet<QString> includedFiles;
    parseMenuRecursive(rootElem, rootItem, &includedFiles);

    // Inclui órfãos diretamente no rootItem (sem "Others")
    QStringList paths = XdgPaths::itemSearchPaths();
    paths.prepend(XdgPaths::userApplicationsDir());
    QSet<QString> seen;
    for (const QString &dirPath : paths) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;
        QFileInfoList files = dir.entryInfoList(QStringList() << "*.desktop", QDir::Files | QDir::NoSymLinks | QDir::Readable);
        for (const QFileInfo &fi : files) {
            QString absPath = fi.absoluteFilePath();
            if (includedFiles.contains(absPath) || seen.contains(absPath)) continue;
            seen.insert(absPath);
            DesktopEntry entry = DesktopFileHandler::load(absPath);
            if (!entry.isValid()) continue;
            QStandardItem *item = new QStandardItem(entry.getString("Name"));
            QString icon = entry.getString("Icon");
            if (!icon.isEmpty()) item->setIcon(QIcon::fromTheme(icon));
            bool nodisplay = entry.getBool("NoDisplay");
            bool hidden = entry.getBool("Hidden");
            if (nodisplay || hidden) {
                item->setForeground(QBrush(Qt::gray));
                item->setToolTip(QStringLiteral("Normalmente oculto (NoDisplay/Hidden)"));
            }
            item->setData(absPath, RoleFilename);
            item->setData(entry.getString("Exec"), RoleExec);
            item->setData((int)MenuItemType::Application, RoleType);
            rootItem->appendRow(item);
        }
    }
    return model;
}

// End of file.


