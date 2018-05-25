#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include <boost/iostreams/device/mapped_file.hpp>

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

    void ResetData();

private:
    const QString file_name;

    struct DataValue
    {
        const char* begin;
        const char* end;
        quint32 flags;
    };

    boost::iostreams::mapped_file_source file;
    size_t buffer_size;
    quint32 mod_mask;
    quint32 mon_mask;
    quint32 sev_mask;
    QVector<DataValue> vstorage;
    QVector<DataValue*> vdata;
    QFont font, fbold;

private:
    void SetFlags(DataValue& dv);

public slots:
    void switchCrit(bool check);
    void switchErr(bool check);
    void switchWarn(bool check);
    void switchTest(bool check);
    void switchInfo(bool check);
    void switchDbg(bool check);

    void switchMo1(bool check);
    void switchMo2(bool check);
    void switchMo3(bool check);
    void switchMo4(bool check);

    void switchMon(bool check);
    void switchRpc(bool check);
    void switchTm(bool check);
    void switchZlg(bool check);
};

#endif // LOGMODEL_H
