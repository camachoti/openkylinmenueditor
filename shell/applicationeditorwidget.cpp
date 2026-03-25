#include "applicationeditorwidget.h"
#include "../backend/desktopfilehandler.h"
#include "../backend/xdgpaths.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QTabWidget>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>
#include <QFont>
#include <QFileInfo>
#include <QFontMetrics>

// ── small helpers ─────────────────────────────────────────────────────────────

static QFrame *makeSeparator()
{
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::NoFrame);
    return sep;
}

static QWidget *makeFieldRow(const QString &labelText, QLineEdit *edit,
                              QWidget *parent = nullptr,
                              const QString &objectName = QString())
{
    Q_UNUSED(parent)
    auto *row = new QWidget;
    auto *h = new QHBoxLayout(row);
    h->setContentsMargins(12, 8, 12, 8);
    auto *lbl = new QLabel(labelText);
    if (!objectName.isEmpty())
        lbl->setObjectName(objectName);
    lbl->setMinimumWidth(140);
    lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    h->addWidget(lbl);
    h->addWidget(edit, 1);
    return row;
}

// ── ApplicationEditorWidget ───────────────────────────────────────────────────

ApplicationEditorWidget::ApplicationEditorWidget(QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    buildUi();
}

#include <QDir>

void ApplicationEditorWidget::buildUi()
{
    m_content = new QWidget;
    m_vlay = new QVBoxLayout(m_content);
    m_vlay->setContentsMargins(16, 16, 16, 16);
    m_vlay->setSpacing(8);

    // ── header: icon + type + file path ──────────────────────────────
    auto *headerRow = new QWidget;
    auto *headerH = new QHBoxLayout(headerRow);
    headerH->setContentsMargins(0, 0, 0, 0);
    headerH->setSpacing(12);

    m_iconBtn = new QToolButton;
    m_iconBtn->setObjectName(QStringLiteral("entry_icon_btn"));
    m_iconBtn->setFixedSize(64, 64);
    m_iconBtn->setIconSize(QSize(48, 48));
    m_iconBtn->setToolTip(tr("Click to change icon"));
    connect(m_iconBtn, &QToolButton::clicked,
            this, &ApplicationEditorWidget::onIconButtonClicked);
    headerH->addWidget(m_iconBtn);

    auto *headerInfo = new QVBoxLayout;
    m_typeLbl = new QLabel;
    m_typeLbl->setObjectName(QStringLiteral("entry_type_label"));
    QFont boldFont = m_typeLbl->font();
    boldFont.setBold(true);
    boldFont.setPointSizeF(boldFont.pointSizeF() * 1.1);
    m_typeLbl->setFont(boldFont);
    headerInfo->addWidget(m_typeLbl);

    m_fileLbl = new QLabel;
    m_fileLbl->setObjectName(QStringLiteral("entry_file_label"));
    m_fileLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_fileLbl->setWordWrap(true);
    headerInfo->addWidget(m_fileLbl);
    headerInfo->addStretch(1);
    headerH->addLayout(headerInfo, 1);
    m_vlay->addWidget(headerRow);

    // Modern visual: menos separadores
    // m_vlay->addWidget(makeSeparator());

    // ── basic fields ──────────────────────────────────────────────────
    auto *basicCard = new QFrame;
    basicCard->setObjectName(QStringLiteral("basic_card"));
    basicCard->setFrameShape(QFrame::NoFrame);
    auto *basicLayout = new QVBoxLayout(basicCard);
    basicLayout->setContentsMargins(0, 0, 0, 0);
    basicLayout->setSpacing(0);

    m_nameEdit       = new QLineEdit;
    m_execEdit       = new QLineEdit;
    m_commentEdit    = new QLineEdit;
    m_categoriesEdit = new QLineEdit;

    basicLayout->addWidget(makeFieldRow(tr("Name"),       m_nameEdit, nullptr, QStringLiteral("label_name")));
    basicLayout->addWidget(makeSeparator());
    basicLayout->addWidget(makeFieldRow(tr("Command"),    m_execEdit, nullptr, QStringLiteral("label_command")));
    basicLayout->addWidget(makeSeparator());
    basicLayout->addWidget(makeFieldRow(tr("Comment"),    m_commentEdit, nullptr, QStringLiteral("label_comment")));
    basicLayout->addWidget(makeSeparator());
    basicLayout->addWidget(makeFieldRow(tr("Categories"), m_categoriesEdit, nullptr, QStringLiteral("label_categories")));

    // Checkboxes
    auto *chkRow = new QWidget;
    auto *chkH = new QHBoxLayout(chkRow);
    chkH->setContentsMargins(12, 8, 12, 8);
    m_terminalChk  = new QCheckBox(tr("Run in terminal"));
    m_noDisplayChk = new QCheckBox(tr("Hide from menus"));
    chkH->addWidget(m_terminalChk);
    chkH->addSpacing(16);
    chkH->addWidget(m_noDisplayChk);
    chkH->addStretch(1);
    basicLayout->addWidget(makeSeparator());
    basicLayout->addWidget(chkRow);

    m_vlay->addWidget(basicCard);

    // ── advanced tab ─────────────────────────────────────────────────
    auto *tabs = new QTabWidget;
    tabs->setObjectName(QStringLiteral("editor_tabs"));

    auto *advancedWidget = new QWidget;
    auto *advLayout = new QVBoxLayout(advancedWidget);
    advLayout->setContentsMargins(0, 8, 0, 0);
    advLayout->setSpacing(0);

    QStringList advancedKeys = {
        QStringLiteral("GenericName"),
        QStringLiteral("Keywords"),
        QStringLiteral("MimeType"),
        QStringLiteral("OnlyShowIn"),
        QStringLiteral("NotShowIn"),
        QStringLiteral("StartupWMClass"),
        QStringLiteral("TryExec"),
        QStringLiteral("Version"),
    };
    for (const QString &key : advancedKeys) {
        auto *edit = new QLineEdit;
        m_advancedEdits[key] = edit;
        QString oname = key;
        oname.replace(' ', '_');
        advLayout->addWidget(makeFieldRow(key, edit, nullptr, QStringLiteral("label_") + oname));
        advLayout->addWidget(makeSeparator());
        connect(edit, &QLineEdit::textEdited,
                this, &ApplicationEditorWidget::onFieldEdited);
    }
    // -- (REMOVIDO) Advanced Overwrite System File Checkbox ---
    // m_overwriteSystemChk = ... (removido)
    // advLayout->addWidget(m_overwriteSystemChk);

    advLayout->addStretch(1);
    tabs->addTab(advancedWidget, tr("Advanced"));
    m_vlay->addWidget(tabs);

    // ── buttons ───────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch(1);
    m_closeBtn = new QPushButton(tr("Close"));
    m_closeBtn->setObjectName(QStringLiteral("close_btn"));
    m_saveBtn  = new QPushButton(tr("Save"));
    m_saveBtn->setObjectName(QStringLiteral("save_btn"));
    m_saveBtn->setDefault(true);
    m_saveBtn->setEnabled(false);  // habilitado apenas quando houver alterações
    btnRow->addWidget(m_closeBtn);
    btnRow->addWidget(m_saveBtn);
    m_vlay->addLayout(btnRow);
    m_vlay->addStretch(1);

    // ── signal connections ────────────────────────────────────────────
    connect(m_nameEdit,       &QLineEdit::textEdited, this, &ApplicationEditorWidget::onFieldEdited);
    connect(m_execEdit,       &QLineEdit::textEdited, this, &ApplicationEditorWidget::onFieldEdited);
    connect(m_commentEdit,    &QLineEdit::textEdited, this, &ApplicationEditorWidget::onFieldEdited);
    connect(m_categoriesEdit, &QLineEdit::textEdited, this, &ApplicationEditorWidget::onFieldEdited);
    connect(m_terminalChk,    &QCheckBox::toggled,    this, &ApplicationEditorWidget::onFieldEdited);
    connect(m_noDisplayChk,   &QCheckBox::toggled,    this, &ApplicationEditorWidget::onFieldEdited);
    connect(m_saveBtn,  &QPushButton::clicked, this, &ApplicationEditorWidget::onSave);
    connect(m_closeBtn, &QPushButton::clicked, this, &ApplicationEditorWidget::closeRequested);

    setWidget(m_content);
}

