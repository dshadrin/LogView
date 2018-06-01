#include "logviewqt.h"
#include "finddialog.h"
#include <QFileDialog>
#include <QSettings>
#include <QtWidgets>

QString confName;

LogViewQt::LogViewQt( const QString& fname, QWidget *parent)
    : QMainWindow(parent)
    , model(nullptr)
    , foundRow(0)
{
    ui.setupUi(this);
	QObject::connect(ui.actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(selectOpenFileName()));
    QObject::connect(ui.actionFind, SIGNAL(triggered()), this, SLOT(findText()));
    QObject::connect(ui.actionNextFind, SIGNAL(triggered()), this, SLOT(findTextNext()));
    QObject::connect(ui.actionPrevFind, SIGNAL(triggered()), this, SLOT(findTextPrev()));
    readSettings();

    const QIcon newIcon(":/LogViewQt/Resources/select.png");
    selectMnu = new QMenu(this);
    newSelectAct = new QAction(newIcon, tr("Copy text to clipboard"), this);
    newSelectAct->setStatusTip(tr("Copy text to clipboard"));
    connect(newSelectAct, SIGNAL(triggered()), this, SLOT(selectText()));
    selectMnu->addAction(newSelectAct);
    connect(ui.centralWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(customContextMenuRequested(const QPoint&)));

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
        QObject::connect(model, SIGNAL(changeModel()), this, SLOT(changeTable()));
        QObject::connect(model, SIGNAL(resetRow(int)), this, SLOT(resetRow(int)));
        QObject::connect(this, SIGNAL(findRow(const QString&, int)), model, SLOT(findRow(const QString&, int)));
        QObject::connect(this, SIGNAL(findRowPrev(const QString&, int)), model, SLOT(findRowPrev(const QString&, int)));
        QObject::connect( ui.actionCRIT, SIGNAL( triggered( bool ) ), model, SLOT( switchCrit( bool ) ) );
        QObject::connect( ui.actionERROR, SIGNAL( triggered( bool ) ), model, SLOT( switchErr( bool ) ) );
        QObject::connect( ui.actionWARN, SIGNAL( triggered( bool ) ), model, SLOT( switchWarn( bool ) ) );
        QObject::connect( ui.actionINFO, SIGNAL( triggered( bool ) ), model, SLOT( switchInfo( bool ) ) );
        QObject::connect( ui.actionTEST, SIGNAL( triggered( bool ) ), model, SLOT( switchTest( bool ) ) );
        QObject::connect( ui.actionDEBUG, SIGNAL( triggered( bool ) ), model, SLOT( switchDbg( bool ) ) );
        QObject::connect( ui.actionMonitor_Log, SIGNAL( triggered( bool ) ), model, SLOT( switchMon( bool ) ) );
        QObject::connect( ui.actionTEST_log, SIGNAL( triggered( bool ) ), model, SLOT( switchRpc( bool ) ) );
        QObject::connect(ui.actionTM_log, SIGNAL(triggered(bool)), model, SLOT(switchTm(bool)));
        QObject::connect(ui.actionZLG_log, SIGNAL(triggered(bool)), model, SLOT(switchZlg(bool)));
        QObject::connect(ui.actionMonitor_1_Log, SIGNAL(triggered(bool)), model, SLOT(switchMo1(bool)));
        QObject::connect(ui.actionMonitor_2_Log, SIGNAL(triggered(bool)), model, SLOT(switchMo2(bool)));
        QObject::connect(ui.actionMonitor_3_Log, SIGNAL(triggered(bool)), model, SLOT(switchMo3(bool)));
        QObject::connect(ui.actionMonitor_4_Log, SIGNAL(triggered(bool)), model, SLOT(switchMo4(bool)));

        ui.centralWidget->setModel(model);
        stModelFlags();
        model->ResetData();
    }
}


LogViewQt::~LogViewQt( )
{
	delete model;
}

