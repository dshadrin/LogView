#include "logviewqt.h"
#include <QtWidgets/QApplication>
#include <QtCore/QSettings>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    int ret = -1;

    QApplication a( argc, argv );
    QString fname = argc > 1 ? argv[1] : "";
    boost::filesystem::path pt( argv[0] );
    boost::filesystem::path confPath( boost::filesystem::absolute( pt ).replace_extension( "conf" ) );
    QString cPt = QString::fromStdString( boost::filesystem::absolute( pt ).remove_filename().string() );
    confName = QString::fromStdString( confPath.string() );
    QSettings::setDefaultFormat( QSettings::IniFormat );
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, cPt );

    LogViewQt w( fname );
    w.show();

    try
    {
        ret = a.exec();
    }
    catch (const std::exception& e)
    {
        QMessageBox messageBox;
        messageBox.critical( 0, "Error", e.what() );
        messageBox.setFixedSize( 500, 200 );
    }
    return ret;
}
