
#include "logmodel.h"
#include <boost/filesystem.hpp>
#include <cstdio>

LogModel::LogModel(const QString& fname, QObject *parent)
    : file_name(fname)
    , buffer(nullptr)
{
    bool status = false;

    if (boost::filesystem::exists(fname.toStdString()))
    {
        buffer_size = boost::filesystem::file_size(fname.toStdString());
        buffer = new char[buffer_size];
        FILE* in = fopen(fname.toStdString().c_str(), "rt");
        if (in)
        {
            if (fread((void*)buffer, 1, buffer_size, in) == buffer_size)
            {
                fclose(in);

                // parse file
                if (buffer[0] == '[') // check for correct log
                {
                    DataValue dv = {nullptr, nullptr};
                    for (size_t pos = 0; pos < buffer_size; ++pos)
                    {

                    }

                    // set fonts
                    font.setFamily("Courier New");
                    font.setPointSize(12);
                    fbold = font;
                    fbold.setBold(true);
                    status = true;
                }
            }
        }
    }

    if(!status)
    {
        throw std::runtime_error("error file operation");
    }
}

LogModel::~LogModel()
{
    delete[] buffer;
}

int LogModel::rowCount( const QModelIndex & /*parent*/ ) const
{
    return vdata.size();
}

int LogModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return 4;
}

QVariant LogModel::data( const QModelIndex &index, int role ) const
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

QVariant LogModel::HandleDisplayRole(int col, int row) const
{
/*    DataValue* dv = vdata[row];
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
    }*/
    return QVariant();
}

QVariant LogModel::HandleBackgroundRole(int col, int row) const
{
/*    if ( col == 3 )
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
    }*/
    return QVariant();
}

QVariant LogModel::HandleTextColorRole(int col, int row) const
{
/*    if ( col == 2 )
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
    }*/
    return QVariant();
}

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
