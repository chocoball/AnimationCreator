#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include "setting.h"
#include "ui_OptionAnimationTab.h"
#include "ui_KeyboardTab.h"
#include "keyboardmodel.h"
#include "glwidget.h"

namespace Ui {
	class OptionAnimationTab ;
	class KeyboardTab ;
}

// ファイル タブ
class FileTab : public QWidget
{
	Q_OBJECT
public:
	explicit FileTab(CSettings *pSetting, QWidget *parent = 0) ;

public slots:
	void slot_clickedSaveImage(bool) ;
	void slot_clickedFlat(bool) ;
	void slot_clickedHierarchy(bool) ;

private:
	CSettings		*m_pSetting ;
};

// アニメーションウィンドウ タブ
class AnimeWindowTab : public QWidget
{
	Q_OBJECT
public:
	explicit AnimeWindowTab(CSettings *pSetting, AnimeGLWidget *pGlWidget, QWidget *parent = 0) ;
	~AnimeWindowTab() ;

public slots:
	void slot_changeBGColor(QString) ;
	void slot_changeUseBackImage( bool ) ;
	void slot_openFileDialog( void ) ;

	void slot_changeScreenW(int val) ;
	void slot_changeScreenH(int val) ;

	void slot_changeWindowW(int val) ;
	void slot_changeWindowH(int val) ;

	void slot_changeUseDepthTest(bool flag) ;
	void slot_changeUseZSort(bool flag) ;

private:
	Ui::OptionAnimationTab	*ui ;
	CSettings				*m_pSetting ;
	AnimeGLWidget			*m_pGlWidget ;
};

// イメージウィンドウ タブ
class ImageWindowTab : public QWidget
{
	Q_OBJECT
public:
	explicit ImageWindowTab(CSettings *pSetting, QWidget *parent = 0) ;

public slots:
	void slot_changeBGColor(QString) ;

private:
	CSettings		*m_pSetting ;
};

// キーボード設定タブ
class KeyboardTab : public QWidget
{
	Q_OBJECT
public:
	explicit KeyboardTab(CSettings *pSetting, QWidget *parent = 0) ;
	~KeyboardTab() ;

public slots:
	void slot_treeClicked(QModelIndex index) ;
	void slot_pushDelButton() ;

protected:
	void handleKeyEvent(QKeyEvent *event) ;
	bool eventFilter(QObject *o, QEvent *e) ;

	void setShortcut(int type, QKeySequence ks) ;

private:
	QList<QStringList> getData() ;

private:
	Ui::KeyboardTab	*ui ;
	CSettings		*m_pSetting ;
	KeyboardModel	*m_pKeyModel ;
	QModelIndex		m_selIndex ;

	bool			m_bShift, m_bCtrl, m_bAlt ;
} ;

// オプションダイアログ
class OptionDialog : public QDialog
{
    Q_OBJECT
public:
	explicit OptionDialog(CSettings *pSetting, AnimeGLWidget *pGlWidget, QWidget *parent = 0);

signals:

public slots:
};

#endif // OPTIONDIALOG_H
