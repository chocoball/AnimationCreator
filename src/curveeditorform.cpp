#include <QStringListModel>
#include <QSplitter>
#include <QHBoxLayout>
#include "curveeditorform.h"
#include "ui_curveeditorform.h"

CurveEditorForm::CurveEditorForm(CEditData *pEditData, CSettings *pSetting, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurveEditorForm)
{
	m_fMag = 1 ;
	m_pEditData = pEditData ;
	m_pSetting = pSetting ;

	ui->setupUi(this) ;

	QSplitter *pSplitter = new QSplitter(this) ;
	pSplitter->addWidget(ui->listView) ;
	pSplitter->addWidget(ui->scrollArea) ;

	QHBoxLayout *pLayout = new QHBoxLayout(this) ;
	pLayout->addWidget(pSplitter) ;

	QStringList list ;
	list << "pos_x" << "pos_y" << "pos_z" << "rot_x" << "rot_y" << "rot_z" << "center_x" << "center_y" << "scale_x" << "scale_y" ;
	list << "top" << "left" << "right" << "bottom" << "r" << "g" << "b" << "a" ;
	QStringListModel *pModel = new QStringListModel(this) ;
	pModel->setStringList(list) ;
	ui->listView->setModel(pModel) ;

	m_pGraphLabel = new CurveGraphLabel(pEditData, this) ;
	m_pGraphLabel->setCurrentMag(m_fMag) ;
	ui->scrollArea->setWidget(m_pGraphLabel) ;

	connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_clickedListView(QModelIndex))) ;
}

CurveEditorForm::~CurveEditorForm()
{
    delete ui;
}

void CurveEditorForm::paintEvent(QPaintEvent *event)
{
	int h_pos = ui->scrollArea->horizontalScrollBar()->value() ;
	int v_pos = ui->scrollArea->verticalScrollBar()->value() ;

	qDebug() << "h_pos:" << h_pos << " v_pos:" << v_pos ;
}

// リストビュークリック時
void CurveEditorForm::slot_clickedListView(QModelIndex index)
{
	m_pGraphLabel->setCurrentDispType(index.row()) ;
	m_pGraphLabel->adjustSize() ;
	m_pGraphLabel->repaint() ;
}

// フレーム リサイズ時
void CurveEditorForm::slot_resizeFrame(QResizeEvent *event)
{
	QSize size = event->size() ;
	ui->scrollArea->resize(size.width()-10-10, size.height()-10-10);
}

// 選択レイヤ変更時
void CurveEditorForm::slot_changeSelLayer(QModelIndex index)
{
	m_pGraphLabel->setCurrentIndex(index) ;
	m_pGraphLabel->adjustSize() ;
	m_pGraphLabel->repaint() ;
}
