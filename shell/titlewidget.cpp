/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "titlewidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

TitleWidget::TitleWidget(QWidget *parent)
    : QWidget(parent)
{
    // Evita fundo opaco, deixa só o paintEvent pintar o visual desejado
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setAutoFillBackground(false);
}

TitleWidget::~TitleWidget()
{
}

void TitleWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    QPainterPath path;
    path.addRoundedRect(this->rect(), 6, 6);
    path.setFillRule(Qt::WindingFill);
    // Fill the bottom-right corner square to make only top corners rounded
    path.addRect(width() - 6, height() - 6, 6, 6);

    // Siga sempre o QPalette::Window para se alinhar ao Peony/UKUI
    QColor bg = qApp->palette().color(QPalette::Window);
    // Aplicar transparência padrão Peony (~82%)
    bg.setAlpha(210); // ajuste conforme desejar (210 = 82% opaco)
    painter.setBrush(bg);
    painter.drawPath(path);

    QWidget::paintEvent(event);
}
