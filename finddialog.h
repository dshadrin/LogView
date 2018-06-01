/*
* Copyright (C) 2017 Rhonda Software.
* All rights reserved.
*/
#pragma once

//////////////////////////////////////////////////////////////////////////
#include <QDialog>
#include "ui_finddialog.h"
//////////////////////////////////////////////////////////////////////////

class FDialog : public QDialog
{
    Q_OBJECT

public:
    FDialog(QWidget *parent = Q_NULLPTR);
    ~FDialog();

    QString getText() const;

protected slots:
    void accept() override;

private:
    Ui::findDialog ui;
};

//////////////////////////////////////////////////////////////////////////
