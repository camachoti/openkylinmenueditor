#include "editorwidget.h"
#include "menutreewidget.h"
#include "applicationeditorwidget.h"
#include "../backend/menuparser.h"
#include "../backend/desktopfilehandler.h"
#include "../backend/xdgpaths.h"
#include "../backend/menuwriter.h"

#include <QSplitter>
#include <QStackedWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

EditorWidget::EditorWidget(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
    loadMenu();
}

void EditorWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(Qt::Horizontal, this);

    // ── left: menu tree ─────────────────────────────────────────────
    m_tree = new MenuTreeWidget(m_splitter);
    m_splitter->addWidget(m_tree);

    // ── right: stacked (placeholder / editor) ───────────────────────
    m_stack = new QStackedWidget(m_splitter);

    m_placeholder = new QLabel(tr("Select an item in the tree to edit it."));
    m_placeholder->setAlignment(Qt::AlignCenter);
    m_placeholder->setObjectName(QStringLiteral("editor_placeholder"));
    m_stack->addWidget(m_placeholder);  // index 0

    m_editor = new ApplicationEditorWidget(m_stack);
    m_stack->addWidget(m_editor);       // index 1

    m_splitter->addWidget(m_stack);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({240, 600});

    layout->addWidget(m_splitter);

    // ── wire up signals ──────────────────────────────────────────────
    connect(m_tree, &MenuTreeWidget::itemSelected,
            this,   &EditorWidget::onItemSelected);
    connect(m_editor, &ApplicationEditorWidget::entryChanged,
            this,     &EditorWidget::onEntrySaved);
    connect(m_editor, &ApplicationEditorWidget::closeRequested,
            this, [this]() { m_stack->setCurrentIndex(0); });
    connect(m_editor, &ApplicationEditorWidget::cloneRequested,
            this, &EditorWidget::onCloneRequested);
    connect(m_tree, &MenuTreeWidget::reloadRequired,
            this, &EditorWidget::reloadMenu);
    // ── toolbar actions ──────────────────────────────────────────────
    connect(m_tree, &MenuTreeWidget::addLauncherRequested,
            this,   &EditorWidget::onAddLauncher);
    connect(m_tree, &MenuTreeWidget::addDirectoryRequested,
            this,   &EditorWidget::onAddDirectory);
    connect(m_tree, &MenuTreeWidget::addSeparatorRequested,
            this,   &EditorWidget::onAddSeparator);
    connect(m_tree, &MenuTreeWidget::removeRequested,
            this,   &EditorWidget::onRemove);
}

void EditorWidget::onCloneRequested(const DesktopEntry &entry)
{
    m_editor->loadEntry(entry);
    m_stack->setCurrentIndex(1);
    m_editor->focusNameField();
}

void EditorWidget::loadMenu()
{
    // ATENÇÃO: para diagnóstico, mostra TODOS os launchers .desktop, inclusive ocultos.
    // Usa o novo modelo de diagnóstico: árvore completa, incluindo ocultos, com ícones e edição
    auto *model = MenuParser::loadMenuIncludingHiddenModel(this);
    m_tree->setMenuModel(model);
    m_stack->setCurrentIndex(0);
}

void EditorWidget::reloadMenu()
{
    // Persiste a ordem actual antes de recarregar do disco
    if (m_tree->menuModel())
        MenuWriter::saveModel(m_tree->menuModel());

    loadMenu();
}

void EditorWidget::onItemSelected(const QString &filename, int itemType)
{
    static QString s_prevLoadedFilePath;
    static QString s_prevLoadedBaseName;
    static bool    s_prevWasSystemFile = false;

    if (filename.isEmpty() || itemType == static_cast<int>(MenuItemType::Separator)) {
        m_stack->setCurrentIndex(0);
        return;
    }

    // Track previous loaded entry's info for save logic
    s_prevLoadedFilePath = filename;
    QFileInfo fi(filename);
    s_prevLoadedBaseName = fi.fileName();
    QString userAppDir = QDir::homePath() + "/.local/share/applications/";
    s_prevWasSystemFile = !filename.startsWith(userAppDir);

    m_editor->loadEntry(filename);
    m_stack->setCurrentIndex(1);
}

