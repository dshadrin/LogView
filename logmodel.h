#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractTableModel>
#include <QFont>

class LogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    LogModel( const QString& fname, QObject *parent );
    ~LogModel();

    int rowCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    int columnCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;

private:
    QVariant HandleTextColorRole(int col, int row) const;
    QVariant HandleBackgroundRole(int col, int row) const;
    QVariant HandleDisplayRole(int col, int row) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

    QVariant HandleHeaderDisplayRole(Qt::Orientation orientation, int section) const;

private:
    const QString file_name;

    struct DataValue
    {
        const char* begin;
        const char* end;
    };

    const char* buffer;
    size_t buffer_size;
    QVector<DataValue> vstorage;
    QVector<DataValue*> vdata;
    QFont font, fbold;
};

#endif // LOGMODEL_H
