#include "logviewqt.h"
#include "finddialog.h"
#include <QFileDialog>
#include <QSettings>
#include <QtWidgets>
#include <QTimer>
#include <QScreen>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string_regex.hpp>

QString confName;

LogViewQt::LogViewQt( const QString& fname, QWidget *parent)
    : QMainWindow(parent)
    , model(nullptr)
    , foundRow(0)
    , modules(nullptr)
{
    ui.setupUi(this);
    modules = new QComboBox( this );
    ui.toolBar->addWidget( modules );

	QObject::connect(ui.actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(selectOpenFileName()));
    QObject::connect(ui.actionFind, SIGNAL(triggered()), this, SLOT(findText()));
    QObject::connect(ui.actionFindCurrent, SIGNAL(triggered()), this, SLOT(findTextCurent()));
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

    if (!fname.isEmpty())
    {
        QSettings settings(confName, QSettings::IniFormat, Q_NULLPTR);
        std::string sp = fname.toStdString();
        boost::filesystem::path p(sp);
        boost::filesystem::path fp = boost::filesystem::absolute(p).parent_path();
        settings.setValue("FilesFolder", QString::fromStdString(fp.string()));
    }

    actionEnable( 0xFFFFFFFF );
    createModel( fname );

    checkSelectTimer = new QTimer(this);
    checkSelectTimer->setInterval(300);
    connect(checkSelectTimer, SIGNAL(timeout()), this, SLOT(selectTickTimer()));
}

void LogViewQt::createModel( const QString& fname )
{
    ui.statusBar->showMessage( fname );

    if ( !fname.isEmpty( ) )
    {
        modules->setModel( nullptr );
        ui.centralWidget->setModel( nullptr );
        delete model;

        model = new LogModel( fname );
        QObject::connect(model, SIGNAL(changeModel()), this, SLOT(changeTable()));
        QObject::connect(model, SIGNAL(resetRow(int)), this, SLOT(resetRow(int)));
        QObject::connect( model, SIGNAL( filterEnable( quint32 ) ), this, SLOT( actionEnable( quint32 ) ) );
        QObject::connect( model, SIGNAL( setColumnEnable( quint32 ) ), this, SLOT( setColumnEnable( quint32 ) ) );
        QObject::connect( model, SIGNAL( setComboModel( QAbstractItemModel * ) ), this, SLOT( setComboModel( QAbstractItemModel * ) ) );
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
        QObject::connect( modules, SIGNAL( activated( int ) ), model, SLOT( switchModule( int ) ) );

        ui.centralWidget->setModel(model);
        stModelFlags();
        model->ResetData();
    }
}


LogViewQt::~LogViewQt( )
{
 	delete model;
}

void LogViewQt::actionEnable(quint32 mask)
{
    ui.actionMonitor_1_Log->setEnabled( (mask & MON_LINE1) ? true : false );
    ui.actionMonitor_2_Log->setEnabled( (mask & MON_LINE2) ? true : false );
    ui.actionMonitor_3_Log->setEnabled( (mask & MON_LINE3) ? true : false );
    ui.actionMonitor_4_Log->setEnabled( (mask & MON_LINE4) ? true : false );
    ui.actionMonitor_Log->setEnabled( (mask & MON_MODULE) ? true : false );
    ui.actionTEST_log->setEnabled( (mask & TEST_MODULE) ? true : false );
    ui.actionZLG_log->setEnabled( (mask & ZLG_MODULE) ? true : false );
    ui.actionTM_log->setEnabled( (mask & RTM_MODULE) ? true : false );
    ui.actionCRIT->setEnabled( (mask & SEVERITY_CRIT) ? true : false );
    ui.actionWARN->setEnabled( (mask & SEVERITY_WARN) ? true : false );
    ui.actionTEST->setEnabled( (mask & SEVERITY_TEST) ? true : false );
    ui.actionINFO->setEnabled( (mask & SEVERITY_INFO) ? true : false );
    ui.actionDEBUG->setEnabled( (mask & SEVERITY_DEBUG) ? true : false );
    ui.actionERROR->setEnabled( (mask & SEVERITY_ERR) ? true : false );
}

void LogViewQt::changeTable()
{
    ui.centralWidget->setWordWrap(true);
    ui.centralWidget->resizeColumnsToContents();
}

void LogViewQt::resetRow(int row)
{
    ui.centralWidget->resizeRowToContents(row);
}