void EditorWidget::onEntrySaved(const QString &filePath)
{
    // If this was an edit of a system launcher, and its basename changed, create a stub .desktop (Hidden=true) for the old name
    // Access the previous file path info stored by onItemSelected
    static QString s_prevLoadedFilePath;
    static QString s_prevLoadedBaseName;
    static bool    s_prevWasSystemFile = false;

    QString userAppDir = QDir::homePath() + "/.local/share/applications/";
    QFileInfo fi(filePath);
    QString newBaseName = fi.fileName();

    // Only relevant if this edit originated from a system file (not user/appdir)
    if (s_prevWasSystemFile && !s_prevLoadedBaseName.isEmpty()
        && newBaseName != s_prevLoadedBaseName) {
        QString stubPath = userAppDir + s_prevLoadedBaseName;
        QFile stub(stubPath);
        if (!QFile::exists(stubPath)) {
            if (stub.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QTextStream ts(&stub);
                ts << "[Desktop Entry]" << Qt::endl;
                ts << "Hidden=true" << Qt::endl;
                stub.close();
            }
        }
        // Show user a confirmation dialog about hiding the original system launcher
        QMessageBox::information(this,
            tr("Hiding original system launcher"),
            tr("You renamed a system launcher. The original system launcher will be hidden in your menu using a user-local Hidden=true file. No system files are affected."));
    }

    // Reset tracking after use
    s_prevWasSystemFile = false;
    s_prevLoadedFilePath.clear();
    s_prevLoadedBaseName.clear();


    // Reload the menu so newly added/edited launchers appear immediately
    reloadMenu();
    // Re-select the updated item so the tree icon/name refreshes
    m_tree->selectByFilename(filePath);
    emit entrySaved(filePath);
}

// ── toolbar action handlers ───────────────────────────────────────────────────

void EditorWidget::onAddLauncher()
{
    DesktopEntry entry = DesktopFileHandler::newApplication();
    m_editor->loadEntry(entry);
    m_stack->setCurrentIndex(1);
    m_editor->focusNameField();
}

void EditorWidget::onAddDirectory()
{
    DesktopEntry entry = DesktopFileHandler::newDirectory();
    m_editor->loadEntry(entry);
    m_stack->setCurrentIndex(1);
    m_editor->focusNameField();
}

void EditorWidget::onAddSeparator()
{
    // Separators have no editable desktop file — just add one in-memory
    // after the currently selected item and trigger a model refresh so
    // it appears in the tree.  libmenu-cache will not persist these unless
    // we write the .menu XML (handled by MenuWriter when saving order).
    QStandardItemModel *model = m_tree->menuModel();
    if (!model) return;

    // Find the parent of the current selection (or invisible root)
    QModelIndex currentProxy = m_tree->currentProxyIndex();
    QStandardItem *parentItem = nullptr;
    int insertRow = 0;

    if (currentProxy.isValid()) {
        QModelIndex srcCurrent = m_tree->proxyToSource(currentProxy);
        QStandardItem *srcItem = model->itemFromIndex(srcCurrent);
        if (srcItem) {
            QStandardItem *p = srcItem->parent()
                ? srcItem->parent()
                : model->invisibleRootItem();
            parentItem = p;
            insertRow  = srcCurrent.row() + 1;
        }
    }
    if (!parentItem)
        parentItem = model->invisibleRootItem();

    auto *sep = new QStandardItem(QStringLiteral("─────"));
    sep->setEditable(false);
    sep->setData(static_cast<int>(MenuItemType::Separator), RoleType);
    sep->setData(QString{}, RoleFilename);
    sep->setData(false,     RoleShow);
    parentItem->insertRow(insertRow, sep);
    m_tree->expandAll();
}

void EditorWidget::onRemove()
{
    QString filename = m_tree->selectedFilename();
    if (filename.isEmpty()) return;

    int itemType = m_tree->selectedItemType();

    // Ask confirmation
    bool isUserLauncher = filename.startsWith(QDir::homePath() + "/.local/share/applications/");
    QString baseName = QFileInfo(filename).baseName();
    QString message;
    if (isUserLauncher) {
        message = tr("Remove '%1' from your user area?\n\nThis will permanently delete your personalized shortcut. This action cannot be undone.").arg(baseName);
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Remove entry"),
            message,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
        QFile::remove(filename);
    } else {
        // System file: ask user for privileged removal
        QString sysRemoveMsg = tr("This launcher is located in a system directory. Do you want to permanently remove the original file?");
        QMessageBox::StandardButton sysReply = QMessageBox::question(
            this,
            tr("Remove system launcher"),
            sysRemoveMsg,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (sysReply != QMessageBox::Yes) return;
        // Try privileged removal using DesktopFileHandler
        if (!DesktopFileHandler::remove(filename)) {
            QMessageBox::warning(this,
                tr("Failed to remove system launcher"),
                tr("Could not remove the system launcher. Try running as administrator."));
            return;
        }
    }

    // Remove item from model and reload
    reloadMenu();
    m_stack->setCurrentIndex(0);
}
