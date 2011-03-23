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

	void toggleLock( void ) ;

signals:

public slots:
	void slot_cursorScreenShort() ;
	void slot_changeScale(QString str) ;

protected:
	void resizeEvent( QResizeEvent *event ) ;

	void fixImage( QSize &size ) ;

private:
	CEditData		*m_pEditData ;
	QLabel			*m_pLabel ;
	MainWindow		*m_pMainWindow ;
	QCheckBox		*m_pCheckBox_Cursor, *m_pCheckBox_Center ;

	QTimer			*m_pTimer ;
	int				m_Scale ;

	QPoint			m_CenterPos ;
};

#endif // CLOUPEWINDOW_H
