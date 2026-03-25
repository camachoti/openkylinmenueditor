/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../libukmge/widgets/CloseButton/closebutton.h"
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTimer>

#include "editorwidget.h"
#include "searchbar.h"

#include "leftsidebarwidget.h"
#include "titlewidget.h"
#include "mstackwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    enum ResizeRegion {
        NoResize,
        ResizeTop,
        ResizeBottom,
        ResizeLeft,
        ResizeRight,
        ResizeTopLeft,
        ResizeTopRight,
        ResizeBottomLeft,
        ResizeBottomRight
    };
    ResizeRegion m_resizeRegion = NoResize;
    bool m_resizing = false;
    QPoint m_dragStartPos;
    QRect m_dragStartGeometry;

    ResizeRegion hitTestResizeRegion(const QPoint &pos) const;


protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void applyRoundedMask(int radius = 16);

private:
    void initUI();
    void initTitleBar();
    void initStyleSheet();
    void initLeftSideBar();
    bool dblOnEdge(QMouseEvent *event);

    // Widgets principais (substituem o ui->XXX)
    QWidget *centralWidget = nullptr;
    LeftsidebarWidget *leftsidebarWidget = nullptr;
    QWidget *leftTopWidget = nullptr;
    QWidget *leftBotWidget = nullptr;
    TitleWidget *titleWidget = nullptr;
    MStackWidget *stackedWidget = nullptr;
    QVBoxLayout *leftsidebarVerLayout = nullptr;
    QVBoxLayout *verticalLayout = nullptr;
    QVBoxLayout *verticalLayout2 = nullptr;
    QVBoxLayout *leftBotLayout = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QHBoxLayout *horizontalLayout2 = nullptr;

    // Utilizados no código antigo
    EditorWidget  *m_editorWidget = nullptr;
    QPushButton *m_backBtn   = nullptr;
    QPushButton *m_optionBtn = nullptr;
    QPushButton *m_minBtn    = nullptr;
    QPushButton *m_maxBtn    = nullptr;
    CloseButton *m_closeBtn  = nullptr;
    QLabel      *m_titleIcon = nullptr;
    QLabel      *m_titleLabel = nullptr;
    QLabel      *m_logoLabel  = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;

    // New: Search bar

private slots:
    void showOptionMenu();

public slots:
    void switchToEditor();

Q_SIGNALS:
    void posChanged();
};

#endif // MAINWINDOW_H
