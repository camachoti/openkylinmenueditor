#include "menuwriter.h"
#include "menuparser.h"   // MenuModelRole, MenuItemType
#include "xdgpaths.h"

#include <QStandardItem>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QTextStream>
#include <QXmlStreamWriter>

// ── helpers internos ──────────────────────────────────────────────────────────

/**
 * Escreve recursivamente os blocos <Menu> para cada diretório filho.
 * Chamado antes de escrever o <Layout> do pai.
 */
static void writeSubMenus(QXmlStreamWriter &xml, QStandardItem *parentItem)
{
    for (int r = 0; r < parentItem->rowCount(); ++r) {
        QStandardItem *child = parentItem->child(r);
        if (!child) continue;
        int type = child->data(RoleType).toInt();
        if (type != static_cast<int>(MenuItemType::Directory)) continue;

        xml.writeStartElement(QStringLiteral("Menu"));

        QString name    = child->text();
        QString dirFile = child->data(RoleFilename).toString();

        xml.writeTextElement(QStringLiteral("Name"), name);

        if (!dirFile.isEmpty()) {
            xml.writeTextElement(QStringLiteral("Directory"),
                                 QFileInfo(dirFile).fileName());
        }

        // Submenus recursivos
        writeSubMenus(xml, child);

        // <Layout> deste diretório
        xml.writeStartElement(QStringLiteral("Layout"));
        xml.writeEmptyElement(QStringLiteral("Merge"));
        xml.writeAttribute(QStringLiteral("type"), QStringLiteral("menus"));

        for (int c = 0; c < child->rowCount(); ++c) {
            QStandardItem *item = child->child(c);
            if (!item) continue;
            int t        = item->data(RoleType).toInt();
            QString file = item->data(RoleFilename).toString();

            if (t == static_cast<int>(MenuItemType::Separator)) {
                xml.writeEmptyElement(QStringLiteral("Separator"));
            } else if (t == static_cast<int>(MenuItemType::Directory)) {
                // Referência ao submenu pelo nome
                xml.writeTextElement(QStringLiteral("Menuname"), item->text());
            } else if (t == static_cast<int>(MenuItemType::Application) ||
                       t == static_cast<int>(MenuItemType::Link)) {
                if (!file.isEmpty()) {
                    // O spec usa apenas o basename (com prefixo de subdir se necessário)
                    QString containingDir = QFileInfo(file).dir().dirName();
                    QString baseName      = QFileInfo(file).fileName();
                    QString desktopId     = (containingDir != QStringLiteral("applications"))
                                           ? containingDir + QLatin1Char('-') + baseName
                                           : baseName;
                    xml.writeTextElement(QStringLiteral("Filename"), desktopId);
                }
            }
        }

        xml.writeEmptyElement(QStringLiteral("Merge"));
        xml.writeAttribute(QStringLiteral("type"), QStringLiteral("files"));
        xml.writeEndElement(); // Layout

        xml.writeEndElement(); // Menu
    }
}

/**
 * Escreve o bloco <Layout> para os filhos directos de parentItem
 * (o nível raiz do modelo — lista de categorias de topo).
 */
static void writeRootLayout(QXmlStreamWriter &xml,
                             QStandardItem   *rootItem)
{
    xml.writeStartElement(QStringLiteral("Layout"));
    // Sem <Merge type="menus"> no nível de topo (igual ao Python original)

    for (int r = 0; r < rootItem->rowCount(); ++r) {
        QStandardItem *child = rootItem->child(r);
        if (!child) continue;
        int type = child->data(RoleType).toInt();

        if (type == static_cast<int>(MenuItemType::Separator)) {
            xml.writeEmptyElement(QStringLiteral("Separator"));
        } else if (type == static_cast<int>(MenuItemType::Directory)) {
            xml.writeTextElement(QStringLiteral("Menuname"), child->text());
        } else {
            QString file = child->data(RoleFilename).toString();
            if (!file.isEmpty())
                xml.writeTextElement(QStringLiteral("Filename"),
                                     QFileInfo(file).fileName());
        }
    }

    xml.writeEndElement(); // Layout
}

// ── MenuWriter público ────────────────────────────────────────────────────────

QString MenuWriter::userMenuFilePath()
{
    QString menuFile = MenuParser::defaultMenuFile();
    return XdgPaths::userMenusDir() + QLatin1Char('/') + menuFile;
}

bool MenuWriter::saveModel(const QStandardItemModel *model)
{
    if (!model) return false;

    QString destPath = userMenuFilePath();
    QDir().mkpath(QFileInfo(destPath).absolutePath());

    QSaveFile file(destPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    // Obtém o arquivo de menu do sistema para fazer o merge
    QString menuFile    = MenuParser::defaultMenuFile();
    QString mergeFile   = XdgPaths::findSystemMenu(menuFile);

    // Detecta o nome do menu (e.g. "Applications", "Kf5" …)
    // A maioria dos prefixos usa "Applications" como nome raiz
    QString menuName = QStringLiteral("Applications");

    QByteArray buffer;
    QXmlStreamWriter xml(&buffer);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);

    xml.writeStartDocument(QStringLiteral("1.0"), false);
    // DOCTYPE não é suportado directamente pelo QXmlStreamWriter;
    // escrevemos o cabeçalho manualmente mais abaixo.

    xml.writeStartElement(QStringLiteral("Menu"));
    xml.writeTextElement(QStringLiteral("Name"), menuName);
    xml.writeEmptyElement(QStringLiteral("DefaultAppDirs"));
    xml.writeEmptyElement(QStringLiteral("DefaultDirectoryDirs"));
    xml.writeEmptyElement(QStringLiteral("DefaultMergeDirs"));

    if (!mergeFile.isEmpty()) {
        xml.writeStartElement(QStringLiteral("MergeFile"));
        xml.writeAttribute(QStringLiteral("type"), QStringLiteral("parent"));
        xml.writeCharacters(mergeFile);
        xml.writeEndElement();
    }

    QStandardItem *root = model->invisibleRootItem();

    // Blocos <Menu> dos subdiretórios
    writeSubMenus(xml, root);

    // <Layout> do nível raiz
    writeRootLayout(xml, root);

    xml.writeEndElement(); // Menu
    xml.writeEndDocument();

    // Junta cabeçalho + corpo
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
           "<!DOCTYPE Menu\n"
           "  PUBLIC '-//freedesktop//DTD Menu 1.0//EN'\n"
           "  'http://standards.freedesktop.org/menu-spec/menu-1.0.dtd'>\n";

    // Remove a linha <?xml ... ?> gerada pelo QXmlStreamWriter (já escrevemos a nossa)
    QString body = QString::fromUtf8(buffer);
    int newline = body.indexOf(QLatin1Char('\n'));
    if (newline >= 0)
        body = body.mid(newline + 1);
    out << body;

    return file.commit();
}
