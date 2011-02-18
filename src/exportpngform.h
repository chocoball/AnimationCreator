#ifndef EXPORTPNGFORM_H
#define EXPORTPNGFORM_H

#include <QWidget>
#include <QtGui>
#include "editdata.h"
#include "setting.h"

namespace Ui {
    class ExportPNGForm;
}

class ExportPNGForm : public QWidget
{
    Q_OBJECT

public:
	explicit ExportPNGForm(CEditData *pEditData, CSettings *pSetting, QWidget *parent = 0);
    ~ExportPNGForm();

signals:
	void sig_changeRect( void ) ;
	void sig_startExport( void ) ;
	void sig_cancel( void ) ;

public slots:
	void resizeEvent( QResizeEvent *event ) ;

	void slot_changeRect( void ) ;
	void slot_openSaveDir( void ) ;

	void slot_changeLeft( int val ) ;
	void slot_changeRight( int val ) ;
	void slot_changeTop( int val ) ;
	void slot_changeBottom( int val ) ;

	void slot_startExport( void ) ;
	void slot_changeSaveDir( void ) ;

	void slot_cancel( void ) ;

private:
	Ui::ExportPNGForm	*ui;
	CEditData			*m_pEditData ;
	CSettings			*m_pSetting ;
};

#endif // EXPORTPNGFORM_H
