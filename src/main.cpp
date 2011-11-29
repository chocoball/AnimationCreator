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

	if ( qApp->arguments().size() > 1 ) {
		QString arg = qApp->arguments().at(1) ;
		arg = arg.replace("\\", "/") ;
		if ( !arg.isEmpty() ) {
			w.fileOpen(arg) ;
		}
	}

    return a.exec();
}
