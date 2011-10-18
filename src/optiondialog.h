#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include "setting.h"
#include "ui_OptionAnimationTab.h"

namespace Ui {
	class OptionAnimationTab ;
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
	explicit AnimeWindowTab(CSettings *pSetting, QWidget *parent = 0) ;
	~AnimeWindowTab() ;

public slots:
	void slot_changeBGColor(QString) ;
	void slot_changeUseBackImage( bool ) ;
	void slot_openFileDialog( void ) ;

	void slot_changeScreenW(int val) ;
	void slot_changeScreenH(int val) ;

private:
	Ui::OptionAnimationTab	*ui ;
	CSettings				*m_pSetting ;
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

// オプションダイアログ
class OptionDialog : public QDialog
{
    Q_OBJECT
public:
	explicit OptionDialog(CSettings *pSetting, QWidget *parent = 0);

signals:

public slots:
};

#endif // OPTIONDIALOG_H
