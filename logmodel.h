#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include <boost/iostreams/device/mapped_file.hpp>
#include <QQueue>
#include <QTimer>

enum class ELogType : uint8_t
{
    EEtfRpcLog,

    ECommonText
};

class LogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    LogModel( const QString& fname, QObject *parent );

    void SetLogFont();

    ~LogModel();

    int rowCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    int columnCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
    void ResetData();
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    void SetFlagCrit(bool check);
    void SetFlagError(bool check);
    void SetFlagWarn(bool check);
    void SetFlagTest(bool check);
    void SetFlagInfo(bool check);
    void SetFlagDebug(bool check);
    void SetFlagMo1(bool check);
    void SetFlagMo2(bool check);
    void SetFlagMo3(bool check);
    void SetFlagMo4(bool check);
    void SetFlagMon(bool check);
    void SetFlagRpc(bool check);
    void SetFlagTm(bool check);
    void SetFlagZlg(bool check);

private:
    QVariant HandleTextColorRole(int col, int row) const;
    QVariant HandleBackgroundRole(int col, int row) const;
    QVariant HandleDisplayRole(int col, int row) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant HandleHeaderDisplayRole(Qt::Orientation orientation, int section) const;
    ELogType AnaliseFileFormat(const std::string& fname, size_t fSize);

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
    mutable QQueue<QVector<DataValue>::size_type> indxs;
    QTimer *tmr;
    QFont font, fbold;

private:
    void SetFlags(DataValue& dv);

private slots:
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

    void tickTimer();
    int findRow(const QString& txt, int indBegin);
    int findRowPrev(const QString& txt, int indBegin);

signals:
    void changeModel();
    void resetRow(int row) const;
};

#endif // LOGMODEL_H
