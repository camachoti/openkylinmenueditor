/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "leftsidebarwidget.h"

#include <QPainter>
#include <QStyleOption>

LeftsidebarWidget::LeftsidebarWidget(QWidget *parent)
    : QWidget(parent)
    , m_transparency(255)
{
}

LeftsidebarWidget::~LeftsidebarWidget()
= default;

void LeftsidebarWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);

    QColor color = palette().color(QPalette::Base);
    color.setAlpha(m_transparency);

    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, color);
    this->setPalette(pal);

    p.setBrush(QBrush(color));
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    Q_UNUSED(event)
}
