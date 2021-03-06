#define _SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING

#include "logmodel.h"
#include "combomodel.h"
#include <boost/filesystem.hpp>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <QColor>
#include <QBrush>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

quint32 FULL_HEADER_LEN = 40;
quint32 LAST_HEADER_POS = 39;
quint32 BEGIN_TIMESTAMP_OFFSET = 13;
quint32 TIMESTAMP_LEN = 15;
quint32 BEGIN_MODULE_NAME_OFFSET = 30;
quint32 MODULE_NAME_LEN = 3;
quint32 BEGIN_SEVERITY_OFFSET = 35;
quint32 SEVERITY_LEN = 4;
quint32 BEGIN_LOG_STRING_OFFSET = 43;

static const std::string strExprRpcLog( R"(^((\[(\d{4}-[A-Za-z]{3}-\d{2}\s\d{2}:\d{2}:\d{2}\.\d{6})\]\[([A-Z0-9\s]+)\]\[([A-Z ]+)\])\s-\s))" );
static const std::string strExprRpcLog2( R"(^((\[(\d{2}:\d{2}:\d{2}\.\d{6})\]\[([A-Z0-9\s]+)\]\[([A-Z ]+)\])\s-\s))" );
static const std::string strExprUartLog( R"(^((\[(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}\.\d{3})\])\s))" );
static const std::string strExprDmesgLog( R"(^((\[(\d+\.\d{6})\])\s(.*)$))" );
static const std::string strExprTestRunnerLog( R"(^(((\d{2}:\d{2}:\d{2},\d{3})\s([A-Z\s]{5}))\s-\s)(.*)$)" );
std::string strExprRpcLogCurrent;

ELogType LogModel::AnaliseFileFormat(const std::string& fname, size_t fSize)
{
    ELogType logType = ELogType::ECommonText;

    try
    {
        std::ifstream ifs(fname, std::ios::binary);
        const size_t blockSize = std::min<size_t>(100, fSize);
        std::string line(blockSize, '\0');
        ifs.read(&line[0], blockSize);
        ifs.close();
        int logVar = 0;

        // remove first empty lines
        boost::algorithm::trim_left_if( line, boost::is_any_of( "\r\n" ) );
        boost::smatch what;

        if (boost::regex_search( line, what, boost::regex( strExprRpcLog ) ))
        {
            strExprRpcLogCurrent = strExprRpcLog;
            logVar = 1;
        }
        else if (boost::regex_search( line, what, boost::regex( strExprRpcLog2 ) ))
        {
            strExprRpcLogCurrent = strExprRpcLog2;
            logVar = 2;
        }

        if (logVar)
        {
            BEGIN_LOG_STRING_OFFSET = what[1].length();
            FULL_HEADER_LEN = what[2].length();
            TIMESTAMP_LEN = what[3].length();
            MODULE_NAME_LEN = what[4].length();
            SEVERITY_LEN = what[5].length();

            LAST_HEADER_POS = FULL_HEADER_LEN - 1;
            BEGIN_TIMESTAMP_OFFSET = logVar == 1 ? 13 : 1;
            BEGIN_MODULE_NAME_OFFSET = TIMESTAMP_LEN + 3;
            BEGIN_SEVERITY_OFFSET = TIMESTAMP_LEN + MODULE_NAME_LEN + 5;
            if (logVar == 1)
            {
                TIMESTAMP_LEN -= 13;
            }
            logType = ELogType::EEtfRpcLog;
        }
        else if (boost::regex_search( line, what, boost::regex( strExprTestRunnerLog ) ))
        {
            BEGIN_LOG_STRING_OFFSET = what[1].length();
            FULL_HEADER_LEN = what[2].length();
            TIMESTAMP_LEN = what[3].length();
            MODULE_NAME_LEN = 0;
            SEVERITY_LEN = what[4].length();
            quint32 tsLen = 0;

            LAST_HEADER_POS = FULL_HEADER_LEN;
            BEGIN_TIMESTAMP_OFFSET = 0;
            BEGIN_MODULE_NAME_OFFSET = 0;
            BEGIN_SEVERITY_OFFSET = TIMESTAMP_LEN + 1;
            logType = ELogType::ETestRunnerLog;
        }
        else if (boost::regex_search( line, what, boost::regex( strExprUartLog ) ))
        {
            BEGIN_LOG_STRING_OFFSET = what[1].length();
            FULL_HEADER_LEN = what[2].length();
            TIMESTAMP_LEN = what[3].length();
            MODULE_NAME_LEN = 0;
            SEVERITY_LEN = 0;

            LAST_HEADER_POS = FULL_HEADER_LEN - 1;
            BEGIN_TIMESTAMP_OFFSET = 12;
            BEGIN_MODULE_NAME_OFFSET = 0;
            BEGIN_SEVERITY_OFFSET = 0;
            TIMESTAMP_LEN = 12;
            logType = ELogType::EUartLog;
        }
        else if (boost::regex_search( line, what, boost::regex( strExprDmesgLog ) ))
        {
            BEGIN_LOG_STRING_OFFSET = what[1].length();
            FULL_HEADER_LEN = what[2].length();
            TIMESTAMP_LEN = what[3].length();
            MODULE_NAME_LEN = 0;
            SEVERITY_LEN = 0;

            LAST_HEADER_POS = FULL_HEADER_LEN - 1;
            BEGIN_TIMESTAMP_OFFSET = 12;
            BEGIN_MODULE_NAME_OFFSET = 0;
            BEGIN_SEVERITY_OFFSET = 0;
            TIMESTAMP_LEN = 12;
            logType = ELogType::EDmesgLog;
        }
        else // ECommonText
        {
            BEGIN_LOG_STRING_OFFSET = 0;
            FULL_HEADER_LEN = 0;
            TIMESTAMP_LEN = 0;
            MODULE_NAME_LEN = 0;
            SEVERITY_LEN = 0;
            quint32 tsLen = 0;

            LAST_HEADER_POS = 0;
            BEGIN_TIMESTAMP_OFFSET = 0;
            BEGIN_MODULE_NAME_OFFSET = 0;
            BEGIN_SEVERITY_OFFSET = 0;
            logType = ELogType::ECommonText;
        }
    }
    catch (const std::exception&)
    {

    }
    
    return logType;
}

