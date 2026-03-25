#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QAction>
#include <QIcon>
#include <QHBoxLayout>

class SearchBar : public QWidget
{
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = nullptr);
    void setPlaceholderText(const QString &text);
    void setToolTip(const QString &text);
    QString text() const;
    void clear();
    void setFixedWidth(int w);
    void setTextMargins(int left, int top, int right, int bottom);

signals:
    void returnPressed();
    void textChanged(const QString &text);

private:
    QLineEdit *m_lineEdit;
};

#endif // SEARCHBAR_H
