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
	setWindowTitle("GraphWindow") ;	// TODO:編集できるようになるまでこれで。

	m_pSplitter = new AnimationWindowSplitter(this) ;
	m_pSplitter->addWidget(ui->listView) ;
	m_pSplitter->addWidget(ui->scrollArea) ;

	QHBoxLayout *pLayout = new QHBoxLayout(this) ;
	pLayout->addWidget(m_pSplitter) ;

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
	connect(ui->scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slot_movedSlider(int))) ;
	connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slot_movedSlider(int))) ;
	connect(m_pSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(slot_movedSplitter(int, int))) ;
}

CurveEditorForm::~CurveEditorForm()
{
    delete ui;
}

void CurveEditorForm::setSplitterPos()
{
	m_pSplitter->MoveSplitter(m_pSetting->getCurveSplitterWidth(), m_pSetting->getCurveSplitterWidthIndex()) ;
}

void CurveEditorForm::resizeEvent(QResizeEvent *event)
{
	m_pGraphLabel->adjustSize() ;
	m_pGraphLabel->repaint() ;
}

// リストビュークリック時
void CurveEditorForm::slot_clickedListView(QModelIndex index)
{
	m_pGraphLabel->setCurrentDispType(index.row()) ;
	m_pGraphLabel->adjustSize() ;
	m_pGraphLabel->repaint() ;
}

// 選択レイヤ変更時
void CurveEditorForm::slot_changeSelLayer(QModelIndex index)
{
	m_pGraphLabel->setCurrentIndex(index) ;
	m_pGraphLabel->adjustSize() ;
	m_pGraphLabel->repaint() ;
}

void CurveEditorForm::slot_movedSlider(int val)
{
	m_pGraphLabel->repaint() ;
}

void CurveEditorForm::slot_movedSplitter(int pos, int index)
{
	m_pSetting->setCurveSplitterWidth(pos) ;
	m_pSetting->setCurveSplitterWidthIndex(index) ;
}
