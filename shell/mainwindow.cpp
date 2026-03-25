/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "mainwindow.h"
#include "searchbar.h"

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window); // Remove decoração padrão, usando barra customizada
    setAttribute(Qt::WA_TranslucentBackground);
    initUI();
    applyRoundedMask(16); // Inicializa máscara arredondada
}

MainWindow::~MainWindow()
{
    // widgets filhos são destruídos automaticamente pelo parent (Qt parent-child ownership)
}

// Mascara arredondada real (clip físico da janela; aplica só ao topo!).
// Não confundir com o paintEvent, que é apenas visual.
// OBS: Em alguns ambientes Linux (Wayland/X11 sem compositor), pode não ser suportado ou ignorado.
void MainWindow::applyRoundedMask(int radius) {
    // Usar apenas rounded rect e sem fallback ellipse para fidelidade Peony
    QPainterPath path;
    path.addRoundedRect(this->rect(), radius, radius);
    QRegion mask = QRegion(path.toFillPolygon().toPolygon());
    setMask(mask);
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor bg = palette().color(QPalette::Window);
    if (bg.alpha() < 255) bg = QColor(240,240,240,255);
    QRect r = this->rect();
    int radius = 12; // Mais fiel ao Peony/UKUI
    QPainterPath path;
    path.addRoundedRect(r, radius, radius);
    painter.fillPath(path, bg);
    painter.setPen(Qt::NoPen);
}

// Interface agora gerada totalmente em C++. Nenhuma dependência de .ui.
void MainWindow::initUI()
{
    this->setMinimumSize(978, 630);

    // Center on primary screen
    QRect screenRect = QApplication::primaryScreen()->geometry();
    this->move(screenRect.center() - this->rect().center());

    // HIERARQUIA VISUAL CONFORME mainwindow.ui:
    // 1. centralWidget (set como centralWidget da QMainWindow)
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 2. Layout principal (QHBoxLayout)
    horizontalLayout2 = new QHBoxLayout(centralWidget);
    horizontalLayout2->setSpacing(0);
    horizontalLayout2->setContentsMargins(0, 0, 0, 0);

    // 3. Lado esquerdo: Widget de barra lateral (LeftsidebarWidget)
    leftsidebarWidget = new LeftsidebarWidget(centralWidget);
    leftsidebarWidget->setMinimumSize(QSize(140, 0));
    leftsidebarWidget->setMaximumSize(QSize(140, 16777215));
    leftsidebarWidget->setFixedWidth(140);
    QVBoxLayout *sidebarVBoxLayout = new QVBoxLayout(leftsidebarWidget);
    sidebarVBoxLayout->setSpacing(0);
    sidebarVBoxLayout->setContentsMargins(0, 0, 0, 0);

    // 3.1 leftsidebarVerLayout
    leftsidebarVerLayout = new QVBoxLayout();
    leftsidebarVerLayout->setSpacing(0);
    leftsidebarVerLayout->setContentsMargins(0, 0, 0, 0);
    sidebarVBoxLayout->addLayout(leftsidebarVerLayout);

    // 3.1.1 TopBar (leftTopWidget)
    leftTopWidget = new QWidget(leftsidebarWidget);
    leftTopWidget->setMinimumSize(QSize(0, 40));
    leftTopWidget->setMaximumSize(QSize(16777215, 40));
    horizontalLayout = new QHBoxLayout(leftTopWidget);
    horizontalLayout->setSpacing(8);
    horizontalLayout->setContentsMargins(8,0,8,0);
    leftsidebarVerLayout->addWidget(leftTopWidget);

    // 3.1.2 Spacer/dummy
    leftsidebarVerLayout->addSpacing(4);

    // 3.1.3 BotBar (leftBotWidget)
    leftBotWidget = new QWidget(leftsidebarWidget);
    leftBotLayout = new QVBoxLayout(leftBotWidget);
    leftBotLayout->setContentsMargins(8,0,8,0);
    leftsidebarVerLayout->addWidget(leftBotWidget);

    // Adiciona barra lateral ao layout principal
    horizontalLayout2->addWidget(leftsidebarWidget);

    // 4. Main vertical layout central
    QWidget *centralRightWidget = new QWidget(centralWidget);
    verticalLayout = new QVBoxLayout(centralRightWidget);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0,0,0,0);
    horizontalLayout2->addWidget(centralRightWidget);

    // 4.1 TitleWidget (acima)
    titleWidget = new TitleWidget(centralRightWidget);
    titleWidget->setMinimumSize(QSize(0, 40));
    titleWidget->setMaximumSize(QSize(16777215, 40));
    verticalLayout->addWidget(titleWidget);

    // 4.2 Conteúdo principal (stacked)
    verticalLayout2 = new QVBoxLayout();
    verticalLayout2->setSpacing(0);
    verticalLayout2->setContentsMargins(0,0,0,0);
    stackedWidget = new MStackWidget(centralRightWidget);
    verticalLayout2->addWidget(stackedWidget);
    verticalLayout->addLayout(verticalLayout2);

    // Inicializar custom widgets/título/botões/layouts
    initTitleBar();
    initStyleSheet();
    initLeftSideBar();

    // Build page widgets
    m_editorWidget = new EditorWidget(this);
    stackedWidget->addWidget(m_editorWidget);
    stackedWidget->setCurrentIndex(0); // Abrir direto na tela do editor

    // Wire up window control buttons
    connect(m_minBtn,   &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(m_maxBtn,   &QPushButton::clicked, this, [=]() {
        if (isMaximized()) {
            showNormal();
            m_maxBtn->setIcon(QIcon(":/icons/window-maximize-symbolic.svg"));
        } else {
            showMaximized();
            m_maxBtn->setIcon(QIcon(":/icons/window-restore-symbolic.svg"));
        }
     });
    connect(m_closeBtn, &CloseButton::clicked, qApp, &QApplication::quit);

    // Option (about) menu
    connect(m_optionBtn, &QPushButton::clicked, this, &MainWindow::showOptionMenu);
    // Install event filter for double-click on title to maximize
    this->installEventFilter(this);
}

