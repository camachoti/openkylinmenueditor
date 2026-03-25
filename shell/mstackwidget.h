/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 OpenKylin Menu Editor Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef MSTACKWIDGET_H
#define MSTACKWIDGET_H

#include <QStackedWidget>

class MStackWidget : public QStackedWidget
{
    Q_OBJECT
public:
    MStackWidget(QWidget *parent = nullptr);
    ~MStackWidget();
    void paintEvent(QPaintEvent *event) override;

public:
    bool status = true; // true = home (base), false = module (window)
};

#endif // MSTACKWIDGET_H
