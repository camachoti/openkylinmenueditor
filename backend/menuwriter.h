#pragma once
#include <QStandardItemModel>
#include <QString>

/**
 * MenuWriter — serializa o QStandardItemModel do menu para um arquivo
 * .menu XML compatível com a especificação freedesktop.org.
 *
 * O arquivo gerado é salvo em:
 *   ~/.config/menus/<prefix>applications.menu
 *
 * O formato segue a mesma estrutura produzida pelo
 * openkylinmenueditor/XmlMenuElementTree.py:
 *
 *   <Menu>
 *     <Name>Applications</Name>
 *     <DefaultAppDirs/>
 *     <DefaultDirectoryDirs/>
 *     <DefaultMergeDirs/>
 *     <MergeFile type="parent">/etc/xdg/menus/applications.menu</MergeFile>
 *     <Menu>
 *       <Name>Accessories</Name>
 *       <Directory>accessories.directory</Directory>
 *       <Layout>
 *         <Merge type="menus"/>
 *         <Filename>gedit.desktop</Filename>
 *         <Separator/>
 *         <Merge type="files"/>
 *       </Layout>
 *     </Menu>
 *     ...
 *     <Layout>
 *       <Menuname>Accessories</Menuname>
 *       ...
 *     </Layout>
 *   </Menu>
 *
 * Apenas a ordem dos itens (Layout) é escrita — as entradas em si
 * continuam sendo lidas via libmenu-cache.
 */
class MenuWriter
{
public:
    /**
     * Salva o modelo em ~/.config/menus/<prefix>applications.menu.
     * Retorna true em caso de sucesso.
     */
    static bool saveModel(const QStandardItemModel *model);

    /**
     * Retorna o caminho do arquivo de menu do utilizador que seria escrito.
     */
    static QString userMenuFilePath();
};
