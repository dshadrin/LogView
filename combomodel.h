#pragma once
#include <QAbstractTableModel>
#include <QFont>


class ComboModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ComboModel( QObject *parent = nullptr );
    ~ComboModel();

    int rowCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    int columnCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;

    void addItem( QString text, quint32 key );

private:
    struct DataValue
    {
        QString text;
        quint32 key;
    };

    QVector<DataValue> vstorage;
};

