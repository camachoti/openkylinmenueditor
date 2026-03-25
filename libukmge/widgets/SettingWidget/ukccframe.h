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
#ifndef UKCCFRAME_H
#define UKCCFRAME_H

#include <QFrame>
#include <QPainterPath>

#include "libukmge_global.h"

#define RADIUS_VALUE   6
#define PC_HEIGHT      60
#define TABLET_HEIGHT  64

class LIBUKMGE_EXPORT UkccFrame : public QFrame {
    Q_OBJECT
public:
    enum BorderRadiusStyle {
        Top,
        None,
        Bottom,
        Around
    };

    explicit UkccFrame(QWidget *parent = nullptr,
                       BorderRadiusStyle style = Around,
                       bool heightAdaptive = false);
    ~UkccFrame();

    void setRadiusType(BorderRadiusStyle style);

    inline bool iscontainer() { return mIscontainer; }
    inline void setContainer(bool b = false) {
        mIscontainer = b;
        if (b) setFrameShape(QFrame::NoFrame);
    }
    inline void setHeightAdaptive(bool isAdaptive) { mHeightAdaptive = isAdaptive; }

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    BorderRadiusStyle mRadiusType;
    bool mHeightAdaptive = false;
    bool mIscontainer = false;
};

#endif // UKCCFRAME_H
