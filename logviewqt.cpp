#include "logviewqt.h"
#include <QDebug>
#include <QFileDialog>

LogViewQt::LogViewQt( const QString& fname, QWidget *parent)
    : QMainWindow(parent)
    , model(nullptr)
{
    ui.setupUi(this);
	QObject::connect(ui.actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect( ui.actionResize, SIGNAL( triggered( ) ), ui.centralWidget, SLOT( resizeRowsToContents( ) ) );
    QObject::connect( ui.actionOpen, SIGNAL( triggered( ) ), this, SLOT( selectOpenFileName( ) ) );

    createModel( fname );
}

void LogViewQt::createModel( const QString& fname )
{
    ui.statusBar->showMessage( fname );

    if ( !fname.isEmpty( ) )
    {
        ui.centralWidget->setModel( nullptr );
        delete model;

        model = new LogModel( fname, nullptr );
        QObject::connect( model, SIGNAL( changeModel( ) ), this, SLOT( ChangeTable( ) ) );
        ui.centralWidget->setModel( model );
        QObject::connect( ui.actionCRIT, SIGNAL( triggered( bool ) ), model, SLOT( switchCrit( bool ) ) );
        QObject::connect( ui.actionERROR, SIGNAL( triggered( bool ) ), model, SLOT( switchErr( bool ) ) );
        QObject::connect( ui.actionWARN, SIGNAL( triggered( bool ) ), model, SLOT( switchWarn( bool ) ) );
        QObject::connect( ui.actionINFO, SIGNAL( triggered( bool ) ), model, SLOT( switchInfo( bool ) ) );
        QObject::connect( ui.actionTEST, SIGNAL( triggered( bool ) ), model, SLOT( switchTest( bool ) ) );
        QObject::connect( ui.actionDEBUG, SIGNAL( triggered( bool ) ), model, SLOT( switchDbg( bool ) ) );
        QObject::connect( ui.actionMonitor_Log, SIGNAL( triggered( bool ) ), model, SLOT( switchMon( bool ) ) );
        QObject::connect( ui.actionRPC_log, SIGNAL( triggered( bool ) ), model, SLOT( switchRpc( bool ) ) );
        QObject::connect( ui.actionTM_log, SIGNAL( triggered( bool ) ), model, SLOT( switchTm( bool ) ) );

        ui.centralWidget->wordWrap( );
        ui.centralWidget->verticalHeader( )->sectionResizeMode( QHeaderView::ResizeToContents );
        ui.centralWidget->resizeColumnsToContents( );
        ui.centralWidget->resizeRowsToContents( );

        ui.actionCRIT->setChecked( true );
        ui.actionDEBUG->setChecked( true );
        ui.actionERROR->setChecked( true );
        ui.actionINFO->setChecked( true );
        ui.actionTEST->setChecked( true );
        ui.actionWARN->setChecked( true );
        ui.actionMonitor_Log->setChecked( true );
        ui.actionRPC_log->setChecked( true );
        ui.actionTM_log->setChecked( true );
    }
}

LogViewQt::~LogViewQt( )
{
	delete model;
}

void LogViewQt::ChangeTable()
{
	ui.centralWidget->setModel(nullptr);
	ui.centralWidget->setModel(model);
	ui.centralWidget->resizeColumnsToContents();
	ui.centralWidget->resizeRowsToContents();
}

void LogViewQt::selectOpenFileName( )
{
    QString fname = QFileDialog::getOpenFileName( nullptr, "Open File ...", "", "*.elog *.log" );
    QFile f( fname );
    if ( f.exists( ) )
    {
        createModel( fname );
    }
}
