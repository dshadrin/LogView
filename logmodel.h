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
    EUartLog,
    EDmesgLog,
    ETestRunnerLog,
    ECommonText
};

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

const quint32 COLUMN_1_MASK = 0x01;
const quint32 COLUMN_2_MASK = 0x02;
const quint32 COLUMN_3_MASK = 0x04;
const quint32 COLUMN_4_MASK = 0x08;

const quint32 SEV_MASK = 0x000000FF;
const quint32 MON_MASK = 0x0000FF00;
const quint32 MOD_MASK = 0x00FF0000;

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
    quint32 colMask;
    QVector<DataValue> vstorage;
    QVector<DataValue*> vdata;
    mutable QQueue<QVector<DataValue>::size_type> indxs;
    QTimer *tmr;
    QFont font, fbold;
    ELogType logType;

private:
    void SetFlags(DataValue& dv);
    int RecalculateColumnIndex( int idx ) const;

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
    void filterEnable(quint32 mask);
    void setColumnEnable(quint32 mask);
};

#endif // LOGMODEL_H
