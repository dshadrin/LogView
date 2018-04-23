#include "logtablemodel.h"
#include <QtGui/QBrush>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QDebug>
#include <QtCore/QTextStream>
#include <QFontMetrics>
#include <QBuffer>
#include <QDebug>

LogTableModel::LogTableModel( QString fname, QObject *parent)
    : QAbstractTableModel(parent)
    , file_name(fname)
{
	font.setFamily("Courier New");
	font.setPointSize(12);
    fbold = font;
    fbold.setBold(true);

	flags.crit = true;
	flags.debug = true;
	flags.info = true;
	flags.monitor = true;
	flags.rpc = true;
	flags.test = true;
	flags.warn = true;
	flags.err = true;
    flags.tm = true;

    load( );
}

LogTableModel::~LogTableModel()
{

}

int LogTableModel::rowCount( const QModelIndex & /*parent*/ ) const
{
    return vdata.size();
}

int LogTableModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return 4;
}

QVariant LogTableModel::data( const QModelIndex &index, int role ) const
{
    int row = index.row( );
    int col = index.column( );

    switch ( role )
    {
    case Qt::DisplayRole:
        return HandleDisplayRole(col, row);

    case Qt::FontRole:
        return ( col == 2 ) ? fbold : font;

    case Qt::BackgroundRole:
        return HandleBackgroundRole(col, row);

    case Qt::TextColorRole:
        return HandleTextColorRole(col, row);

    case Qt::TextAlignmentRole:
        return Qt::AlignLeft + Qt::AlignVCenter;
	}

    return QVariant( );
}

QVariant LogTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
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

void LogTableModel::load( )
{
	QFile file(file_name);
	if (file.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
        QByteArray ba = file.readAll( );
        file.close( );
        QBuffer buf( &ba );
        buf.open( QIODevice::ReadOnly | QIODevice::Text );
        
        QTextStream ts( &buf );
		DataValue dv;
        QRegularExpression rx("^\\[\\d{4}\\-[A-Za-z]{3}\\-\\d{2}\\s(?<time>\\d{2}:\\d{2}:\\d{2}.\\d{6})\\]\\[(?<module>[A-Z\\s]{3})\\]\\[(?<severity>[A-Z\\s]{4})\\]\\s\\-\\s(?<msg>.*)$");

		while (!ts.atEnd())
		{
			QString str = ts.readLine();
            QRegularExpressionMatch match = rx.match( str );

			if ( str[0] == '[')
			{
                QRegularExpressionMatch match = rx.match( str );
                if ( match.hasMatch( ) )
                {
                    dv.cont = false;
                    dv.time = match.captured( "time" );
                    dv.module = match.captured( "module" );
                    dv.severity = match.captured( "severity" );
                    dv.msg = match.captured( "msg" );
                }
                else
                {
                    qDebug( ) << "Error log line: " << str;
                }
			}
			else
			{
				dv.cont = true;
				dv.msg = str;
			}

            if ( !dv.msg.isEmpty( ) )
            {
				vstorage.push_back( dv );
            }
		}
	}

    for ( auto it = vstorage.begin() ; it != vstorage.end(); ++it )
    {
        vdata.push_back( &(*it) );
    }
}

void LogTableModel::switchCrit(bool check)
{
	flags.crit = check;
	fillData();
}

void LogTableModel::switchErr(bool check)
{
	flags.err = check;
	fillData();
}

void LogTableModel::switchWarn(bool check)
{
	flags.warn = check;
	fillData();
}

void LogTableModel::switchTest(bool check)
{
	flags.test = check;
	fillData();
}

void LogTableModel::switchInfo(bool check)
{
	flags.info = check;
	fillData();
}

void LogTableModel::switchDbg(bool check)
{
	flags.debug = check;
	fillData();
}

void LogTableModel::switchMon(bool check)
{
	flags.monitor = check;
	fillData();
}

void LogTableModel::switchRpc(bool check)
{
	flags.rpc = check;
	fillData();
}

void LogTableModel::switchTm(bool check)
{
    flags.tm = check;
    fillData();
}

void LogTableModel::fillData()
{
	vdata.clear();

	for (auto it = vstorage.begin(); it != vstorage.end(); ++it)
	{
		bool ins = false;
		DataValue* dv = &(*it);
		if ((dv->severity == "CRIT" && flags.crit == true) ||
			(dv->severity == "ERR " && flags.err == true) ||
			(dv->severity == "WARN" && flags.warn == true) ||
			(dv->severity == "TEST" && flags.test == true) ||
			(dv->severity == "INFO" && flags.info == true) ||
			(dv->severity == "DBG " && flags.debug == true) )
		{
			ins = true;
		}

		if (ins == true)
		{
			if (dv->module == "MON")
			{
				if (flags.monitor == false)
					ins = false;
			}
            else if (dv->module == "TM ")
            {
                if (flags.tm == false)
                    ins = false;
            }
			else
			{
				if (flags.rpc == false)
					ins = false;
			}
		}

		if (ins == true)
		{
			vdata.push_back(dv);
		}
	}

    changeModel();
}

QVariant LogTableModel::HandleDisplayRole(int col, int row) const
{
    DataValue* dv = vdata[row];
    switch ( col )
    {
    case 0:
        return dv->cont == false ? dv->time : "";
    case 1:
        return dv->cont == false ? dv->module : "";
    case 2:
        return dv->cont == false ? dv->severity : "";
    case 3:
        return dv->msg;
    }
    return QVariant();
}

QVariant LogTableModel::HandleBackgroundRole(int col, int row) const
{
    if ( col == 3 )
    {
        QColor cl;

        QString sev = vdata[row]->severity;
        QString mod = vdata[row]->module;

        if ( sev == "WARN" )
            return QBrush( QColor(255, 191, 223) );
        else if ( sev == "TEST" )
            return QBrush( QColor(170, 255, 170) );
        else if ( sev == "INFO" )
            return QBrush( QColor(171, 185, 255) );
        else if ( sev == "ERR " || sev == "CRIT" )
            return QBrush( QColor(255, 150, 150) );
        else if ( mod == "MON" )
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

QVariant LogTableModel::HandleTextColorRole(int col, int row) const
{
    if ( col == 2 )
    {
        const QString& sev = vdata[row]->severity;
        if ( sev == "WARN" )
            return QColor(Qt::darkMagenta);
        else if ( sev == "TEST" )
            return QColor(Qt::darkGreen);
        else if ( sev == "INFO" )
            return QColor(Qt::blue);
        else if ( sev == "ERR " || sev == "CRIT" )
            return QColor(Qt::red);
    }
    return QVariant();
}

QVariant LogTableModel::HandleHeaderDisplayRole(Qt::Orientation orientation, int section) const
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

