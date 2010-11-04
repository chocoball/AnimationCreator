#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(Resource) ;

    QApplication a(argc, argv);

	QTranslator translator;
	translator.load(":/root/Resources/lang/linguist_ja.qm");
	qApp->installTranslator(&translator) ;

    MainWindow w;
    w.show();

    return a.exec();
}