//////////////////////////////////////////////////////////////////////////
LogModel::LogModel(const QString& fname, QObject* parent)
    : QAbstractTableModel(parent)
    , file_name(fname)
    , buffer_size(0)
    , mod_mask(0x00FF0000)
    , mon_mask(0x0000FF00)
    , sev_mask(0x000000FF)
    , tmr(Q_NULLPTR)
    , logType(ELogType::ECommonText)
    , colMask(0)
    , cbModel(nullptr)
    , keyModule(0)
    , isModuleNamesSet(false)
{
    bool status = false;
    std::string stdFName = fname.toStdString();

    if (boost::filesystem::exists(stdFName))
    {
        buffer_size = boost::filesystem::file_size(stdFName);
        file.open( stdFName, buffer_size );
        if (file.is_open())
        {
            const char* buffer = file.data();
            const char* const buffer_end = buffer + buffer_size;
            logType = AnaliseFileFormat( stdFName, buffer_size );


            if (logType == ELogType::EEtfRpcLog)
            {
                boost::regex expr{ strExprRpcLogCurrent };
                boost::cmatch what;
                // parse file
                for (size_t pos = 0; pos < buffer_size;)
                {
                    DataValue dv = { nullptr, nullptr, 0, 0 };
                    size_t old = pos;
                    while (pos < buffer_size && (buffer[pos] != '[')) ++pos;
                    if (boost::regex_search( buffer + pos, buffer + std::min<size_t>( pos + 50, buffer_size ), what, expr ))
                    {
                        dv.begin = buffer + pos;
                        while (pos < buffer_size && (buffer[pos] != '\r' && buffer[pos] != '\n')) ++pos;
                        dv.end = buffer + pos;
                    }
                    else
                    {
                        break;
                    }
                    iteration:
                    while (pos < buffer_size && (buffer[pos] == '\r' || buffer[pos] == '\n')) ++pos;
                    if (boost::regex_search( buffer + pos, buffer + std::min<size_t>( pos + 50, buffer_size ), what, expr ) || buffer_size == pos)
                    {
                        SetFlags( dv );
                        vstorage.push_back( dv );
                    }
                    else
                    {
                        while (pos < buffer_size && (buffer[pos] != '\r' && buffer[pos] != '\n')) ++pos;
                        dv.end = buffer + pos;
                        goto iteration;
                    }
                }

                SetLogFont();
                status = true;
            }
            else
            {
                for (size_t pos = 0; pos < buffer_size; ++pos)
                {
                    DataValue dv = { nullptr, nullptr, 0, 0 };
                    while (pos < buffer_size && (buffer[pos] == '\r' || buffer[pos] == '\n')) ++pos;
                    dv.begin = buffer + pos;
                    while (pos < buffer_size && (buffer[pos] != '\r' && buffer[pos] != '\n')) ++pos;
                    dv.end = buffer + pos;
                    SetFlags( dv );
                    vstorage.push_back(dv);
                }
                SetLogFont();
                status = true;
            }
        }
    }

    if (!vstorage.empty() && logType == ELogType::EEtfRpcLog)
    {
        std::vector<std::string> listModules;
        for (auto& dv : vstorage)
        {
            std::string n( dv.begin + BEGIN_MODULE_NAME_OFFSET, MODULE_NAME_LEN );
            if (n == "18." || n == "the" || n == "[sh" || n == "5][")
            {
                continue;
            }
            listModules.emplace_back( dv.begin + BEGIN_MODULE_NAME_OFFSET, MODULE_NAME_LEN );
        }
        std::sort( listModules.begin(), listModules.end() );
        auto last = std::unique( listModules.begin(), listModules.end() );
        listModules.erase( last, listModules.end() );
        if (!listModules.empty())
        {
            quint32 key = 0;
            cbModel = new ComboModel();
            for (auto& name : listModules)
            {
                mapModulesByName.insert( std::make_pair( ++key, name ) );
                cbModel->addItem( QString::fromStdString(name), key );
            }

            auto setModuleKey = [this]( DataValue& dv )
            {
                std::string m( dv.begin + BEGIN_MODULE_NAME_OFFSET, MODULE_NAME_LEN );
                for (quint32 k = 1; k <= mapModulesByName.size(); ++k)
                {
                    if (mapModulesByName[k] == m)
                    {
                        dv.key = k;
                    }
                }
            };

            for (auto& dv : vstorage)
            {
                setModuleKey( dv );
            }
        }
    }

    if(!status)
    {
        throw std::runtime_error("error file operation");
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::SetLogFont()
{
    // set fonts
    font.setFamily( "Courier New" );
    font.setPointSize( 10 );
    fbold = font;
    fbold.setBold( true );
    tmr = new QTimer( this );
    tmr->setInterval( 100 );
    connect( tmr, SIGNAL( timeout() ), this, SLOT( tickTimer() ) );
}

//////////////////////////////////////////////////////////////////////////
LogModel::~LogModel()
{
    file.close();
    delete cbModel;
}

//////////////////////////////////////////////////////////////////////////
int LogModel::rowCount( const QModelIndex & /*parent*/ ) const
{
    return vdata.size();
}

//////////////////////////////////////////////////////////////////////////
int LogModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return 4;
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::data( const QModelIndex &index, int role ) const
{
    int colIdx = RecalculateColumnIndex( index.column() );

    switch ( role )
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return HandleDisplayRole( colIdx, index.row());

    case Qt::FontRole:
        return (colIdx == 2 || colIdx == 0 ) ? fbold : font;

    case Qt::BackgroundRole:
        return HandleBackgroundRole( colIdx, index.row());

    case Qt::ForegroundRole: // TextColorRole:
        return HandleTextColorRole( colIdx, index.row());

    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    return QVariant( );
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    int colIdx = RecalculateColumnIndex( section );

    switch ( role )
    {
    case Qt::DisplayRole:
        return HandleHeaderDisplayRole(orientation, colIdx );

    case Qt::TextAlignmentRole:
        return QVariant( Qt::AlignLeft | Qt::AlignVCenter );

    case Qt::FontRole:
        return fbold;
    }

    return QVariant( );
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::HandleDisplayRole(int col, int row) const
{
    const DataValue * const dv = vdata[row];
    if (logType == ELogType::EDmesgLog)
    {
        static boost::regex expr{ strExprDmesgLog };
        boost::cmatch what;
        if (boost::regex_search( dv->begin, dv->end, what, expr ))
        {
            switch (col)
            {
            case 0:
                return QString::fromStdString( std::string( what[3] ) );
            case 3:
                return QString::fromStdString( std::string( what[4] ) );
            }
        }
    }
    else
    {
        switch (col)
        {
        case 0:
            return QString::fromStdString( std::string( dv->begin + BEGIN_TIMESTAMP_OFFSET, TIMESTAMP_LEN ) );
        case 1:
            return QString::fromStdString( std::string( dv->begin + BEGIN_MODULE_NAME_OFFSET, MODULE_NAME_LEN ) );
        case 2:
            return QString::fromStdString( std::string( dv->begin + BEGIN_SEVERITY_OFFSET, SEVERITY_LEN ) );
        case 3:
            return QString::fromStdString( boost::algorithm::trim_copy_if( std::string( dv->begin + BEGIN_LOG_STRING_OFFSET, dv->end ), boost::is_any_of( "\r\n" ) ) );
        }
    }
    return QVariant();
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::HandleBackgroundRole(int col, int row) const
{
    if ( col == 3 )
    {
        const DataValue * const dv = vdata[row];
        indxs.push_back(row);

        if ( dv->flags & SEVERITY_WARN )
            return QBrush( QColor(255, 191, 223) );
        else if (dv->flags & SEVERITY_TEST)
            return QBrush( QColor(170, 255, 170) );
        else if (dv->flags & SEVERITY_INFO)
            return QBrush( QColor(171, 185, 255) );
        else if (dv->flags & (SEVERITY_ERR | SEVERITY_CRIT) )
            return QBrush( QColor(255, 150, 150) );
        else if (dv->flags & MON_MODULE)
            return QBrush( QColor(255, 255, 128) );
        else
            return QBrush( QColor(Qt::white) );
    }
    else if ( row % 2 != 0 )
    {
        return QBrush (QColor(232, 232, 232));
    }
    return QVariant();
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::HandleTextColorRole(int col, int row) const
{
    const DataValue * const dv = vdata[row];
    if ( col == 2 )
    {
        if (dv->flags & SEVERITY_WARN)
            return QColor(Qt::darkMagenta);
        else if (dv->flags & SEVERITY_TEST)
            return QColor(Qt::darkGreen);
        else if (dv->flags & SEVERITY_INFO)
            return QColor(Qt::blue);
        else if (dv->flags & (SEVERITY_ERR | SEVERITY_CRIT))
            return QColor(Qt::red);
    }
    else if (col == 1)
    {
        if (dv->flags & MON_MODULE)
            return QColor(Qt::darkMagenta);
        else if (dv->flags & RTM_MODULE)
            return QColor(Qt::darkGreen);
        else if (dv->flags & TEST_MODULE)
            return QColor(Qt::blue);
        else if (dv->flags & ZLG_MODULE)
            return QColor(Qt::darkYellow);
    }
    return QVariant();
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::HandleHeaderDisplayRole(Qt::Orientation orientation, int section) const
{
    if ( orientation == Qt::Horizontal ) {
        switch ( section )
        {
        case 0:
            return QString( "TimeStamp" );
        case 1:
            return QString( "Module" );
        case 2:
            return QString( "Severity" );
        case 3:
            return QString( "Data" );
        }
    }
    return QVariant();
}

//////////////////////////////////////////////////////////////////////////
void LogModel::ResetData()
{
    if (!isModuleNamesSet)
    {
        emit setComboModel( cbModel );
        isModuleNamesSet = true;
    }

    tmr->stop();
    beginResetModel();
    vdata.clear();
    indxs.clear();
    for (DataValue& dv : vstorage)
    {
        if (keyModule == 0 || keyModule == dv.key)
        {
            if (dv.flags & mod_mask)
            {
                if (dv.flags & sev_mask)
                {
                    if (!(dv.flags & MON_MODULE) || (dv.flags & mon_mask))
                    {
                        vdata.push_back( &dv );
                    }
                }
            }
        }
    }
    endResetModel();
    tmr->start();
    emit changeModel();
    quint32 mask = 0;
    switch (logType)
    {
    case ELogType::EEtfRpcLog:
        mask = MOD_MASK | MON_MASK | SEV_MASK;
        colMask = COLUMN_1_MASK | COLUMN_2_MASK | COLUMN_3_MASK | COLUMN_4_MASK;
        break;
    case ELogType::EUartLog:
    case ELogType::EDmesgLog:
        colMask = COLUMN_1_MASK | COLUMN_4_MASK;
        break;
    case ELogType::ETestRunnerLog:
        mask = SEV_MASK;
        colMask = COLUMN_1_MASK | COLUMN_3_MASK | COLUMN_4_MASK;
        break;
    case ELogType::ECommonText:
    default:
        colMask = COLUMN_4_MASK;
        break;
    }
    emit filterEnable( mask );
    emit setColumnEnable( colMask );
}

//////////////////////////////////////////////////////////////////////////
Qt::ItemFlags LogModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags val = Qt::ItemIsEnabled;
    val |= Qt::ItemIsSelectable;
    val |= Qt::ItemIsEditable;
    return val;
}

//////////////////////////////////////////////////////////////////////////
void LogModel::SetFlags(DataValue& dv)
{
    dv.flags = 0;

    if (BEGIN_SEVERITY_OFFSET > 0)
    {
        const char* const sevPtr = dv.begin + BEGIN_SEVERITY_OFFSET;
        if (memcmp( sevPtr, "CRIT", 4 ) == 0)
        {
            dv.flags |= SEVERITY_CRIT;
        }
        else if (memcmp( sevPtr, "ERR ", 3 ) == 0)
        {
            dv.flags |= SEVERITY_ERR;
        }
        else if (memcmp( sevPtr, "WARN", 4 ) == 0)
        {
            dv.flags |= SEVERITY_WARN;
        }
        else if (memcmp( sevPtr, "INFO", 4 ) == 0)
        {
            dv.flags |= SEVERITY_INFO;
        }
        else if (memcmp( sevPtr, "TEST", 4 ) == 0)
        {
            dv.flags |= SEVERITY_TEST;
        }
        else if (memcmp( sevPtr, "DBG ", 3 ) == 0 || memcmp( sevPtr, "DEBUG ", 5 ) == 0)
        {
            dv.flags |= SEVERITY_DEBUG;
        }
    }
    else
    {
        dv.flags |= SEVERITY_TEST;
    }

    if (BEGIN_MODULE_NAME_OFFSET > 0)
    {
        const char* const modPtr = dv.begin + BEGIN_MODULE_NAME_OFFSET;
        if (memcmp( modPtr, "SP01", MODULE_NAME_LEN ) == 0 || memcmp( modPtr, "MON ", MODULE_NAME_LEN ) == 0)
        {
            dv.flags |= MON_LINE1;
            dv.flags |= MON_MODULE;
        }
        else if (memcmp( modPtr, "SP02", MODULE_NAME_LEN ) == 0 || memcmp( modPtr, "MO2 ", MODULE_NAME_LEN ) == 0)
        {
            dv.flags |= MON_LINE2;
            dv.flags |= MON_MODULE;
        }
        else if (memcmp( modPtr, "SP03", MODULE_NAME_LEN ) == 0 || memcmp( modPtr, "MO3 ", MODULE_NAME_LEN ) == 0)
        {
            dv.flags |= MON_LINE3;
            dv.flags |= MON_MODULE;
        }
        else if (memcmp( modPtr, "SP04", MODULE_NAME_LEN ) == 0 || memcmp( modPtr, "MO4 ", MODULE_NAME_LEN ) == 0)
        {
            dv.flags |= MON_LINE4;
            dv.flags |= MON_MODULE;
        }
        else if (memcmp( modPtr, "TM  ", MODULE_NAME_LEN ) == 0)
        {
            dv.flags |= RTM_MODULE;
        }
        else if (memcmp( modPtr, "ZLG ", MODULE_NAME_LEN ) == 0)
        {
            dv.flags |= ZLG_MODULE;
        }
        else
        {
            dv.flags |= TEST_MODULE;
        }
    }
    else
    {
        dv.flags |= TEST_MODULE;
    }
}


int LogModel::RecalculateColumnIndex( int idx ) const
{
    int result = idx;
    switch (logType)
    {
    case ELogType::EEtfRpcLog:
        break;
    case ELogType::EUartLog:
    case ELogType::EDmesgLog:
        if (idx == 1) result = 3;
        break;
    case ELogType::ETestRunnerLog:
        if (idx == 1) result = 2;
        if (idx == 2) result = 3;
        break;
    case ELogType::ECommonText:
    default:
        result = 3;
        break;
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchCrit(bool check)
{
    SetFlagCrit(check);
    ResetData();
}

void LogModel::SetFlagCrit(bool check)
{
    if (check)
    {
        sev_mask |= SEVERITY_CRIT;
    }
    else
    {
        sev_mask &= ~SEVERITY_CRIT;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchErr(bool check)
{
    SetFlagError(check);
    ResetData();
}

void LogModel::SetFlagError(bool check)
{
    if (check)
    {
        sev_mask |= SEVERITY_ERR;
    }
    else
    {
        sev_mask &= ~SEVERITY_ERR;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchWarn(bool check)
{
    SetFlagWarn(check);
    ResetData();
}

void LogModel::SetFlagWarn(bool check)
{
    if (check)
    {
        sev_mask |= SEVERITY_WARN;
    }
    else
    {
        sev_mask &= ~SEVERITY_WARN;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchTest(bool check)
{
    SetFlagTest(check);
    ResetData();
}

void LogModel::SetFlagTest(bool check)
{
    if (check)
    {
        sev_mask |= SEVERITY_TEST;
    }
    else
    {
        sev_mask &= ~SEVERITY_TEST;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchInfo(bool check)
{
    SetFlagInfo(check);
    ResetData();
}

void LogModel::SetFlagInfo(bool check)
{
    if (check)
    {
        sev_mask |= SEVERITY_INFO;
    }
    else
    {
        sev_mask &= ~SEVERITY_INFO;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchDbg(bool check)
{
    SetFlagDebug(check);
    ResetData();
}

void LogModel::SetFlagDebug(bool check)
{
    if (check)
    {
        sev_mask |= SEVERITY_DEBUG;
    }
    else
    {
        sev_mask &= ~SEVERITY_DEBUG;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchMo1(bool check)
{
    SetFlagMo1(check);
    ResetData();
}

void LogModel::SetFlagMo1(bool check)
{
    if (check)
    {
        mon_mask |= MON_LINE1;
    }
    else
    {
        mon_mask &= ~MON_LINE1;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchMo2(bool check)
{
    SetFlagMo2(check);
    ResetData();
}

void LogModel::SetFlagMo2(bool check)
{
    if (check)
    {
        mon_mask |= MON_LINE2;
    }
    else
    {
        mon_mask &= ~MON_LINE2;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchMo3(bool check)
{
    SetFlagMo3(check);
    ResetData();
}

void LogModel::SetFlagMo3(bool check)
{
    if (check)
    {
        mon_mask |= MON_LINE3;
    }
    else
    {
        mon_mask &= ~MON_LINE3;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchMo4(bool check)
{
    SetFlagMo4(check);
    ResetData();
}

void LogModel::SetFlagMo4(bool check)
{
    if (check)
    {
        mon_mask |= MON_LINE4;
    }
    else
    {
        mon_mask &= ~MON_LINE4;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchMon(bool check)
{
    SetFlagMon(check);
    ResetData();
}

void LogModel::SetFlagMon(bool check)
{
    if (check)
    {
        mod_mask |= MON_MODULE;
    }
    else
    {
        mod_mask &= ~MON_MODULE;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchRpc(bool check)
{
    SetFlagRpc(check);
    ResetData();
}

void LogModel::SetFlagRpc(bool check)
{
    if (check)
    {
        mod_mask |= TEST_MODULE;
    }
    else
    {
        mod_mask &= ~TEST_MODULE;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchTm(bool check)
{
    SetFlagTm(check);
    ResetData();
}

void LogModel::SetFlagTm(bool check)
{
    if (check)
    {
        mod_mask |= RTM_MODULE;
    }
    else
    {
        mod_mask &= ~RTM_MODULE;
    }
}

//////////////////////////////////////////////////////////////////////////
void LogModel::switchZlg(bool check)
{
    SetFlagZlg(check);
    ResetData();
}

void LogModel::switchModule( int key )
{
    keyModule = key;
    ResetData();
}

void LogModel::tickTimer()
{
    int count = 50;
    while (!indxs.empty() && count--)
    {
        emit resetRow(indxs.front());
        indxs.pop_front();
    }
    indxs.clear();
}

int LogModel::findRow(const QString& txt, int indBegin)
{
    for (int idx = indBegin; idx < vdata.size(); ++idx)
    {
        std::string str(vdata[idx]->begin + BEGIN_TIMESTAMP_OFFSET, vdata[idx]->end);
        if (boost::algorithm::contains(str, txt))
            return idx;
    }
    return -1;
}

int LogModel::findRowPrev(const QString& txt, int indBegin)
{
    for (int idx = indBegin; idx >= 0; --idx)
    {
        std::string str(vdata[idx]->begin + BEGIN_TIMESTAMP_OFFSET, vdata[idx]->end);
        if (boost::algorithm::contains(str, txt))
            return idx;
    }
    return -1;
}

void LogModel::SetFlagZlg(bool check)
{
    if (check)
    {
        mod_mask |= ZLG_MODULE;
    }
    else
    {
        mod_mask &= ~ZLG_MODULE;
    }
}

