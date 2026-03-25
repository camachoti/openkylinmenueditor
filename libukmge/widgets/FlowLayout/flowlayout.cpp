/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "flowlayout.h"

#include <QWidget>
#include <QDebug>
#include <QtMath>

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(QWidget *parent, bool home, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing), m_home(home)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0))) {}
}

void FlowLayout::addItem(QLayoutItem *item) { itemList.append(item); }

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0 || m_hSpace == -1)
        return m_hSpace;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0 || m_vSpace == -1)
        return m_vSpace;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const { return itemList.size(); }

QLayoutItem *FlowLayout::itemAt(int index) const { return itemList.value(index); }

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const { return {}; }
bool FlowLayout::hasHeightForWidth() const { return true; }
int FlowLayout::heightForWidth(int width) const { return doLayout(QRect(0, 0, width, 0), true); }
void FlowLayout::setGeometry(const QRect &rect) { QLayout::setGeometry(rect); doLayout(rect, false); }
QSize FlowLayout::sizeHint() const { return minimumSize(); }

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : itemList)
        size = size.expandedTo(item->minimumSize());
    int m = contentsMargins().left();
    size += QSize(2 * m, 2 * m);
    return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;
    int fillX = 0;
    bool bFillX = false;

    for (QLayoutItem *item : itemList) {
        QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1) {
            if (!bFillX) { bFillX = true; fillX = fillSpaceX(wid); }
            spaceX = fillX;
        }
        int spaceY = verticalSpacing();
        if (spaceY == -1 && fillX >= 0) spaceY = fillX;
        if (m_home) spaceY = 32;

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }
        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) return -1;
    if (parent->isWidgetType())
        return static_cast<QWidget *>(parent)->style()->pixelMetric(pm, nullptr, static_cast<QWidget *>(parent));
    return static_cast<QLayout *>(parent)->spacing();
}

int FlowLayout::fillSpaceX(QWidget *wid) const
{
    int num = 0;
    int space = m_home ? 24 : 4;
    int len = parentWidget()->width() - contentsMargins().left() - contentsMargins().right();
    while (true) {
        num++;
        if (num * (wid->width() + space) - space >= len) break;
    }
    num = num - 1;
    if (num <= 1) return 32;

    int numH = (int)ceil((double)itemList.size() / num);
    int x = len + space - num * (wid->width() + space);
    x = (int)ceil((double)x / (num - 1)) + space - 1;

    int maxY = m_home ? numH * (wid->height() + 32)
                      : numH * (wid->height() + x) + 32 - x;
    if (m_home && parentWidget() && parentWidget()->parentWidget())
        parentWidget()->parentWidget()->setFixedHeight(maxY);
    parentWidget()->setFixedHeight(maxY);
    return x;
}
