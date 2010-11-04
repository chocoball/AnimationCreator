#ifndef CLOUPEWINDOW_H
#define CLOUPEWINDOW_H

#include <QtGui>
#include <QWidget>
#include "editimagedata.h"

class CLoupeWindow : public QWidget
{
    Q_OBJECT
public:
	explicit CLoupeWindow(CEditImageData *pEditImageData, QWidget *parent = 0);

signals:

public slots:
	void slot_cursorScreenShort() ;
	void slot_changeScale(QString str) ;

private:
	CEditImageData	*m_pEditImageData ;
	QLabel			*m_pLabel ;

	QTimer			*m_pTimer ;
	int				m_Scale ;
};

#endif // CLOUPEWINDOW_H
