
#include "logmodel.h"
#include <boost/filesystem.hpp>
#include <sstream>
#include <string>
#include <QColor>
#include <QBrush>
#include <boost/algorithm/string.hpp>

const quint32 FULL_HEADER_LEN = 40;
const quint32 LAST_HEADER_POS = 39;
const quint32 BEGIN_TIMESTAMP_OFFSET = 13;
const quint32 TIMESTAMP_LEN = 15;
const quint32 BEGIN_MODULE_NAME_OFFSET = 30;
const quint32 MODULE_NAME_LEN = 3;
const quint32 BEGIN_SEVERITY_OFFSET = 35;
const quint32 SEVERITY_LEN = 4;
const quint32 BEGIN_LOG_STRING_OFFSET = 43;

const quint32 SEVERITY_CRIT = 0x00000001;
const quint32 SEVERITY_ERR = 0x00000002;
const quint32 SEVERITY_WARN = 0x00000004;
const quint32 SEVERITY_INFO = 0x00000008;
const quint32 SEVERITY_TEST = 0x00000010;
const quint32 SEVERITY_DEBUG = 0x00000020;
const quint32 MON_LINE1 = 0x00000100;
const quint32 MON_LINE2 = 0x00000200;
const quint32 MON_LINE3 = 0x00000400;
const quint32 MON_LINE4 = 0x00000800;
const quint32 MON_MODULE = 0x00010000;
const quint32 RTM_MODULE = 0x00020000;
const quint32 ZLG_MODULE = 0x00040000;
const quint32 TEST_MODULE = 0x00080000;

//////////////////////////////////////////////////////////////////////////
LogModel::LogModel(const QString& fname, QObject *parent)
    : file_name(fname)
    , mod_mask(0x00FF0000)
    , mon_mask(0x0000FF00)
    , sev_mask(0x000000FF)
    , tmr(Q_NULLPTR)
{
    bool status = false;
    std::stringstream ss;
    ss << std::endl;
    std::string endOfStr = ss.str();
    size_t endOfStrLen = endOfStr.length();

    if (boost::filesystem::exists(fname.toStdString()))
    {
        buffer_size = boost::filesystem::file_size(fname.toStdString());
        file.open(fname.toStdString(), buffer_size);
        if (file.is_open())
        {
            const char * buffer = file.data();
            const char * const buffer_end = buffer + buffer_size;

            // parse file
            if (buffer[0] == '[') // check for correct log
            {
                DataValue dv = {nullptr, nullptr};
                for (size_t pos = 0; pos < buffer_size; ++pos)
                {
                    if ((buffer[pos] == '[') &&
                        (buffer + pos + BEGIN_LOG_STRING_OFFSET) <= buffer_end &&
                        (buffer[pos + LAST_HEADER_POS] == ']'))
                    {
                        dv.begin = buffer + pos;
                        pos += BEGIN_LOG_STRING_OFFSET;
                        dv.end = buffer + pos;
                        SetFlags(dv);
                    }
                    else if (buffer[pos] != '\r' && buffer[pos] != '\n')
                    {
                        ++dv.end;
                    }
                    else
                    {
                        while (pos < buffer_size && (buffer[pos] == '\r' || buffer[pos] == '\n')) ++pos;
                        if (((buffer[pos] == '[') &&
                             (buffer + pos + BEGIN_LOG_STRING_OFFSET) <= buffer_end &&
                             (buffer[pos + LAST_HEADER_POS] == ']')))
                        {
                            ++dv.end;
                            vstorage.push_back(dv);
                            --pos;
                        }
                        else if (pos >= buffer_size)
                        {
                            ++dv.end;
                            vstorage.push_back(dv);
                        }
                        else
                        {
                            dv.end = buffer + pos;
                        }
                    }
                }

                // set fonts
                font.setFamily("Courier New");
                font.setPointSize(10);
                fbold = font;
                fbold.setBold(true);
                status = true;
                tmr = new QTimer(this);
                tmr->setInterval(100);
                connect(tmr, SIGNAL(timeout()), this, SLOT(tickTimer()));
            }
        }
    }

    if(!status)
    {
        throw std::runtime_error("error file operation");
    }
}

//////////////////////////////////////////////////////////////////////////
LogModel::~LogModel()
{
    file.close();
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
    switch ( role )
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return HandleDisplayRole(index.column(), index.row());

    case Qt::FontRole:
        return (index.column() == 2 ) ? fbold : font;

    case Qt::BackgroundRole:
        return HandleBackgroundRole(index.column(), index.row());

    case Qt::TextColorRole:
        return HandleTextColorRole(index.column(), index.row());

    case Qt::TextAlignmentRole:
        return Qt::AlignLeft + Qt::AlignVCenter;
    }

    return QVariant( );
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    switch ( role )
    {
    case Qt::DisplayRole:
        return HandleHeaderDisplayRole(orientation, section);

    case Qt::TextAlignmentRole:
        return Qt::AlignLeft + Qt::AlignVCenter;

    case Qt::FontRole:
        return fbold;
    }

    return QVariant( );
}

