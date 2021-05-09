#define _SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING

#include "combomodel.h"
//////////////////////////////////////////////////////////////////////////
ComboModel::ComboModel(QObject* parent) :
    QAbstractTableModel(parent)
{
    addItem( "", 0 );
}

//////////////////////////////////////////////////////////////////////////
ComboModel::~ComboModel()
{

}

//////////////////////////////////////////////////////////////////////////
int ComboModel::rowCount( const QModelIndex & /*parent*/ ) const
{
    return vstorage.size();
}

//////////////////////////////////////////////////////////////////////////
int ComboModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return 1;
}

//////////////////////////////////////////////////////////////////////////
QVariant ComboModel::data( const QModelIndex &index, int role ) const
{
    switch ( role )
    {
    case Qt::DisplayRole:
        return vstorage[index.row()].text;

//     case Qt::FontRole:
//         return (colIdx == 2 || colIdx == 0 ) ? fbold : font;

    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    return QVariant( );
}

void ComboModel::addItem( QString text, quint32 key )
{
    vstorage.append( { text, key } );
}
