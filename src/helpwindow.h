#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QWidget>
#include <QtGui>
#include "helpbrowser.h"

class HelpWindow : public QWidget
{
	Q_OBJECT
public:
	explicit HelpWindow(QWidget *parent = 0);

	bool isLoaded() { return (m_pEngine) ? true : false ; }

private:
	QHelpEngine		*m_pEngine ;
	HelpBrowser		*m_pBrowser ;
};


#endif // HELPWINDOW_H
