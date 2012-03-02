#ifndef COMMAND_H
#define COMMAND_H

#include <QtGui>
#include <QUndoCommand>
#include <QStandardItemModel>
#include "editdata.h"

class CEditData ;

// アイテム追加 コマンド
class Command_AddItem : public QUndoCommand
{
public:
	Command_AddItem(CEditData *pEditData, QString &str, QModelIndex &parent) ;

	void redo();
	void undo();

	QModelIndex getIndex() { return m_index ; }

private:
	CEditData		*m_pEditData ;
	QString			m_str ;
	QModelIndex		m_index ;
	int				m_parentRow ;
	int				m_row ;
} ;

// アイテム削除 コマンド
class Command_DelItem : public QUndoCommand
{
public:
	Command_DelItem(CEditData *pEditData, QModelIndex &index) ;

	void redo();
	void undo();

private:
	CEditData		*m_pEditData ;
	int				m_row ;
	int				m_parentRow ;
	int				m_relRow ;
	ObjectItem		*m_pItem ;
} ;


// フレームデータ追加コマンド
class Command_AddFrameData : public QUndoCommand
{
public:
	Command_AddFrameData(CEditData *pEditData,
						 QModelIndex &index,
						 FrameData &data,
						 QList<QWidget *> &updateWidget) ;

	void redo() ;
	void undo() ;

private:
	CEditData			*m_pEditData ;
	CObjectModel		*m_pObjModel ;
	FrameData			m_frameData ;
	int					m_row ;
	int					m_flag ;

	QList<QWidget *>	m_UpdateWidgetList ;
};

// フレームデータ削除コマンド
class Command_DelFrameData : public QUndoCommand
{
public:
	Command_DelFrameData(CEditData *pEditData,
						 QModelIndex &index,
						 int frame,
						 QList<QWidget *> &updateWidget) ;
	void redo() ;
	void undo() ;

private:
	CEditData			*m_pEditData ;
	CObjectModel		*m_pObjModel ;
	FrameData			m_FrameData ;
	int					m_row ;

	QList<QWidget *>	m_UpdateWidgetList ;
};

// フレームデータ編集コマンド
class Command_EditFrameData : public QUndoCommand
{
public:
	Command_EditFrameData(CEditData			*pEditData,
						  QModelIndex		&index,
						  int				frame,
						  FrameData			&data,
						  FrameData			*pOld,
						  QList<QWidget *>	&updateWidget) ;
	void redo() ;
	void undo() ;

private:
	CEditData			*m_pEditData ;
	CObjectModel		*m_pObjModel ;
	FrameData			m_FrameData, m_OldFrameData ;
	int					m_row ;
	int					m_frame ;

	bool				m_bSetOld ;

	QList<QWidget *>	m_UpdateWidgetList ;
};

// オブジェクトコピーコマンド
class Command_CopyObject : public QUndoCommand
{
public:
	Command_CopyObject( CEditData *pEditData, QModelIndex &index, QList<QWidget *> &updateWidget ) ;

	void redo() ;
	void undo() ;

private:
	CEditData			*m_pEditData ;
	ObjectItem			*m_pObject ;
	int					m_row ;
	QList<QWidget *>	m_UpdateWidgetList ;
} ;

// レイヤコピー
class Command_CopyIndex : public QUndoCommand
{
public:
	Command_CopyIndex( CEditData *pEditData, int row, ObjectItem *pLayer, QModelIndex parent, QList<QWidget *> &updateWidget ) ;

	void redo() ;
	void undo() ;

private:
	CEditData			*m_pEditData ;
	int					m_row, m_relRow, m_parentRow ;
	ObjectItem			*m_pLayer ;
	QList<QWidget *>	m_UpdateWidgetList ;
} ;

// フレームデータ移動
class Command_MoveFrameData : public QUndoCommand
{
public:
	Command_MoveFrameData(CEditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame, QList<QWidget *> &updateWidget) ;

	void redo() ;
	void undo() ;

private:
	CEditData			*m_pEditData ;
	int					m_row ;
	QList<QWidget *>	m_UpdateWidgetList ;

	int					m_srcFrame, m_dstFrame ;
	FrameData			m_srcData, m_dstData ;
};

// 全フレームデータ移動
class Command_MoveAllFrameData : public QUndoCommand
{
public:
	Command_MoveAllFrameData(CEditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame, QList<QWidget *> &updateWidget) ;

	void redo() ;
	void undo() ;

private:
	void save_frameData(ObjectItem *pItem, int srcFrame, int dstFrame) ;
	void restore_frameData(ObjectItem *pItem, int srcFrame, int dstFrame) ;

private:
	CEditData						*m_pEditData ;
	int								m_row ;
	QList<QWidget *>				m_UpdateWidgetList ;

	int								m_srcFrame, m_dstFrame ;
	QList<QPair<int, FrameData> >	m_dstDatas ;
};

// ツリーアイテム上に移動
class Command_MoveItemUp : public QUndoCommand
{
public:
	Command_MoveItemUp(CEditData *pEditData, const QModelIndex &index) ;

	void redo() ;
	void undo() ;

private:
	CEditData		*m_pEditData ;
	QModelIndex		m_index ;
	ObjectItem		*m_pItem ;
};

// ツリーアイテム下に移動
class Command_MoveItemDown : public QUndoCommand
{
public:
	Command_MoveItemDown(CEditData *pEditData, const QModelIndex &index) ;

	void redo() ;
	void undo() ;

private:
	CEditData		*m_pEditData ;
	QModelIndex		m_index ;
	ObjectItem		*m_pItem ;
};

// UVスケール
class Command_ScaleUv : public QUndoCommand
{
public:
	Command_ScaleUv(CEditData *pEditData, double scale) ;

	void redo() ;
	void undo() ;

private:
	void save_framedata(ObjectItem *pItem) ;

private:
	CEditData						*m_pEditData ;
	QList<QPair<int, FrameData> >	m_changeFrameDatas ;
	double							m_scale ;
};

// フレームスケール
class Command_ScaleFrame : public QUndoCommand
{
public:
	Command_ScaleFrame(CEditData *pEditData, double scale) ;

	void redo() ;
	void undo() ;

private:
	void save_framedata(ObjectItem *pItem) ;

private:
	CEditData								*m_pEditData ;
	QList<QPair<int, QList<FrameData> > >	m_changeFrameDatas ;
	double									m_scale ;
	QList<QWidget *>	m_UpdateWidgetList ;
};

#endif // COMMAND_H
