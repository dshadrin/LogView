#ifndef LOGVIEWQT_H
#define LOGVIEWQT_H

#include <QtWidgets/QMainWindow>
#include "ui_logviewqt.h"
#include "logmodel.h"
#include <boost/filesystem.hpp>

extern QString confName;

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

private slots:
    void changeTable();
    void resetRow(int row);
    void selectOpenFileName( );
    void findText();
    void findTextNext();
    void findTextPrev();

    void selectText();
    void customContextMenuRequested(const QPoint &pos);


signals:
    int findRow(const QString& txt, int indBegin);
    int findRowPrev(const QString& txt, int indBegin);

private:
    Ui::LogViewQtClass ui;
    LogModel * model;

    QMenu* selectMnu;
    QAction* newSelectAct;

    // stored find data
    QString foundStr;
    int foundRow;
};

#endif // LOGVIEWQT_H
