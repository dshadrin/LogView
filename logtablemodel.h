#ifndef LOGTABLEMODEL_H
#define LOGTABLEMODEL_H

#include <QAbstractTableModel>
#include <QtGui/QFont>

class LogTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    LogTableModel( QString fname, QObject *parent );
    ~LogTableModel();

    int rowCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    int columnCount( const QModelIndex & /*parent*/ ) const Q_DECL_OVERRIDE;
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;

private:
    QVariant HandleTextColorRole(int col, int row) const;
    QVariant HandleBackgroundRole(int col, int row) const;
    QVariant HandleDisplayRole(int col, int row) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

    QVariant HandleHeaderDisplayRole(Qt::Orientation orientation, int section) const;

    void load();

public slots:
	void switchCrit(bool check);
	void switchErr(bool check);
	void switchWarn(bool check);
	void switchTest(bool check);
	void switchInfo(bool check);
	void switchDbg(bool check);
	void switchMon(bool check);
	void switchRpc(bool check);
    void switchTm(bool check);

signals:
	void changeModel();

private:
	void fillData();

private:
    QString file_name;

    struct DataValue
    {
        QString time;
        QString module;
        QString severity;
        QString msg;
		bool cont;
    };

	QVector<DataValue*> vdata;
	QVector<DataValue> vstorage;
	QFont font, fbold;

	struct Flags
	{
		bool crit;
		bool err;
		bool warn;
		bool test;
		bool info;
		bool debug;
		bool monitor;
		bool rpc;
        bool tm;
	};

	Flags flags;
};

#endif // LOGTABLEMODEL_H