void MainWindow::initTitleBar()
{
    m_titleLayout = new QHBoxLayout(titleWidget);
    m_titleLayout->setContentsMargins(0, 0, 0, 0);
    m_titleLayout->setSpacing(0);

    m_backBtn    = new QPushButton(titleWidget);
    m_optionBtn  = new QPushButton(titleWidget);
    m_minBtn     = new QPushButton(titleWidget);
    m_minBtn->setObjectName("window_min_btn");
    m_maxBtn     = new QPushButton(titleWidget);
    m_maxBtn->setObjectName("window_max_btn");
    m_closeBtn   = new CloseButton(titleWidget, ":/icons/window-close-symbolic.svg");
    m_closeBtn->setObjectName("window_close_btn");
    m_titleIcon  = new QLabel(titleWidget);
    m_titleLabel = new QLabel(tr("Menu Editor"), titleWidget);
    m_logoLabel  = new QLabel(titleWidget);

    m_backBtn->setObjectName("go-home");
    m_backBtn->setToolTip(tr("Back"));
    m_backBtn->setFixedSize(36, 36);
    m_backBtn->setHidden(true); // hidden on home page initially

    m_optionBtn->setToolTip(tr("Option"));
    m_optionBtn->setFixedSize(30, 30);

    m_minBtn->setToolTip(tr("Minimize"));
    m_minBtn->setFixedSize(30, 30);

    m_maxBtn->setToolTip(tr("Maximize"));
    m_maxBtn->setFixedSize(30, 30);

    m_closeBtn->setToolTip(tr("Close"));
    m_closeBtn->setFixedSize(30, 30);
    m_closeBtn->setIconSize(16);

    m_titleIcon->setFixedSize(24, 24);
    m_titleIcon->setPixmap(QIcon::fromTheme("accessories-character-map",
                           QIcon::fromTheme("applications-other")).pixmap(24, 24));

    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_logoLabel->setFixedSize(24, 24);
    m_logoLabel->setPixmap(QIcon::fromTheme("accessories-character-map",
                           QIcon::fromTheme("applications-other")).pixmap(24, 24));

    // Left part of sidebar top (logo + app name)
    horizontalLayout->addWidget(m_logoLabel);
    QLabel *textLabel = new QLabel(tr("Menu Editor"), leftTopWidget);
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalLayout->addWidget(textLabel);

    // Title bar centre + right
    m_titleLayout->addWidget(m_titleIcon);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addWidget(m_backBtn);
    
    // Add search bar (Peony-fidelity)
    
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_optionBtn);
    m_titleLayout->addWidget(m_minBtn);
    m_titleLayout->addWidget(m_maxBtn);
    m_titleLayout->addWidget(m_closeBtn);
}

