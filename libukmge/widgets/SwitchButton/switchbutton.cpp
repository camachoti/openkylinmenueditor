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
#include "switchbutton.h"

#include <QDebug>
#include <QApplication>

#define TIMER_INTERVAL  5
#define MOVING_STEPS   40

SwitchButton::SwitchButton(QWidget *parent)
    : QWidget(parent)
{
    this->setFixedSize(QSize(50, 24));

    checked   = false;
    hover     = false;
    disabled  = false;
    isMoving  = false;
    isAnimation = true;

    space      = 4;
    rectRadius = height() / 2;

    mStep   = width() / MOVING_STEPS;
    mStartX = 0;
    mEndX   = 0;

    mTimer = new QTimer(this);
    mTimer->setInterval(TIMER_INTERVAL);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updatevalue()));

    // Detect dark/light palette from the application
    bool isDark = (qApp->palette().color(QPalette::Window).lightness() < 128);
    changeColor(isDark);
    // Palette changes are now handled by an event() override in Qt6.
}

SwitchButton::~SwitchButton() {}

bool SwitchButton::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        bool isDark = (qApp->palette().color(QPalette::Window).lightness() < 128);
        changeColor(isDark);
        update();
        return true;
    }
    return QWidget::event(event);
}

void SwitchButton::changeColor(bool isDark)
{
    if (hover) return;

    if (isDark) {
        bgColorOff         = QColor(OFF_BG_DARK_COLOR);
        bgColorOn          = QColor(ON_BG_DARK_COLOR);
        rectColorEnabled   = QColor(ENABLE_RECT_DARK_COLOR);
        rectColorDisabled  = QColor(DISABLE_RECT_DARK_COLOR);
        sliderColorDisabled = QColor(DISABLE_RECT_DARK_COLOR);
        sliderColorEnabled = QColor(ENABLE_RECT_DARK_COLOR);
        bgHoverOnColor     = QColor(ON_HOVER_BG_DARK_COLOR);
        bgHoverOffColor    = QColor(OFF_HOVER_BG_DARK_COLOR);
        bgColorDisabled    = QColor(DISABLE_DARK_COLOR);
    } else {
        bgColorOff         = QColor(OFF_BG_LIGHT_COLOR);
        bgColorOn          = QColor(ON_BG_LIGHT_COLOR);
        rectColorEnabled   = QColor(ENABLE_RECT_LIGHT_COLOR);
        rectColorDisabled  = QColor(DISABLE_RECT_LIGHT_COLOR);
        sliderColorDisabled = QColor(DISABLE_RECT_LIGHT_COLOR);
        sliderColorEnabled = QColor(ENABLE_RECT_LIGHT_COLOR);
        bgHoverOnColor     = QColor(ON_HOVER_BG_LIGHT_COLOR);
        bgHoverOffColor    = QColor(OFF_HOVER_BG_LIGHT_COLOR);
        bgColorDisabled    = QColor(DISABLE_LIGHT_COLOR);
    }
}

void SwitchButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    drawBg(&painter);
    if (!isAnimation) isMoving = false;
    if (isMoving) animation(&painter);
    drawSlider(&painter);
    painter.end();
}

void SwitchButton::animation(QPainter *painter)
{
    painter->save();
    int h = height();
    int w = width();
    painter->setPen(Qt::NoPen);
    if (checked) {
        painter->setBrush(bgColorOn);
        rect.setRect(0, 0, h + mStartX, h);
    } else {
        painter->setBrush(bgColorOff);
        rect.setRect(mStartX, 0, w - mStartX, h);
    }
    painter->drawRoundedRect(rect, rectRadius, rectRadius);
    painter->restore();
}

void SwitchButton::drawBg(QPainter *painter)
{
    int w = width();
    int h = height();
    painter->save();
    painter->setPen(Qt::NoPen);
    if (disabled) {
        painter->setBrush(bgColorDisabled);
    } else {
        if (checked) {
            if (isMoving) {
                painter->setBrush(bgColorOff);
                rect.setRect(mStartX, 0, w - mStartX, h);
            } else {
                painter->setBrush(bgColorOn);
                rect.setRect(0, 0, w, h);
            }
        } else {
            if (isMoving) {
                painter->setBrush(bgColorOn);
                rect.setRect(0, 0, mStartX + h, h);
            } else {
                painter->setBrush(bgColorOff);
                rect.setRect(0, 0, w, h);
            }
        }
    }
    painter->drawRoundedRect(rect, rectRadius, rectRadius);
    painter->restore();
}

void SwitchButton::drawSlider(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    if (!disabled)
        painter->setBrush(sliderColorEnabled);
    else
        painter->setBrush(sliderColorDisabled);

    if (disabled) {
        if (!checked) {
            QRect smallRect(8, height() / 2 - 2, 10, 4);
            painter->drawRoundedRect(smallRect, 3, 3);
        } else {
            QRect smallRect(width() - 16, height() / 2 - 2, 10, 4);
            painter->drawRoundedRect(smallRect, 3, 3);
        }
    }

    QRect r(0, 0, width(), height());
    int sliderWidth = r.height() - space * 2;
    QRect sliderRect(mStartX + space, space, sliderWidth, sliderWidth);
    painter->drawEllipse(sliderRect);
    painter->restore();
}

void SwitchButton::mousePressEvent(QMouseEvent *)
{
    if (isMoving) return;
    if (disabled) {
        mEndX = 0;
        Q_EMIT disabledClick();
        return;
    }
    checked = !checked;
    Q_EMIT checkedChanged(checked);
    mStep = width() / MOVING_STEPS;
    mEndX = checked ? (width() - height()) : 0;
    mTimer->start();
    isMoving = true;
}

void SwitchButton::resizeEvent(QResizeEvent *)
{
    mStep     = width() / MOVING_STEPS;
    mStartX   = checked ? (width() - height()) : 0;
    rectRadius = height() / 2;
    update();
}

void SwitchButton::enterEvent(QEvent *event)
{
    bgColorOn  = bgHoverOnColor;
    bgColorOff = bgHoverOffColor;
    hover = true;
    update();
    QWidget::enterEvent(event);
}

void SwitchButton::leaveEvent(QEvent *event)
{
    hover = false;
    bool isDark = (qApp->palette().color(QPalette::Window).lightness() < 128);
    changeColor(isDark);
    update();
    QWidget::leaveEvent(event);
}

void SwitchButton::updatevalue()
{
    if (checked) {
        if (mStartX < mEndX - mStep)
            mStartX = mStartX + mStep;
        else {
            mStartX = mEndX;
            mTimer->stop();
            isMoving = false;
        }
    } else {
        if (mStartX > mEndX + mStep)
            mStartX = mStartX - mStep;
        else {
            mStartX = mEndX;
            mTimer->stop();
            isMoving = false;
        }
    }
    update();
}

void SwitchButton::setChecked(bool c)
{
    if (this->checked != c) {
        this->checked = c;
        Q_EMIT checkedChanged(c);
        update();
    }
    mStep = width() / MOVING_STEPS;
    mEndX = c ? (width() - height()) : 0;
    mTimer->start();
    isMoving = true;
}

bool SwitchButton::isChecked() { return checked; }

void SwitchButton::setDisabledFlag(bool value) { disabled = value; update(); }
bool SwitchButton::getDisabledFlag() { return disabled; }
void SwitchButton::setAnimation(bool on) { isAnimation = on; }
