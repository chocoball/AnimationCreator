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
	QModelIndex		m_parent ;
	QModelIndex		m_index ;
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
	QString			m_str ;
	QModelIndex		m_index ;
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
	QModelIndex			m_index ;
	FrameData			m_FrameData ;

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
	int					m_Index ;

	QList<QWidget *>	m_UpdateWidgetList ;
};

// フレームデータ編集コマンド
class Command_EditFrameData : public QUndoCommand
{
public:
	Command_EditFrameData(CEditData			*pEditData,
						  QModelIndex		&index,
						  int				frame,
						  QList<FrameData>	&datas,
						  QList<QWidget *>	&updateWidget) ;
	void redo() ;
	void undo() ;

private:
	CEditData			*m_pEditData ;
	CObjectModel		*m_pObjModel ;
	QModelIndex			m_index ;
	QList<FrameData>	m_FrameData, m_OldFrameData ;
	int					m_Frame ;

	QList<QWidget *>	m_UpdateWidgetList ;
};

// オブジェクトコピーコマンド
class Command_CopyObject : public QUndoCommand
{
public:
	Command_CopyObject( CEditData *pEditData, CObjectModel::typeID objID, QList<QWidget *> &updateWidget ) ;

	void redo() ;
	void undo() ;

private:
	CEditData						*m_pEditData ;
	CObjectModel::typeID			m_objID ;
	QList<QWidget *>				m_UpdateWidgetList ;

	CObjectModel::ObjectGroup		m_objGroup ;
} ;


#endif // COMMAND_H
