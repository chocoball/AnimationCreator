#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QtGui>
#include <QScrollArea>
#include "setting.h"
#include "editimagedata.h"

class CGridLabel ;
class AnimationForm ;

class ImageWindow : public QWidget
{
	Q_OBJECT

public:
	explicit ImageWindow(CSettings *p, CEditImageData *pEditImage, AnimationForm *pAnimForm, QWidget *parent = 0);
	~ImageWindow() ;

	void setAnimationForm( AnimationForm *p )
	{
		m_pAnimationForm = p ;
	}

protected:
	void dragEnterEvent(QDragEnterEvent *event) ;
	void dropEvent(QDropEvent *event);

	void addTab(int imageIndex) ;

	void contextMenuEvent(QContextMenuEvent *event) ;

signals:
	void sig_addImage(int imageNo) ;
	void sig_delImage(int imageNo) ;

public slots:
	void slot_delImage( void ) ;
	void slot_modifiedImage( int index ) ;

private:
	CSettings		*m_pSetting ;
	CEditImageData	*m_pEditImageData ;

	QCheckBox		*m_pCheckBox ;

	QTabWidget		*m_pTabWidget ;
	AnimationForm	*m_pAnimationForm ;

	QAction			*m_pActDelImage ;
};

#endif // IMAGEWINDOW_H
