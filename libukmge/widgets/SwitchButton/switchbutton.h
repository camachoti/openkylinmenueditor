/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>

#include "libukmge_global.h"

#define OFF_BG_DARK_COLOR        "#404040"
#define OFF_HOVER_BG_DARK_COLOR  "#666666"
#define ON_BG_DARK_COLOR         "#3790FA"
#define ON_HOVER_BG_DARK_COLOR   "#40A9FB"
#define DISABLE_DARK_COLOR       "#474747"
#define DISABLE_RECT_DARK_COLOR  "#6E6E6E"
#define ENABLE_RECT_DARK_COLOR   "#FFFFFF"

#define OFF_BG_LIGHT_COLOR        "#E0E0E0"
#define OFF_HOVER_BG_LIGHT_COLOR  "#B3B3B3"
#define ON_BG_LIGHT_COLOR         "#3790FA"
#define ON_HOVER_BG_LIGHT_COLOR   "#40A9FB"
#define DISABLE_LIGHT_COLOR       "#E9E9E9"
#define DISABLE_RECT_LIGHT_COLOR  "#B3B3B3"
#define ENABLE_RECT_LIGHT_COLOR   "#FFFFFF"

class LIBUKMGE_EXPORT SwitchButton : public QWidget
{
    Q_OBJECT

public:
    SwitchButton(QWidget *parent = nullptr);
    ~SwitchButton();

    void setChecked(bool checked);
    void setAnimation(bool on);
    bool isChecked();
    void setDisabledFlag(bool);
    bool getDisabledFlag();

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void drawBg(QPainter *painter);
    void drawSlider(QPainter *painter);
    void changeColor(bool isDark);

private:
    bool checked;
    bool disabled;
    bool hover;
    bool isMoving;
    bool isAnimation;

    void animation(QPainter *painter);
    QRect rect;

    QColor bgColorOff;
    QColor bgColorOn;
    QColor bgHoverOnColor;
    QColor bgHoverOffColor;
    QColor bgColorDisabled;
    QColor sliderColorEnabled;
    QColor sliderColorDisabled;
    QColor rectColorEnabled;
    QColor rectColorDisabled;
    QColor sliderColorOff;
    QColor sliderColorOn;

    int space;
    int rectRadius;
    int mStep;
    int mStartX;
    int mEndX;

    QTimer *mTimer;

private Q_SLOTS:
    void updatevalue();

Q_SIGNALS:
    void checkedChanged(bool checked);
    void disabledClick();
};

#endif // SWITCHBUTTON_H
