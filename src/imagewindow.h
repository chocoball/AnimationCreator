#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QtGui>
#include <QScrollArea>
#include "setting.h"
#include "editimagedata.h"
#include "ui_imagewindow.h"

class CGridLabel ;
class AnimationForm ;
class MainWindow ;

namespace Ui {
	class ImageWindow ;
}

class ImageWindow : public QWidget
{
	Q_OBJECT

public:
	explicit ImageWindow(CSettings *p, CEditImageData *pEditImage, AnimationForm *pAnimForm, MainWindow *pMainWindow, QWidget *parent = 0);
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

	void updateGridLabel( void ) ;

signals:
	void sig_addImage(int imageNo) ;
	void sig_delImage(int imageNo) ;

public slots:
	void slot_delImage( void ) ;
	void slot_modifiedImage( int index ) ;

	void slot_changeUVBottom( int val ) ;
	void slot_changeUVTop( int val ) ;
	void slot_changeUVLeft( int val ) ;
	void slot_changeUVRight( int val ) ;
	void slot_setUI( QRect rect ) ;

	void slot_endedOption( void ) ;

private:
	Ui::ImageWindow	*ui ;

	CSettings		*m_pSetting ;
	CEditImageData	*m_pEditImageData ;

	AnimationForm	*m_pAnimationForm ;

	QAction			*m_pActDelImage ;

	MainWindow		*m_pMainWindow ;
};

#endif // IMAGEWINDOW_H
