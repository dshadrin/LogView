#include "logviewqt.h"
#include <QtWidgets/QApplication>
#include <QtCore/QSettings>

int main(int argc, char *argv[])
{
    boost::filesystem::path pt(argv[0]);
    boost::filesystem::path confPath(boost::filesystem::absolute(pt).replace_extension("conf"));
    QString cPt = QString::fromStdString(boost::filesystem::absolute(pt).remove_filename().string());
    confName = QString::fromStdString(confPath.string());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, cPt);
    QApplication a(argc, argv);
	QString fname = argc > 1 ? argv[1] : "";
	LogViewQt w(fname);
    w.show();
    return a.exec();
}
