#include "logviewqt.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString fname = argc > 1 ? argv[1] : "";
	LogViewQt w(fname);
    w.show();
    return a.exec();
}
