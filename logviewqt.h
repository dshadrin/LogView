#ifndef LOGVIEWQT_H
#define LOGVIEWQT_H

#include <QtWidgets/QMainWindow>
#include "ui_logviewqt.h"
#include "logtablemodel.h"

class LogViewQt : public QMainWindow
{
    Q_OBJECT

public:
    LogViewQt( const QString& file_name, QWidget *parent = 0 );
    ~LogViewQt( );

private:
    void createModel( const QString& fname );

public slots:
	void ChangeTable();
    void selectOpenFileName( );

private:
    Ui::LogViewQtClass ui;
    LogTableModel * model;
};

#endif // LOGVIEWQT_H
