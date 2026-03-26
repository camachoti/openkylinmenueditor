#pragma once
#include <QWidget>
#include "../backend/desktopfilehandler.h"

class MenuTreeWidget;
class ApplicationEditorWidget;
class QSplitter;
class QStackedWidget;
class QLabel;

/**
 * EditorWidget — top-level editor pane.
 *
 * Layout:  QSplitter
 *           ├── left:  MenuTreeWidget  (220px min)
 *           └── right: QStackedWidget
 *                        page 0: placeholder label (nothing selected)
 *                        page 1: ApplicationEditorWidget
 *
 * Connects the tree's itemSelected signal to load the selected
 * entry in the ApplicationEditorWidget.
 *
 * Mirrors the combined behaviour of openkylinmenueditor_qt/app.py
 * (the main QSplitter containing Treeview + ApplicationEditor).
 */
class EditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EditorWidget(QWidget *parent = nullptr);

    /** Reload the menu model from disk (e.g. after external change). */
    void reloadMenu();

signals:
    /** Emitted after an entry is saved to disk. */
    void entrySaved(const QString &filePath);

private slots:
    void onItemSelected(const QString &filename, int itemType);
    void onEntrySaved(const QString &filePath);
    void onAddLauncher();
    void onAddDirectory();
    void onAddSeparator();
    void onRemove();
    void onCloneRequested(const DesktopEntry &entry);

private:
    void buildUi();
    void loadMenu();

    QSplitter                  *m_splitter = nullptr;
    MenuTreeWidget             *m_tree     = nullptr;
    QStackedWidget             *m_stack    = nullptr;
    QLabel                     *m_placeholder = nullptr;
    ApplicationEditorWidget    *m_editor   = nullptr;
};