//////////////////////////////////////////////////////////////////////////
QVariant LogModel::HandleDisplayRole(int col, int row) const
{
    const DataValue * const dv = vdata[row];
    switch ( col )
    {
    case 0:
        return QString::fromStdString(std::string(dv->begin + BEGIN_TIMESTAMP_OFFSET, TIMESTAMP_LEN));
    case 1:
        return QString::fromStdString(std::string(dv->begin + BEGIN_MODULE_NAME_OFFSET, MODULE_NAME_LEN));
    case 2:
        return QString::fromStdString(std::string(dv->begin + BEGIN_SEVERITY_OFFSET, SEVERITY_LEN));
    case 3:
        return QString::fromStdString(std::string(dv->begin + BEGIN_LOG_STRING_OFFSET, dv->end));
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
            return QString("TimeStamp");
        case 1:
            return QString("Module");
        case 2:
            return QString("Severity");
        case 3:
            return QString("Data");
        }
    }
    return QVariant();
}

//////////////////////////////////////////////////////////////////////////
void LogModel::ResetData()
{
    tmr->stop();
    beginResetModel();
    vdata.clear();
    indxs.clear();
    for (DataValue& dv : vstorage)
    {
        if (dv.flags & mod_mask)
        {
            if (dv.flags & sev_mask)
            {
                if (dv.flags & MON_MODULE)
                {
                    if (dv.flags & mon_mask)
                    {
                        vdata.push_back(&dv);
                    }
                }
                else
                {
                    vdata.push_back(&dv);
                }
            }
        }
    }
    endResetModel();
    tmr->start();
    changeModel();
}

//////////////////////////////////////////////////////////////////////////
Qt::ItemFlags LogModel::flags(const QModelIndex & index) const
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

    const char * const sevPtr = dv.begin + BEGIN_SEVERITY_OFFSET;
    if (memcmp(sevPtr, "CRIT", SEVERITY_LEN) == 0)
    {
        dv.flags |= SEVERITY_CRIT;
    }
    else if (memcmp(sevPtr, "ERR ", SEVERITY_LEN) == 0)
    {
        dv.flags |= SEVERITY_ERR;
    }
    else if (memcmp(sevPtr, "WARN", SEVERITY_LEN) == 0)
    {
        dv.flags |= SEVERITY_WARN;
    }
    else if (memcmp(sevPtr, "INFO", SEVERITY_LEN) == 0)
    {
        dv.flags |= SEVERITY_INFO;
    }
    else if (memcmp(sevPtr, "TEST", SEVERITY_LEN) == 0)
    {
        dv.flags |= SEVERITY_TEST;
    }
    else if (memcmp(sevPtr, "DBG ", SEVERITY_LEN) == 0)
    {
        dv.flags |= SEVERITY_DEBUG;
    }

    const char * const modPtr = dv.begin + BEGIN_MODULE_NAME_OFFSET;
    bool monFlag = false;
    if (*modPtr == 'M')
    {
        if (*(modPtr + 1) == 'O')
        {
            if (*(modPtr + 2) == 'N')
            {
                dv.flags |= MON_LINE1;
                monFlag = true;
            }
            else if (*(modPtr + 2) == '2')
            {
                dv.flags |= MON_LINE2;
                monFlag = true;
            }
            else if (*(modPtr + 2) == '3')
            {
                dv.flags |= MON_LINE3;
                monFlag = true;
            }
            else if (*(modPtr + 2) == '4')
            {
                dv.flags |= MON_LINE4;
                monFlag = true;
            }

            if (monFlag)
            {
                dv.flags |= MON_MODULE;
            }
        }
    }
    else if (memcmp(modPtr, "TM ", MODULE_NAME_LEN) == 0)
    {
        dv.flags |= RTM_MODULE;
    }
    else if (memcmp(modPtr, "ZLG", MODULE_NAME_LEN) == 0)
    {
        dv.flags |= ZLG_MODULE;
    }
    else
    {
        dv.flags |= TEST_MODULE;
    }
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

void LogModel::tickTimer()
{
    int count = 50;
    while (!indxs.empty() && count--)
    {
        resetRow(indxs.front());
        indxs.pop_front();
    }
    indxs.clear();
}

int LogModel::findRow(const QString& txt, int indBegin)
{
    for (int idx = indBegin; idx < vdata.size(); ++idx)
    {
        std::string str(vdata[idx]->begin + BEGIN_LOG_STRING_OFFSET, vdata[idx]->end);
        if (boost::algorithm::contains(str, txt))
            return idx;
    }
    return -1;
}

int LogModel::findRowPrev(const QString& txt, int indBegin)
{
    for (int idx = indBegin; idx >= 0; --idx)
    {
        std::string str(vdata[idx]->begin + BEGIN_LOG_STRING_OFFSET, vdata[idx]->end);
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