void ApplicationEditorWidget::loadEntry(const QString &filePath)
{
    DesktopEntry e = DesktopFileHandler::load(filePath);
    loadEntry(e);
}

void ApplicationEditorWidget::loadEntry(const DesktopEntry &entry)
{
    m_loading = true;
    m_entry = entry;


    // Icon
    QString iconName = entry.getString(QStringLiteral("Icon"));
    if (!iconName.isEmpty()) {
        QIcon icon;
        if (iconName.startsWith(QLatin1Char('/')))
            icon = QIcon(iconName);
        else
            icon = QIcon::fromTheme(iconName);
        m_iconBtn->setIcon(icon.isNull()
            ? QIcon::fromTheme(QStringLiteral("application-x-executable"))
            : icon);
    } else {
        m_iconBtn->setIcon(QIcon::fromTheme(QStringLiteral("application-x-executable")));
    }

    // Type label
    QString type = entry.getString(QStringLiteral("Type"));
    m_typeLbl->setText(type.isEmpty() ? tr("Entry") : type);

    // File label
    if (!entry.filePath.isEmpty()) {
        QFontMetrics fm(m_fileLbl->font());
        m_fileLbl->setText(fm.elidedText(entry.filePath, Qt::ElideMiddle, 400));
        m_fileLbl->setToolTip(entry.filePath);
    } else {
        m_fileLbl->setText(tr("(unsaved)"));
    }

    // Fields
    m_nameEdit->setText(entry.getString(QStringLiteral("Name")));
    m_execEdit->setText(entry.getString(QStringLiteral("Exec")));
    m_commentEdit->setText(entry.getString(QStringLiteral("Comment")));
    m_categoriesEdit->setText(entry.getString(QStringLiteral("Categories")));
    m_terminalChk->setChecked(entry.getBool(QStringLiteral("Terminal")));
    m_noDisplayChk->setChecked(entry.getBool(QStringLiteral("NoDisplay")));

    // Show/hide exec row depending on type
    bool isApp = (type == QStringLiteral("Application"));
    m_execEdit->setEnabled(isApp);
    m_terminalChk->setEnabled(isApp);
    m_categoriesEdit->setEnabled(isApp);

    // Advanced fields
    for (auto it = m_advancedEdits.begin(); it != m_advancedEdits.end(); ++it)
        it.value()->setText(entry.getString(it.key()));

    m_loading = false;
    // Nova entrada sem arquivo: já está "suja" (precisa ser salva)
    m_dirty = entry.filePath.isEmpty();
    m_saveBtn->setEnabled(m_dirty);
}

