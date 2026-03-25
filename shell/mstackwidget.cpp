/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "mstackwidget.h"

#include <QPainter>
#include <QPainterPath>

MStackWidget::MStackWidget(QWidget *parent)
    : QStackedWidget(parent)
{
}

MStackWidget::~MStackWidget()
{
}

void MStackWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    QPainterPath path;
    path.addRoundedRect(this->rect(), 6, 6);
    path.setFillRule(Qt::WindingFill);
    path.addRect(width() - 6, height() - 6, 6, 6);

    // status == true  → home page  → use base colour
    // status == false → module page → use window colour
    painter.setBrush(status ? this->palette().base()
                            : this->palette().window());
    painter.drawPath(path);

    QWidget::paintEvent(event);
}
