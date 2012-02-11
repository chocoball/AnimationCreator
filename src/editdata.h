#ifndef EDITDATA_H
#define EDITDATA_H

#include <QImage>
#include <QtOpenGL>
#include <QUndoStack>
#include "objectmodel.h"
#include "CRect.h"

class CEditData
{
public:
	typedef struct {
		int				nNo ;
		QImage			Image ;
		int				origImageW, origImageH ;
		GLuint			nTexObj ;
		QString			fileName ;
		QDateTime		lastModified ;	// fileNameの最終更新時間
	} ImageData ;

	enum {
		kMaxFrame	= 1024		// 最大アニメーションフレーム数
	} ;

	// edit mode
	enum {
		kEditMode_Animation = 0,
		kEditMode_ExportPNG,

		kEditMode_Num
	} ;

public:
	CEditData() ;
	~CEditData() ;

	void setImageData( QList<ImageData> &data ) { m_ImageDataList = data ; }
	void addImageData(ImageData &data) { m_ImageDataList.append(data) ; }

	int getImageDataListSize( void ) { return m_ImageDataList.size() ; }

	ImageData *getImageData(int index)
	{
		if ( index < 0 || index >= m_ImageDataList.size() ) {
			return NULL ;
		}
		return &m_ImageDataList[index] ;
	}
	ImageData *getImageDataFromNo(int no)
	{
		for ( int i = 0 ; i < m_ImageDataList.size() ; i ++ ) {
			if ( m_ImageDataList[i].nNo == no ) {
				return &m_ImageDataList[i] ;
			}
		}
		return NULL ;
	}
	void removeImageDataByNo(int no)
	{
		for ( int i = 0 ; i < m_ImageDataList.size() ; i ++ ) {
			if ( m_ImageDataList[i].nNo != no ) { continue ; }

			if ( m_ImageDataList[i].nTexObj ) {
				glDeleteTextures(1, &m_ImageDataList[i].nTexObj) ;
			}
			m_ImageDataList.removeAt(i) ;
			return ;
		}
	}

	void sortImageDatas()
	{
		for ( int i = 0 ; i < m_ImageDataList.size() ; i ++ ) {
			for ( int j = 0 ; j < i ; j ++ ) {
				if ( m_ImageDataList.at(i).nNo < m_ImageDataList.at(j).nNo ) {
					m_ImageDataList.swap(i, j) ;
				}
			}
		}
	}

	CObjectModel		*getObjectModel( void )	{ return m_pObjectModel ; }
	QUndoStack			*getUndoStack( void )	{ return m_pUndoStack ; }

	void	resetData( void ) ;
	void	initData( void ) ;

	QModelIndex cmd_addItem(QString str, QModelIndex parent = QModelIndex()) ;
	void cmd_delItem(QModelIndex &index) ;

	void cmd_addFrameData( QModelIndex &index, FrameData &data, QList<QWidget *> &updateWidget ) ;
	void cmd_delFrameData( QModelIndex			&index,
						   int					frame,
						   QList<QWidget *>		&updateWidget ) ;
	void cmd_editFrameData( QModelIndex			index,
							int					frame,
							FrameData			&data,
							FrameData			*pOld,
							QList<QWidget *>	&updateWidget ) ;
	void cmd_copyObject(QModelIndex &index, QList<QWidget *> &updateWidget) ;
	void cmd_copyIndex(int row, ObjectItem *pItem, QModelIndex parent, QList<QWidget *> &updateWidget) ;
	void cmd_moveFrameData(QModelIndex &index, int prevFrame, int nextFrame, QList<QWidget *> &updateWidget) ;
	void cmd_moveItemUp(const QModelIndex &index) ;
	void cmd_moveItemDown(const QModelIndex &index) ;

	void setCurrLoopNum( int num )
	{
		ObjectItem *p = m_pObjectModel->getObject(getSelIndex()) ;
		if ( !p ) { return ; }
		p->setCurrLoop(num) ;
	}
	bool addCurrLoopNum( int num )
	{
		ObjectItem *p = m_pObjectModel->getObject(getSelIndex()) ;
		if ( !p ) { return true ; }
		p->setCurrLoop(p->getCurrLoop() + num) ;
		if ( p->getLoop() < 0 ) { return false ; }	// 無限ループ
		return p->getCurrLoop() > p->getLoop() ? true : false ;
	}


