#pragma once
#include <QScrollArea>
#include <QWidget>
#include <QMap>
#include "../backend/desktopfilehandler.h"

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QToolButton;
class QGridLayout;
class QVBoxLayout;
class QTabWidget;

/**
 * ApplicationEditorWidget — right-panel property editor for a single
 * launcher (.desktop) or directory (.directory) entry.
 *
 * Mirrors openkylinmenueditor_qt/editor.py: class ApplicationEditor.
 *
 * Signals
 * -------
 * entryChanged(filePath)  — user edited and applied changes; filePath is
 *                           the path that was written to disk.
 * saveRequested()         — user pressed Save
 * closeRequested()        — user wants to go back / close editor
 */
class ApplicationEditorWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit ApplicationEditorWidget(QWidget *parent = nullptr);

    /** Load and display the given desktop file. */
    void loadEntry(const QString &filePath);

    /** Load a pre-constructed entry (for new launchers not yet on disk). */
    void loadEntry(const DesktopEntry &entry);

    /** Return the entry currently being edited. */
    DesktopEntry currentEntry() const { return m_entry; }

    /** Apply the current form values to m_entry and save to disk.
     *  Returns the path written, or empty on failure. */
    QString applyAndSave();

    /** Give keyboard focus to the Name field (used when creating new entries). */
    void focusNameField();

signals:
    void entryChanged(const QString &filePath);
    void saveRequested();
    void closeRequested();
    void cloneRequested(const DesktopEntry &entry);

private slots:
    void onIconButtonClicked();
    void onSave();
    void onClone();
    void onFieldEdited();

private:
    void buildUi();
    //bool isSystemFile(const QString& filePath) const; // (no longer needed for now)

    QWidget     *m_content  = nullptr;
    QVBoxLayout *m_vlay     = nullptr;

    // Header
    QToolButton *m_iconBtn  = nullptr;
    QLabel      *m_typeLbl  = nullptr;
    QLabel      *m_fileLbl  = nullptr;

    // Basic fields
    QLineEdit   *m_nameEdit     = nullptr;
    QLineEdit   *m_execEdit     = nullptr;
    QLineEdit   *m_commentEdit  = nullptr;
    QLineEdit   *m_categoriesEdit = nullptr;
    QCheckBox   *m_terminalChk  = nullptr;
    QCheckBox   *m_noDisplayChk = nullptr;

    // Advanced tab (generic name, keywords, etc.)
    QMap<QString, QLineEdit*> m_advancedEdits;

    QPushButton *m_saveBtn  = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QPushButton *m_cloneBtn = nullptr;


    DesktopEntry m_entry;
    bool         m_loading = false;  // suprime sinais durante o carregamento
    bool         m_dirty   = false;  // há alterações não salvas
};
