#ifndef CLOUPEWINDOW_H
#define CLOUPEWINDOW_H

#include <QtGui>
#include <QWidget>
#include "editdata.h"

class MainWindow ;

class CLoupeWindow : public QWidget
{
    Q_OBJECT
public:
	explicit CLoupeWindow(CEditData *pEditData, MainWindow *pMainWindow, QWidget *parent = 0);

signals:

public slots:
	void slot_cursorScreenShort() ;
	void slot_changeScale(QString str) ;

private:
	CEditData		*m_pEditData ;
	QLabel			*m_pLabel ;
	MainWindow		*m_pMainWindow ;

	QTimer			*m_pTimer ;
	int				m_Scale ;
};

#endif // CLOUPEWINDOW_H
