#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QtGui>
#include <QScrollArea>
#include "setting.h"
#include "editimagedata.h"

class CGridLabel ;

class ImageWindow : public QWidget
{
	Q_OBJECT

public:
	explicit ImageWindow(CSettings *p, CEditImageData *pEditImage, QWidget *parent = 0);
	~ImageWindow() ;

	CGridLabel	*getGridLabel() { return m_pGridLabel ; }

signals:

public slots:

protected:
//	void paintEvent( QPaintEvent *event ) ;

private:

private:
	CSettings		*m_pSetting ;
	CEditImageData	*m_pEditImageData ;
	QScrollArea		*m_pScrollArea ;
	QLabel			*m_pImageLabel ;
	CGridLabel		*m_pGridLabel ;

	int				m_Scale ;
};

#endif // IMAGEWINDOW_H
