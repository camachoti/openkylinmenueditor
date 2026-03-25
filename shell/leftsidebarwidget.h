/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef LEFTSIDEBARWIDGET_H
#define LEFTSIDEBARWIDGET_H

#include <QWidget>

class LeftsidebarWidget : public QWidget
{
    Q_OBJECT
public:
    LeftsidebarWidget(QWidget *parent = nullptr);
    ~LeftsidebarWidget() override;
    void paintEvent(QPaintEvent *event) override;

private:
    int m_transparency = 255;
};

#endif // LEFTSIDEBARWIDGET_H