#include <QImage>



void MainWindow::initStyleSheet()
{
    qDebug() << "QPalette::Window color:" << qApp->palette().color(QPalette::Window).name();

    // Window icon & title
    QIcon appIcon;
    if (QIcon::hasThemeIcon("accessories-character-map"))
        appIcon = QIcon::fromTheme("accessories-character-map");
    else
        appIcon = QIcon::fromTheme("applications-other");
    qApp->setWindowIcon(appIcon);
    this->setWindowTitle(tr("Menu Editor"));

    stackedWidget->setAutoFillBackground(true);
    centralWidget->setAttribute(Qt::WA_TranslucentBackground);
    leftsidebarWidget->setFixedWidth(140);

    // Back button
    m_backBtn->setProperty("useIconHighlightEffect", true);
    m_backBtn->setProperty("iconHighlightEffectMode", 1);
    m_backBtn->setFlat(true);
    m_backBtn->setIcon(QIcon::fromTheme("go-home-symbolic",
                       QIcon::fromTheme("go-previous")));
    m_backBtn->setIconSize(QSize(16, 16));

    // Window buttons
    m_optionBtn->setFlat(true);
    m_optionBtn->setIcon(QIcon(":/icons/open-menu-symbolic.svg"));

    m_minBtn->setFlat(true);
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setProperty("isWindowButton", 0x01);
    m_maxBtn->setFlat(true);
    m_maxBtn->setProperty("useIconHighlightEffect", 0x2);
    m_maxBtn->setProperty("isWindowButton", 0x1);
    /* Flat not needed as CloseButton renders its own */

    // Assegura cor/fundo e ícone igual ao botão de opções
    // Apenas flat, sem stylesheet: deixa o tema do SO desenhar tudo
    m_minBtn->setFlat(true);
    m_maxBtn->setFlat(true);
    /* Flat not needed as CloseButton renders its own */

    // Ícones symbolic do tema (herdam cor e hover do sistema)
    m_minBtn->setIcon(QIcon(":/icons/window-minimize-symbolic.svg"));
    m_maxBtn->setIcon(QIcon(":/icons/window-maximize-symbolic.svg"));
    m_closeBtn->setIcon(QIcon(":/icons/window-close-symbolic.svg"));
    m_minBtn->setText("");
    m_maxBtn->setText("");
    m_closeBtn->setText("");
}



void MainWindow::initLeftSideBar()
{
    // A barra lateral é opcional/visível conforme o conteúdo
    leftsidebarWidget->setHidden(true);
}

bool MainWindow::dblOnEdge(QMouseEvent *event)
{
    int globalMouseY = static_cast<int>(event->globalPos().y());
    int frameY       = this->y();
    return (globalMouseY >= frameY && globalMouseY <= frameY + 50);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        if (event->type() == QEvent::WindowStateChange) {
        if (this->windowState() == Qt::WindowMaximized)
            m_maxBtn->setIcon(QIcon(":/icons/window-restore-symbolic.svg"));
        else
            m_maxBtn->setIcon(QIcon(":/icons/window-maximize-symbolic.svg"));
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton && dblOnEdge(me)) {
                isMaximized() ? showNormal() : showMaximized();
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QMainWindow::keyPressEvent(event);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT posChanged();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // Em modo maximizado, remover arredondamento (cantos retos)
    // Fallback: se ambiente não suporta compositor ou mascara (X11 antigo/Wayland), os cantos podem não funcionar
    // Nesses casos, ao menos o visual estará correto pelo paintEvent, mas a interação física será quadrada.
    if (isMaximized()) {
        clearMask();
    } else {
        applyRoundedMask(16);
    }
}


void MainWindow::switchToEditor()
{
    stackedWidget->setCurrentIndex(1);
}

#include <QMenu>
#include <QAction>
#include <QMessageBox>

void MainWindow::showOptionMenu()
{
    QMenu aboutMenu;
    QAction *aboutAct = aboutMenu.addAction(tr("About"));
    connect(aboutAct, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About OpenKylin Menu Editor"),
            tr("OpenKylin Menu Editor\nModerno editor de menus XDG em Qt5/C++.\n\nCopyright (C) 2024 OpenKylin Menu Editor Contributors"));
    });
    aboutMenu.exec(m_optionBtn->mapToGlobal(QPoint(0, m_optionBtn->height())));
}

