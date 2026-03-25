/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "ukccframe.h"
#include <QPainter>
#include <QApplication>

UkccFrame::UkccFrame(QWidget *parent, BorderRadiusStyle style, bool heightAdaptive)
    : QFrame(parent), mRadiusType(style), mHeightAdaptive(heightAdaptive), mIscontainer(false)
{
    this->setFrameShape(QFrame::Box);
    if (mHeightAdaptive) {
        this->setMinimumSize(550, PC_HEIGHT);
        this->setMaximumSize(16777215, PC_HEIGHT);
    }
}

UkccFrame::~UkccFrame() {}

void UkccFrame::setRadiusType(BorderRadiusStyle style)
{
    mRadiusType = style;
    update();
}

void UkccFrame::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (this->frameShape() != QFrame::NoFrame)
        painter.setBrush(qApp->palette().base());
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();

    int normalRadius = RADIUS_VALUE;

    if (mRadiusType == UkccFrame::None) {
        painter.drawRoundedRect(rect, 0, 0);
    } else {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, normalRadius, normalRadius);
        painterPath.setFillRule(Qt::WindingFill);
        if (mRadiusType == UkccFrame::Bottom) {
            painterPath.addRect(rect.width() - normalRadius, 0, normalRadius, normalRadius);
            painterPath.addRect(0, 0, normalRadius, normalRadius);
        } else if (mRadiusType == UkccFrame::Top) {
            painterPath.addRect(0, rect.height() - normalRadius, normalRadius, normalRadius);
            painterPath.addRect(rect.width() - normalRadius, rect.height() - normalRadius, normalRadius, normalRadius);
        }
        painter.drawPath(painterPath);
    }
}