void ApplicationEditorWidget::focusNameField()
{
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

void ApplicationEditorWidget::onIconButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("Select Icon"),
        QStringLiteral("/usr/share/icons"),
        tr("Images (*.png *.svg *.xpm *.ico)")
    );
    if (path.isEmpty()) return;
    m_entry.setString(QStringLiteral("Icon"), path);
    m_iconBtn->setIcon(QIcon(path));
    if (!m_dirty) {
        m_dirty = true;
        m_saveBtn->setEnabled(true);
    }
}

void ApplicationEditorWidget::onFieldEdited()
{
    if (m_loading) return;
    if (!m_dirty) {
        m_dirty = true;
        m_saveBtn->setEnabled(true);
    }
}

void ApplicationEditorWidget::onSave()
{
    applyAndSave();
    m_dirty = false;
    m_saveBtn->setEnabled(false);
    emit saveRequested();
}

QString ApplicationEditorWidget::applyAndSave()
{
    // Pull values from form into entry
    m_entry.setString(QStringLiteral("Name"),       m_nameEdit->text());
    m_entry.setString(QStringLiteral("Exec"),       m_execEdit->text());
    m_entry.setString(QStringLiteral("Comment"),    m_commentEdit->text());
    m_entry.setString(QStringLiteral("Categories"), m_categoriesEdit->text());
    m_entry.setBool(QStringLiteral("Terminal"),     m_terminalChk->isChecked());
    m_entry.setBool(QStringLiteral("NoDisplay"),    m_noDisplayChk->isChecked());

    for (auto it = m_advancedEdits.begin(); it != m_advancedEdits.end(); ++it)
        m_entry.setString(it.key(), it.value()->text());

    // Determine save path
    QString type = m_entry.getString(QStringLiteral("Type"), QStringLiteral("Application"));
    QString userAppDir = QDir::homePath() + "/.local/share/applications/";
    bool isSystemFile = !m_entry.filePath.isEmpty() && !m_entry.filePath.startsWith(userAppDir);
    QString savePath;
    if (isSystemFile) {
        // Ask user for confirmation to overwrite system file
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("System File Overwrite"));
        msgBox.setText(tr("This launcher is located in a system directory. Do you want to overwrite the original file?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes) {
            savePath = m_entry.filePath;
        } else {
            savePath = XdgPaths::getSavePath(
                m_entry.getString(QStringLiteral("Name")),
                m_entry.filePath,
                type);
        }
    } else {
        savePath = XdgPaths::getSavePath(
            m_entry.getString(QStringLiteral("Name")),
            m_entry.filePath,
            type);
    }

    if (DesktopFileHandler::save(m_entry, savePath)) {
        m_entry.filePath = savePath;
        m_fileLbl->setText(savePath);
        emit entryChanged(savePath);
        return savePath;
    } else {
        QMessageBox::critical(this, tr("Save Failed"),
            tr("Could not save the launcher. Try running as administrator or check your permissions."));
        return {};
    }
}