void LogViewQt::changeTable()
{
    ui.centralWidget->setWordWrap(true);
    ui.centralWidget->resizeColumnsToContents();
//    ui.centralWidget->resizeRowsToContents();
}

void LogViewQt::resetRow(int row)
{
    ui.centralWidget->resizeRowToContents(row);
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

void LogViewQt::findText()
{
    QScopedPointer<FDialog> dlg(new FDialog(Q_NULLPTR));

    if (dlg)
    {
        int result = dlg->exec();
        if (result == 1)
        {
            QString fStr = dlg->getText();
            setCursor(Qt::WaitCursor);
            int row = findRow(fStr, 0);
            setCursor(Qt::ArrowCursor);
            if (row >= 0)
            {
                foundStr = fStr;
                foundRow = row;
                QModelIndex idx = model->index(row, 1);
                ui.centralWidget->setCurrentIndex(idx);
            }
        }
    }
}

void LogViewQt::findTextNext()
{
    setCursor(Qt::WaitCursor);
    int row = findRow(foundStr, foundRow + 1);
    setCursor(Qt::ArrowCursor);
    if (row >= 0)
    {
        foundRow = row;
        QModelIndex idx = model->index(row, 1);
        ui.centralWidget->setCurrentIndex(idx);
    }
}

void LogViewQt::findTextPrev()
{
    if (foundRow > 0)
    {
        setCursor(Qt::WaitCursor);
        int row = findRowPrev(foundStr, foundRow - 1);
        setCursor(Qt::ArrowCursor);
        if (row >= 0)
        {
            foundRow = row;
            QModelIndex idx = model->index(row, 1);
            ui.centralWidget->setCurrentIndex(idx);
        }
    }
}

void LogViewQt::customContextMenuRequested(const QPoint &pos)
{
    selectMnu->exec(ui.centralWidget->mapToGlobal(pos));
}

void LogViewQt::selectText()
{
    QModelIndex index = ui.centralWidget->selectionModel()->currentIndex();
    QString data = model->data(index, Qt::DisplayRole).toString();
    QClipboard *cb = qApp->clipboard();
    cb->setText(data);
}

void LogViewQt::readSettings()
{
    QSettings settings(confName, QSettings::IniFormat, Q_NULLPTR);
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
            (availableGeometry.height() - height()) / 2);

        settings.setValue("/Severity/CRIT", ui.actionCRIT->isChecked());
        settings.setValue("/Severity/ERROR", ui.actionERROR->isChecked());
        settings.setValue("/Severity/WARN", ui.actionWARN->isChecked());
        settings.setValue("/Severity/TEST", ui.actionTEST->isChecked());
        settings.setValue("/Severity/INFO", ui.actionINFO->isChecked());
        settings.setValue("/Severity/DEBUG", ui.actionDEBUG->isChecked());

        settings.setValue("/Monitor/Uart1", ui.actionMonitor_1_Log->isChecked());
        settings.setValue("/Monitor/Uart2", ui.actionMonitor_2_Log->isChecked());
        settings.setValue("/Monitor/Uart3", ui.actionMonitor_3_Log->isChecked());
        settings.setValue("/Monitor/Uart4", ui.actionMonitor_4_Log->isChecked());

        settings.setValue("/Module/Monitor", ui.actionMonitor_Log->isChecked());
        settings.setValue("/Module/Test", ui.actionTEST_log->isChecked());
        settings.setValue("/Module/TM", ui.actionTM_log->isChecked());
        settings.setValue("/Module/Zlog", ui.actionZLG_log->isChecked());
    }
    else
    {
        restoreGeometry(geometry);
        bool isChecked = settings.value("/Severity/CRIT", true).toBool();
        ui.actionCRIT->setChecked(isChecked);
        isChecked = settings.value("/Severity/ERROR", true).toBool();
        ui.actionERROR->setChecked(isChecked);
        isChecked = settings.value("/Severity/WARN", true).toBool();
        ui.actionWARN->setChecked(isChecked);
        isChecked = settings.value("/Severity/TEST", true).toBool();
        ui.actionTEST->setChecked(isChecked);
        isChecked = settings.value("/Severity/INFO", true).toBool();
        ui.actionINFO->setChecked(isChecked);
        isChecked = settings.value("/Severity/DEBUG", true).toBool();
        ui.actionDEBUG->setChecked(isChecked);

        isChecked = settings.value("/Monitor/Uart1", true).toBool();
        ui.actionMonitor_1_Log->setChecked(isChecked);
        isChecked = settings.value("/Monitor/Uart2", true).toBool();
        ui.actionMonitor_2_Log->setChecked(isChecked);
        isChecked = settings.value("/Monitor/Uart3", false).toBool();
        ui.actionMonitor_3_Log->setChecked(isChecked);
        isChecked = settings.value("/Monitor/Uart4", false).toBool();
        ui.actionMonitor_4_Log->setChecked(isChecked);

        isChecked = settings.value("/Module/Monitor", true).toBool();
        ui.actionMonitor_Log->setChecked(isChecked);
        isChecked = settings.value("/Module/Test", true).toBool();
        ui.actionTEST_log->setChecked(isChecked);
        isChecked = settings.value("/Module/TM", false).toBool();
        ui.actionTM_log->setChecked(isChecked);
        isChecked = settings.value("/Module/Zlog", false).toBool();
        ui.actionZLG_log->setChecked(isChecked);
    }
}

