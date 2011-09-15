#include "exportpngform.h"
#include "ui_exportpngform.h"

ExportPNGForm::ExportPNGForm(CEditData *pEditData, CSettings *pSetting, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExportPNGForm)
{
	m_pEditData = pEditData ;
	m_pSetting = pSetting ;

	m_pEditData->setEditMode(CEditData::kEditMode_ExportPNG);
    ui->setupUi(this);

	setWindowTitle(trUtf8("連番PNG 保存"));
	ui->textEdit->setText(m_pSetting->getSavePngDir()) ;

	connect(ui->pushButton_sel_dir, SIGNAL(clicked()), this, SLOT(slot_openSaveDir())) ;
	connect(ui->spinBox_left, SIGNAL(valueChanged(int)), this, SLOT(slot_changeLeft(int))) ;
	connect(ui->spinBox_right, SIGNAL(valueChanged(int)), this, SLOT(slot_changeRight(int))) ;
	connect(ui->spinBox_top, SIGNAL(valueChanged(int)), this, SLOT(slot_changeTop(int))) ;
	connect(ui->spinBox_bottom, SIGNAL(valueChanged(int)), this, SLOT(slot_changeBottom(int))) ;
	connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(slot_startExport())) ;
	connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(slot_changeSaveDir())) ;
	connect(ui->pushButton_cancel, SIGNAL(clicked()), this, SLOT(slot_cancel())) ;

	slot_changeRect() ;
}

ExportPNGForm::~ExportPNGForm()
{
	m_pEditData->setEditMode(CEditData::kEditMode_Animation);
	delete ui;
}

void ExportPNGForm::resizeEvent( QResizeEvent *event )
{
	if ( event->oldSize().width() < 0 || event->oldSize().height() < 0 ) {
		return ;
	}

	QSize sub = event->size() - event->oldSize() ;
	ui->textEdit->resize(QSize(ui->textEdit->width()+sub.width(), ui->textEdit->height())) ;
	ui->pushButton_sel_dir->move(QPoint(ui->pushButton_sel_dir->pos().x()+sub.width(), ui->pushButton_sel_dir->pos().y())) ;
}

void ExportPNGForm::slot_changeRect( void )
{
	int rect[4] ;
	m_pEditData->getExportPNGRect(rect);
	ui->spinBox_left->setValue(rect[0]);
	ui->spinBox_top->setValue(rect[1]);
	ui->spinBox_right->setValue(rect[2]);
	ui->spinBox_bottom->setValue(rect[3]);
}

void ExportPNGForm::slot_openSaveDir( void )
{
	QString dir = QFileDialog::getExistingDirectory(this, trUtf8("保存するディレクトリを選択"),
													 m_pSetting->getSaveDir(),
													 QFileDialog::ShowDirsOnly
													 | QFileDialog::DontResolveSymlinks);
	if ( dir.isEmpty() ) { return ; }
	m_pSetting->setSavePngDir(dir);
	ui->textEdit->setText(dir) ;
}

void ExportPNGForm::slot_changeLeft( int val )
{
	int rect[4] ;
	m_pEditData->getExportPNGRect(rect);
	rect[0] = val ;
	m_pEditData->setExportPNGRect(rect);
	emit sig_changeRect() ;
}

void ExportPNGForm::slot_changeRight( int val )
{
	int rect[4] ;
	m_pEditData->getExportPNGRect(rect);
	rect[2] = val ;
	m_pEditData->setExportPNGRect(rect);
	emit sig_changeRect() ;
}

void ExportPNGForm::slot_changeTop( int val )
{
	int rect[4] ;
	m_pEditData->getExportPNGRect(rect);
	rect[1] = val ;
	m_pEditData->setExportPNGRect(rect);
	emit sig_changeRect() ;
}

void ExportPNGForm::slot_changeBottom( int val )
{
	int rect[4] ;
	m_pEditData->getExportPNGRect(rect);
	rect[3] = val ;
	m_pEditData->setExportPNGRect(rect);
	emit sig_changeRect() ;
}

void ExportPNGForm::slot_startExport( void )
{
	QString saveDir = m_pSetting->getSavePngDir() ;
	QDir dir(saveDir) ;
	if ( !dir.exists() ) {
		QMessageBox::warning(this, tr("Error"), trUtf8("ディレクトリが存在しません") ) ;
		return ;
	}
	m_pEditData->startExportPNG(saveDir) ;
	emit sig_startExport() ;
}

void ExportPNGForm::slot_changeSaveDir( void )
{
	QString dir = ui->textEdit->toPlainText() ;
	m_pSetting->setSavePngDir(dir);
	qDebug() << "dir:" << dir ;
}

void ExportPNGForm::slot_cancel( void )
{
	emit sig_cancel() ;
}