	// 連番PNG保存関連 ----------------------
	void startExportPNG( QString dir )
	{
		m_nExportEndFrame = 0 ;
		m_strExportPNGDir = dir ;
		m_bExportPNG = true ;
	}

	void endExportPNG( void )
	{
		m_bExportPNG = false ;
	}

	QString getExportPNGDir( void )
	{
		return m_strExportPNGDir ;
	}

	bool isExportPNG( void )
	{
		return m_bExportPNG ;
	}

	void getExportPNGRect( int ret[4] )
	{
		ret[0] = m_exPngRect[0] ;
		ret[1] = m_exPngRect[1] ;
		ret[2] = m_exPngRect[2] ;
		ret[3] = m_exPngRect[3] ;
	}

	void setExportPNGRect( int rect[4] )
	{
		m_exPngRect[0] = rect[0] ;
		m_exPngRect[1] = rect[1] ;
		m_exPngRect[2] = rect[2] ;
		m_exPngRect[3] = rect[3] ;
	}

	int getExportEndFrame( void )
	{
		return m_nExportEndFrame ;
	}
	void setExportEndFrame( int frame )
	{
		m_nExportEndFrame = frame ;
	}
	// --------------------------------------

	// フレームデータ コピー関連 ------------
	void setCopyFrameData( FrameData data )
	{
		m_CopyFrameData = data ;
		m_bCopyFrameData = true ;
	}
	FrameData getCopyFrameData( void )
	{
		return m_CopyFrameData ;
	}
	bool isCopyData( void )
	{
		return m_bCopyFrameData ;
	}
	// --------------------------------------

	// レイヤコピー関連 ----------------------
	void setCopyLayer( ObjectItem *p )
	{
		if ( m_pCopyLayer ) {
			delete m_pCopyLayer ;
		}
		m_pCopyLayer = new ObjectItem("copy", NULL) ;
		m_pCopyLayer->copy(p) ;
		m_bCopyLayer = true ;
	}
	ObjectItem *getCopyLayer( void )
	{
		return m_pCopyLayer ;
	}
	bool isCopyLayer( void )
	{
		return m_bCopyLayer ;
	}
	// --------------------------------------

	bool getNowSelectFrameData(FrameData &ret) ;
	QMatrix4x4 getNowSelectMatrix() ;

	void sortFrameDatas( void ) ;

	QModelIndex getSelIndex()
	{
		if ( !m_pTreeViewRef ) { return QModelIndex() ; }
		return m_pTreeViewRef->currentIndex() ;
	}

private:
	void sortFrameDatas(ObjectItem *pItem) ;

	kAccessor(int, m_editMode, EditMode)
	kAccessor(CRectF, m_catchRect, CatchRect)
	kAccessor(QPoint, m_center, Center)
	kAccessor(QTreeView*, m_pTreeViewRef, TreeView)
	kAccessor(bool, m_bPlayAnime, PlayAnime)
	kAccessor(bool, m_bPauseAnime, PauseAnime)
	kAccessor(bool, m_bDraggingImage, DraggingImage)
	kAccessor(int, m_selectFrame, SelectFrame)

private:
	QList<ImageData>				m_ImageDataList ;

	CObjectModel					*m_pObjectModel ;
	QUndoStack						*m_pUndoStack ;

	bool							m_bExportPNG ;
	QString							m_strExportPNGDir ;
	int								m_exPngRect[4] ;	// [0]left, [1]top, [2]right, [3]bottom
	int								m_nExportEndFrame ;	// 吐き出し終わったフレーム

	bool							m_bCopyFrameData ;
	FrameData						m_CopyFrameData ;

	bool							m_bCopyLayer ;
	ObjectItem						*m_pCopyLayer ;
} ;

#endif // EDITDATA_H