void LogViewQt::writeSettings()
{
    QSettings settings(confName, QSettings::IniFormat, Q_NULLPTR);
    settings.setValue("geometry", saveGeometry());

    settings.setValue("/Severity/CRIT", ui.actionCRIT->isChecked());
    settings.setValue("/Severity/ERROR", ui.actionERROR->isChecked());
    settings.setValue("/Severity/WARN", ui.actionWARN->isChecked());
    settings.setValue("/Severity/TEST", ui.actionTEST->isChecked());
    settings.setValue("/Severity/INFO", ui.actionINFO->isChecked());
    settings.setValue("/Severity/DEBUG", ui.actionDEBUG->isChecked());

    settings.setValue("/Monitor/Uart1", ui.actionMonitor_1_Log->isChecked());
    settings.setValue("/Monitor/Uart2", ui.actionMonitor_2_Log->isChecked());
    settings.setValue("/Monitor/Uart3", ui.actionMonitor_3_Log->isChecked());
    settings.setValue("/Monitor/Uart4", ui.actionMonitor_4_Log->isChecked());

    settings.setValue("/Module/Monitor", ui.actionMonitor_Log->isChecked());
    settings.setValue("/Module/Test", ui.actionTEST_log->isChecked());
    settings.setValue("/Module/TM", ui.actionTM_log->isChecked());
    settings.setValue("/Module/Zlog", ui.actionZLG_log->isChecked());
}

void LogViewQt::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void LogViewQt::stModelFlags() const
{
    model->SetFlagCrit(ui.actionCRIT->isChecked());
    model->SetFlagError(ui.actionERROR->isChecked());
    model->SetFlagWarn(ui.actionWARN->isChecked());
    model->SetFlagTest(ui.actionTEST->isChecked());
    model->SetFlagInfo(ui.actionINFO->isChecked());
    model->SetFlagDebug(ui.actionDEBUG->isChecked());
    model->SetFlagMo1(ui.actionMonitor_1_Log->isChecked());
    model->SetFlagMo2(ui.actionMonitor_2_Log->isChecked());
    model->SetFlagMo3(ui.actionMonitor_3_Log->isChecked());
    model->SetFlagMo4(ui.actionMonitor_4_Log->isChecked());
    model->SetFlagMon(ui.actionMonitor_Log->isChecked());
    model->SetFlagRpc(ui.actionTEST_log->isChecked());
    model->SetFlagTm(ui.actionTM_log->isChecked());
    model->SetFlagZlg(ui.actionZLG_log->isChecked());
}
