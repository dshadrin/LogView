/*
* Copyright (C) 2017 Rhonda Software.
* All rights reserved.
*/

//////////////////////////////////////////////////////////////////////////
#include "finddialog.h"
//////////////////////////////////////////////////////////////////////////

FDialog::FDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_CustomWhatsThis);
}

FDialog::~FDialog()
{
}

QString FDialog::getText() const
{
    return ui.lineEdit->displayText();
}

//////////////////////////////////////////////////////////////////////////
void FDialog::accept()
{
    QDialog::accept();
}
