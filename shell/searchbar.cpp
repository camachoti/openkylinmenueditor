#include "searchbar.h"
#include <QIcon>
#include <QHBoxLayout>

SearchBar::SearchBar(QWidget *parent) : QWidget(parent)
{
    m_lineEdit = new QLineEdit(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto searchIcon = QIcon::fromTheme("edit-find-symbolic");
    if (!searchIcon.isNull()) {
        m_lineEdit->addAction(searchIcon, QLineEdit::TrailingPosition);
    }
    m_lineEdit->setPlaceholderText(tr("Input search key..."));
    m_lineEdit->setToolTip(tr("Input the search key of files you would like to find."));
    m_lineEdit->setTextMargins(5, 0, 0, 0);

    connect(m_lineEdit, &QLineEdit::returnPressed, this, &SearchBar::returnPressed);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &SearchBar::textChanged);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_lineEdit);
    setLayout(layout);
}

void SearchBar::setPlaceholderText(const QString &text)
{
    m_lineEdit->setPlaceholderText(text);
}

void SearchBar::setToolTip(const QString &text)
{
    m_lineEdit->setToolTip(text);
}

QString SearchBar::text() const
{
    return m_lineEdit->text();
}

void SearchBar::clear()
{
    m_lineEdit->clear();
}

void SearchBar::setFixedWidth(int w)
{
    m_lineEdit->setFixedWidth(w);
}

void SearchBar::setTextMargins(int left, int top, int right, int bottom)
{
    m_lineEdit->setTextMargins(left, top, right, bottom);
}
