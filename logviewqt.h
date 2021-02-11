#ifndef LOGVIEWQT_H
#define LOGVIEWQT_H

#include <QtWidgets/QMainWindow>
#include "ui_logviewqt.h"
#include "logmodel.h"
#include <boost/filesystem.hpp>
#include <QComboBox>

extern QString confName;

class QTimer;

class LogViewQt : public QMainWindow
{
    Q_OBJECT

public:
    LogViewQt( const QString& file_name, QWidget *parent = 0 );
    ~LogViewQt( );

private:
    void createModel( const QString& fname );
    void readSettings();
    void writeSettings();
    void closeEvent(QCloseEvent *event) override;
    void stModelFlags() const;
    void findInTable(int fromRow);

private slots:
    void changeTable();
    void resetRow(int row);
    void selectOpenFileName( );
    void findText();
    void findTextCurent();
    void findTextNext();
    void findTextPrev();
    void actionEnable(quint32 mask);
    void selectText();
    void customContextMenuRequested(const QPoint &pos);
    void setColumnEnable( quint32 mask );
    void selectTickTimer();
    void setComboModel( QAbstractItemModel* mdl );

signals:
    int findRow(const QString& txt, int indBegin);
    int findRowPrev(const QString& txt, int indBegin);

private:
    Ui::LogViewQtClass ui;
    LogModel * model;

    QMenu* selectMnu;
    QAction* newSelectAct;
    QComboBox* modules;

    // stored find data
    QTimer* checkSelectTimer;
    QString foundStr;
    int foundRow;
};

#endif // LOGVIEWQT_H