void LogViewQt::selectOpenFileName( )
{
    QSettings settings(confName, QSettings::IniFormat, Q_NULLPTR);
    QString folder = settings.value("FilesFolder", QDir::currentPath()).toString();
    QString fname = QFileDialog::getOpenFileName( nullptr, "Open File ...", folder, "Logs (*.log *.elog);;Text files (*.txt);;XML files (*.xml)" );

    try
    {
        QFile f( fname );
        if (f.exists())
        {
            std::string sp = fname.toStdString();
            boost::filesystem::path p( sp );
            boost::filesystem::path fp = boost::filesystem::absolute( p ).parent_path();
            settings.setValue( "FilesFolder", QString::fromStdString( fp.string() ) );
            createModel( fname );
        }
    }
    catch (const std::exception&)
    {
        throw std::runtime_error( "Error open or wrong format " + fname.toStdString() );
    }
}

void LogViewQt::findInTable(int fromRow)
{
    QScopedPointer<FDialog> dlg(new FDialog(Q_NULLPTR));

    if (dlg)
    {
        int result = dlg->exec();
        if (result == 1)
        {
            QString fStr = dlg->getText();
            setCursor(Qt::WaitCursor);
            int row = findRow(fStr, fromRow);
            setCursor(Qt::ArrowCursor);
            if (row >= 0)
            {
                foundStr = fStr;
                foundRow = row;
                QModelIndex idx = model->index(row, 3);
                ui.centralWidget->setCurrentIndex(idx);
                checkSelectTimer->start();
            }
        }
    }
}

void LogViewQt::findText()
{
    findInTable(0);
}

void LogViewQt::findTextCurent()
{
    QModelIndex idx = ui.centralWidget->selectionModel()->currentIndex();
    int currentRow = 0;
    if (idx.isValid())
    {
        currentRow = idx.row() + 1;
    }
    findInTable(currentRow);
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
        checkSelectTimer->start();
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
            checkSelectTimer->start();
        }
    }
}

void LogViewQt::customContextMenuRequested(const QPoint &pos)
{
    selectMnu->exec(ui.centralWidget->mapToGlobal(pos));
}


void LogViewQt::setColumnEnable( quint32 mask )
{
    if (mask & COLUMN_1_MASK) ui.centralWidget->showColumn( 1 );
    else ui.centralWidget->hideColumn( 1 );

    if (mask & COLUMN_2_MASK) ui.centralWidget->showColumn( 2 );
    else ui.centralWidget->hideColumn( 2 );

    if (mask & COLUMN_3_MASK) ui.centralWidget->showColumn( 3 );
    else ui.centralWidget->hideColumn( 3 );

    if (mask & COLUMN_4_MASK) ui.centralWidget->showColumn( 4 );
    else ui.centralWidget->hideColumn( 4 );
}

void LogViewQt::selectTickTimer()
{
    checkSelectTimer->stop();
    int firstRow = ui.centralWidget->rowAt(0);
    int lastRow = ui.centralWidget->rowAt(ui.centralWidget->height());

    if (!(firstRow <= foundRow && lastRow > foundRow))
    {
        foundRow = firstRow;
        QModelIndex idx = model->index(foundRow, 1);
        ui.centralWidget->setCurrentIndex(idx);
        findTextNext();
    }
}

void LogViewQt::setComboModel( QAbstractItemModel* mdl )
{
    modules->setModel( mdl );
}

void LogViewQt::selectText()
{
    QItemSelectionModel* selection = ui.centralWidget->selectionModel();
    int curColumn = selection->currentIndex().column();
    QModelIndexList selList = selection->selectedRows(curColumn);
    QString data;
    for (auto& sel : selList)
    {
        QString line = model->data(sel, Qt::DisplayRole).toString();
        line.replace('\0', ' ');
        data.append(line);
        data.append("\n");
    }
    std::string str = data.toStdString();
    boost::algorithm::erase_all_regex(str, boost::regex{R"(\x1B\[([0-9]+;)*[0-9]*[m|h|l]{1})"});
    data = QString::fromStdString(str);
    QClipboard *cb = qApp->clipboard();
    cb->setText(data);
}

void LogViewQt::readSettings()
{
    QSettings settings(confName, QSettings::IniFormat, Q_NULLPTR);
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = screen()->availableGeometry(); // QApplication::desktop()->availableGeometry(this);
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
