#include <QStandardItemModel>
#include "defines.h"
#include "animationform.h"
#include "ui_animationform.h"
#include "mainwindow.h"

AnimationForm::AnimationForm(CEditData *pImageData, CSettings *pSetting, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnimationForm)
{
	m_pEditData		= pImageData ;
	m_pSetting		= pSetting ;
	m_bDontSetData	= false ;

	ui->setupUi(this);

	setFocusPolicy(Qt::StrongFocus);

	m_pGlWidget = new AnimeGLWidget(pImageData, pSetting, this) ;
	ui->scrollArea_anime->setWidget(m_pGlWidget);
	m_pGlWidget->resize(1024, 1024);
	m_pGlWidget->setDrawArea(1024, 1024);
	m_pGlWidget->show();

	ui->radioButton_pos->setChecked(true);
	ui->spinBox_nowSequence->setMaximum(CEditData::kMaxFrame);
	ui->horizontalSlider_nowSequence->setMaximum(CEditData::kMaxFrame);
	ui->spinBox_pos_x->setMinimum(-1028);
	ui->spinBox_pos_x->setMaximum(1028);
	ui->spinBox_pos_y->setMinimum(-1028);
	ui->spinBox_pos_y->setMaximum(1028);
	ui->spinBox_pos_z->setMinimum(-4096);
	ui->spinBox_pos_z->setMaximum(4096);
	ui->spinBox_rot_x->setMinimum(-360);
	ui->spinBox_rot_x->setMaximum(360);
	ui->spinBox_rot_y->setMinimum(-360);
	ui->spinBox_rot_y->setMaximum(360);
	ui->spinBox_rot_z->setMinimum(-360);
	ui->spinBox_rot_z->setMaximum(360);
	ui->spinBox_center_x->setMinimum(-1028);
	ui->spinBox_center_x->setMaximum(1028);
	ui->spinBox_center_y->setMinimum(-1028);
	ui->spinBox_center_y->setMaximum(1028);
	ui->doubleSpinBox_scale_x->setMinimum(-1024.0);
	ui->doubleSpinBox_scale_x->setMaximum(1024.0);
	ui->doubleSpinBox_scale_x->setSingleStep(0.01);
	ui->doubleSpinBox_scale_y->setMinimum(-1024.0);
	ui->doubleSpinBox_scale_y->setMaximum(1024.0);
	ui->doubleSpinBox_scale_y->setSingleStep(0.01);
	ui->spinBox_uv_left->setMinimum(0);
	ui->spinBox_uv_left->setMaximum(4096);
	ui->spinBox_uv_right->setMinimum(0);
	ui->spinBox_uv_right->setMaximum(4096);
	ui->spinBox_uv_top->setMinimum(0);
	ui->spinBox_uv_top->setMaximum(4096);
	ui->spinBox_uv_bottom->setMinimum(0);
	ui->spinBox_uv_bottom->setMaximum(4096);
	ui->spinBox_loop->setMinimum(-1);
	ui->spinBox_loop->setMaximum(1024);
	ui->spinBox_r->setMinimum(0);
	ui->spinBox_r->setMaximum(255);
	ui->spinBox_g->setMinimum(0);
	ui->spinBox_g->setMaximum(255);
	ui->spinBox_b->setMinimum(0);
	ui->spinBox_b->setMaximum(255);
	ui->spinBox_a->setMinimum(0);
	ui->spinBox_a->setMaximum(255);
	ui->checkBox_grid->setChecked(true);
	ui->comboBox_fps->addItem(tr("60 fps"));
	ui->comboBox_fps->addItem(tr("30 fps"));
	ui->comboBox_fps->addItem(tr("15 fps"));
	ui->comboBox_fps->setCurrentIndex(0);
	ui->checkBox_frame->setChecked(pSetting->getDrawFrame());

	for ( int i = 0 ; i < m_pEditData->getImageDataSize() ; i ++ ) {
		ui->comboBox_image_no->addItem(tr("%1").arg(i));
	}

	m_pSplitter = new QSplitter(this) ;
	m_pSplitter->addWidget(ui->treeView) ;
	m_pSplitter->addWidget(ui->scrollArea_anime) ;
	m_pSplitter->setGeometry(ui->treeView->pos().x(),
						   ui->treeView->pos().y(),
						   ui->scrollArea_anime->width()+ui->scrollArea_anime->pos().x()-ui->treeView->pos().x(),
						   ui->treeView->height());

	{
		QStandardItemModel *pTreeModel = m_pEditData->getTreeModel() ;

		ui->treeView->setModel(pTreeModel);
		ui->treeView->header()->setHidden(true);
		ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

		if ( !pTreeModel->invisibleRootItem()->rowCount() ) {
			addNewObject(trUtf8("New Object"));
		}
		else {
			QStandardItem *pItem = pTreeModel->item(0, 0) ;
			if ( pItem ) {
				ui->treeView->setCurrentIndex(pItem->index());
				m_pEditData->setSelectObject(pItem);
			}
		}
	}

	m_pActTreeViewAdd		= new QAction(QString("Add Object"), this);
	m_pActTreeViewCopy		= new QAction(QString("Copy Object"), this) ;
	m_pActTreeViewDel		= new QAction(QString("Delete"), this);
	m_pActTreeViewLayerDisp = new QAction(QString("Disp"), this) ;

	m_pTimer = new QTimer(this) ;
	m_pTimer->setInterval((int)(100.0f/6.0f));

	m_pDataMarker = new CDataMarkerLabel(m_pEditData, this) ;
	m_pDataMarker->setGeometry(ui->horizontalSlider_nowSequence->x()+5,
							   ui->horizontalSlider_nowSequence->y() + ui->horizontalSlider_nowSequence->height(),
							   ui->horizontalSlider_nowSequence->width()-10,
							   8);
	m_pDataMarker->setForegroundRole(QPalette::Highlight);
	m_pDataMarker->show();

	connect(ui->radioButton_pos, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioPos(bool))) ;
	connect(ui->radioButton_rot, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioRot(bool))) ;
	connect(ui->radioButton_center, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioCenter(bool))) ;
	connect(ui->radioButton_scale, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioScale(bool))) ;

	connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),	this, SLOT(slot_treeViewMenuReq(QPoint))) ;
//	connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),			this, SLOT(slot_treeViewDoubleClicked(QModelIndex))) ;
	connect(ui->treeView, SIGNAL(clicked(QModelIndex)),					this, SLOT(slot_changeSelectObject(QModelIndex))) ;

	connect(m_pGlWidget, SIGNAL(sig_dropedImage(QRect, QPoint, int)),
			this,		SLOT(slot_dropedImage(QRect, QPoint, int))) ;
	connect(m_pGlWidget, SIGNAL(sig_selectLayerChanged(QList<CObjectModel::typeID>)),
			this,		SLOT(slot_selectLayerChanged(QList<CObjectModel::typeID>))) ;
	connect(m_pGlWidget, SIGNAL(sig_dragedImage(CObjectModel::FrameData)),
			this,		SLOT(slot_setUI(CObjectModel::FrameData))) ;
	connect(m_pGlWidget, SIGNAL(sig_deleteFrameData()),
			this,		SLOT(slot_deleteFrameData())) ;
	connect(m_pGlWidget, SIGNAL(sig_selectPrevLayer(CObjectModel::typeID, CObjectModel::typeID, int, CObjectModel::FrameData)),
			this,		SLOT(slot_addNewFrameData(CObjectModel::typeID, CObjectModel::typeID, int, CObjectModel::FrameData))) ;
	connect(m_pGlWidget, SIGNAL(sig_frameDataMoveEnd()),
			this,		SLOT(slot_frameDataMoveEnd())) ;
//	connect(m_pGlWidget, SIGNAL(sig_copyFrameData()), this, SLOT(slot_copyFrameData())) ;
//	connect(m_pGlWidget, SIGNAL(sig_pasteFrameData()), this, SLOT(slot_pasteFrameData())) ;

	connect(ui->horizontalSlider_nowSequence, SIGNAL(valueChanged(int)), this, SLOT(slot_frameChanged(int))) ;
	connect(ui->spinBox_pos_x,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changePosX(int))) ;
	connect(ui->spinBox_pos_y,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changePosY(int))) ;
	connect(ui->spinBox_pos_z,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changePosZ(int))) ;
	connect(ui->spinBox_rot_x,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changeRotX(int))) ;
	connect(ui->spinBox_rot_y,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changeRotY(int))) ;
	connect(ui->spinBox_rot_z,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changeRotZ(int))) ;
	connect(ui->doubleSpinBox_scale_x,	SIGNAL(valueChanged(double)),	this, SLOT(slot_changeScaleX(double))) ;
	connect(ui->doubleSpinBox_scale_y,	SIGNAL(valueChanged(double)),	this, SLOT(slot_changeScaleY(double))) ;
	connect(ui->spinBox_uv_left,		SIGNAL(valueChanged(int)),		this, SLOT(slot_changeUvLeft(int))) ;
	connect(ui->spinBox_uv_right,		SIGNAL(valueChanged(int)),		this, SLOT(slot_changeUvRight(int))) ;
	connect(ui->spinBox_uv_top,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changeUvTop(int))) ;
	connect(ui->spinBox_uv_bottom,		SIGNAL(valueChanged(int)),		this, SLOT(slot_changeUvBottom(int))) ;
	connect(ui->spinBox_center_x,		SIGNAL(valueChanged(int)),		this, SLOT(slot_changeCenterX(int))) ;
	connect(ui->spinBox_center_y,		SIGNAL(valueChanged(int)),		this, SLOT(slot_changeCenterY(int))) ;
	connect(m_pActTreeViewAdd,			SIGNAL(triggered()),			this, SLOT(slot_createNewObject())) ;
	connect(m_pActTreeViewCopy,			SIGNAL(triggered()),			this, SLOT(slot_copyObject())) ;
	connect(m_pActTreeViewDel,			SIGNAL(triggered()),			this, SLOT(slot_deleteObject())) ;
	connect(m_pActTreeViewLayerDisp,	SIGNAL(triggered()),			this, SLOT(slot_changeLayerDisp())) ;
	connect(ui->pushButton_play,		SIGNAL(clicked()),				this, SLOT(slot_playAnimation())) ;
	connect(ui->pushButton_stop,		SIGNAL(clicked()),				this, SLOT(slot_stopAnimation())) ;
	connect(ui->pushButton_backward,	SIGNAL(clicked()),				this, SLOT(slot_backwardFrameData())) ;
	connect(ui->pushButton_forward,		SIGNAL(clicked()),				this, SLOT(slot_forwardFrameData())) ;
	connect(ui->checkBox_grid,			SIGNAL(clicked(bool)),			m_pGlWidget, SLOT(slot_setDrawGrid(bool))) ;
	connect(ui->checkBox_uv_anime,		SIGNAL(clicked(bool)),			this, SLOT(slot_changeUVAnime(bool))) ;
	connect(ui->comboBox_fps,			SIGNAL(activated(int)),			this, SLOT(slot_changeAnimeSpeed(int))) ;
	connect(ui->comboBox_image_no,		SIGNAL(activated(int)),			this, SLOT(slot_changeImageIndex(int))) ;
	connect(m_pTimer,					SIGNAL(timeout()),				this, SLOT(slot_timerEvent())) ;
	connect(ui->spinBox_loop,			SIGNAL(valueChanged(int)),		this, SLOT(slot_changeLoop(int))) ;
	connect(ui->spinBox_r,				SIGNAL(valueChanged(int)),		this, SLOT(slot_changeColorR(int))) ;
	connect(ui->spinBox_g,				SIGNAL(valueChanged(int)),		this, SLOT(slot_changeColorG(int))) ;
	connect(ui->spinBox_b,				SIGNAL(valueChanged(int)),		this, SLOT(slot_changeColorB(int))) ;
	connect(ui->spinBox_a,				SIGNAL(valueChanged(int)),		this, SLOT(slot_changeColorA(int))) ;
	connect(ui->checkBox_frame,			SIGNAL(clicked(bool)),			this, SLOT(slot_changeDrawFrame(bool))) ;

#ifndef LAYOUT_OWN
	QGridLayout *pLayout = new QGridLayout(this) ;
	pLayout->addWidget(ui->listWidget, 0, 0, 5, 1);
	pLayout->addWidget(ui->spinBox_nowSequence, 0, 1, 1, 1);
	pLayout->addWidget(ui->horizontalSlider_nowSequence, 0, 2, 1, 1);
	pLayout->addWidget(ui->scrollArea, 1, 1, 4, 4);
	pLayout->addWidget(ui->label_x, 1, 6, 1, 1);
	pLayout->addWidget(ui->spinBox_x, 1, 7, 1, 1);
	pLayout->addWidget(ui->label_y, 2, 6, 1, 1);
	pLayout->addWidget(ui->spinBox_y, 2, 7, 1, 1);
	pLayout->addWidget(ui->label_w, 3, 6, 1, 1);
	pLayout->addWidget(ui->spinBox_w, 3, 7, 1, 1);
	pLayout->addWidget(ui->label_h, 4, 6, 1, 1);
	pLayout->addWidget(ui->spinBox_h, 4, 7, 1, 1);
	setLayout(pLayout) ;
#endif
}

AnimationForm::~AnimationForm()
{
	delete ui;
}

#ifdef LAYOUT_OWN
// サイズ変更イベント
void AnimationForm::resizeEvent(QResizeEvent *event)
{
	QSize add = event->size() - event->oldSize() ;
	QSize add_h = QSize(0, add.height()) ;
//	QSize add_w = QSize(add.width(), 0) ;

	if ( event->oldSize().width() < 0 || event->oldSize().height() < 0 ) {
		return ;
	}

	ui->treeView->resize(ui->treeView->size()+add_h);
	ui->scrollArea_anime->resize(ui->scrollArea_anime->size()+add);
	m_pSplitter->resize(m_pSplitter->size()+add);

	ui->comboBox_image_no->move(ui->comboBox_image_no->pos() + QPoint(add.width(), 0));
	ui->groupBox->move(ui->groupBox->pos() + QPoint(add.width(), 0));

	QWidget *tmp[] = {
		ui->label_x,
		ui->label_y,
		ui->label_z,
		ui->label_uv,
		ui->label_uv_left,
		ui->label_uv_right,
		ui->label_uv_top,
		ui->label_uv_bottom,
		ui->label_image,
		ui->label_loop,
		ui->label_color,
		ui->label_r,
		ui->label_g,
		ui->label_b,
		ui->label_a,
		ui->spinBox_pos_x,
		ui->spinBox_pos_y,
		ui->spinBox_pos_z,
		ui->spinBox_rot_x,
		ui->spinBox_rot_y,
		ui->spinBox_rot_z,
		ui->spinBox_uv_left,
		ui->spinBox_uv_right,
		ui->spinBox_uv_top,
		ui->spinBox_uv_bottom,
		ui->spinBox_center_x,
		ui->spinBox_center_y,
		ui->spinBox_loop,
		ui->spinBox_r,
		ui->spinBox_g,
		ui->spinBox_b,
		ui->spinBox_a,
		ui->doubleSpinBox_scale_x,
		ui->doubleSpinBox_scale_y,
		ui->checkBox_uv_anime,
		ui->line_0,
		ui->line_1,
	} ;
	for ( int i = 0 ; i < ARRAY_NUM(tmp) ; i ++ ) {
		tmp[i]->move(tmp[i]->pos() + QPoint(add.width(), 0));
	}
}
#endif

// スクロールバーをセンターに。
void AnimationForm::setBarCenter( void )
{
	QScrollBar *pBar ;
	pBar = ui->scrollArea_anime->horizontalScrollBar() ;
	pBar->setValue( (pBar->maximum()-pBar->minimum()) / 2 );
	pBar = ui->scrollArea_anime->verticalScrollBar() ;
	pBar->setValue( (pBar->maximum()-pBar->minimum()) / 2 );
}

// デバッグ用。オブジェクト情報ダンプ
void AnimationForm::dbgDumpObject( void )
{
	if ( !m_pEditData->getObjectModel() ) { return ; }

	qDebug("Deump Object ------------------------") ;
	const CObjectModel::ObjectList &objList = m_pEditData->getObjectModel()->getObjectList() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		qDebug("Object [%d] ID:%p", i, objList.at(i).id) ;

		const CObjectModel::LayerGroupList &layerGroupList = objList.at(i).layerGroupList ;
		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			qDebug("  LayerGroup [%d] ID:%p", j, layerGroupList.at(j).first ) ;

			const CObjectModel::FrameDataList &frameDataList = layerGroupList.at(j).second ;
			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				const CObjectModel::FrameData data = frameDataList.at(k) ;
				qDebug("    FrameData [%d] x:%d y:%d z:%d", k, data.pos_x, data.pos_y, data.pos_z) ;
				qDebug("              cx:%d cy:%d frame:%d", data.center_x, data.center_y, data.frame) ;
				qDebug("              scaX:%f scaY:%f", data.fScaleX, data.fScaleY) ;
				qDebug("              l:%d r:%d t:%d b:%d", data.left, data.right, data.top, data.bottom) ;
				qDebug("              bUVAnime:%d", data.bUVAnime) ;
			}
		}
	}
	qDebug("end ---------------------------------") ;
}

// オブジェクト新規作成
void AnimationForm::slot_createNewObject( void )
{
	bool bOk = false ;
	QString str = QInputDialog::getText(this,
										trUtf8("New Object"),
										trUtf8("オブジェクト名を入力してください"),
										QLineEdit::Normal,
										trUtf8("New Object"),
										&bOk) ;
	if ( !bOk ) { return ; }
	if ( str.isEmpty() ) { return ; }

//	addNewObject(str);
	QStandardItem *newItem = m_pEditData->cmd_addNewObject(str);
	ui->treeView->setCurrentIndex(newItem->index());
	m_pEditData->setSelectObject(newItem);

	ui->spinBox_loop->setValue(0);
}

// オブジェクト削除
void AnimationForm::slot_deleteObject( void )
{
//	QStandardItemModel *pTreeModel = m_pEditData->getTreeModel() ;
	QModelIndex index = ui->treeView->currentIndex() ;

	if ( !index.isValid() ) { return ; }

	m_pEditData->cmd_delObject(index, m_pDataMarker);
	slot_changeSelectObject(ui->treeView->currentIndex());
}

// フレームデータ削除
void AnimationForm::slot_deleteFrameData(void)
{
	CObjectModel *pModel			= m_pEditData->getObjectModel() ;
	CObjectModel::typeID objID		= m_pEditData->getSelectObject() ;
	CObjectModel::typeID layerID	= m_pEditData->getSelectLayer() ;
	int frame						= m_pEditData->getSelectFrame() ;
#if 1
	QList<QWidget *> update ;
	update << m_pDataMarker << m_pGlWidget ;
	m_pEditData->cmd_delFrameData(objID, layerID, frame, update) ;

	CObjectModel::FrameDataList *p = pModel->getFrameDataListFromID(objID, layerID) ;
	if ( !p || p->size() == 0 ) {
		QStandardItem *item = (QStandardItem *)layerID ;
		m_pEditData->cmd_delObject(item->index(), m_pDataMarker);
		slot_changeSelectObject(ui->treeView->currentIndex());
	}
#else
	if ( pModel->delLayerData(objID, layerID, frame) ) {
		CObjectModel::FrameDataList *p = pModel->getFrameDataListFromID(objID, layerID) ;
		if ( !p || p->size() == 0 ) {
			QStandardItem *item = (QStandardItem *)layerID ;
			m_pEditData->getTreeModel()->removeRow(item->index().row(), item->parent()->index()) ;
			slot_changeSelectObject(ui->treeView->currentIndex());
		}
	}
	m_pDataMarker->update();
#endif
}

// ドロップ時のスロット
// レイヤ追加
void AnimationForm::slot_dropedImage( QRect rect, QPoint pos, int imageIndex )
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	int frameNum  = ui->horizontalSlider_nowSequence->value() ;
	QModelIndex index = ui->treeView->currentIndex() ;

	qDebug() << "AnimationForm::slot_dropedImage" ;
	qDebug("row:%d col:%d ptr:%p root:%p", index.row(), index.column(), index.internalPointer(), m_pEditData->getTreeModel()->invisibleRootItem()) ;

	if ( !index.isValid() ) {
		qWarning() << "slot_dropedImage current index invalid 0" ;
		return ;
	}

	while ( index.internalPointer() != m_pEditData->getTreeModel()->invisibleRootItem() ) {
		index = index.parent() ;
		if ( !index.isValid() ) {
			qWarning() << "slot_dropedImage current index invalid 1" ;
			return ;
		}
	}

	QStandardItem *pParentItem = m_pEditData->getTreeModel()->itemFromIndex(index) ;
	if ( !pParentItem ) {
		qWarning() << "slot_dropedImage pParentItem == NULLpo !!" ;
		return ;
	}

	CObjectModel::LayerGroupList *pLayerGroupList = pModel->searchLayerGroupList(pParentItem) ;
	if ( !pLayerGroupList ) {
		qWarning() << "slot_dropedImage pLayerGroupList == NULLpo !!" ;
		return ;
	}

	pos -= QPoint(512, 512) ;	// GLWidgetのローカルポスに変換

	// ツリービューに追加
	QStandardItem *newItem = new QStandardItem(QString("Layer %1").arg(pLayerGroupList->size())) ;
	newItem->setData(true, Qt::CheckStateRole);
#if 1
	CObjectModel::FrameData frameData ;
	frameData.pos_x = pos.x() ;
	frameData.pos_y = pos.y() ;
	frameData.pos_z = 0 ;
	frameData.rot_x =
	frameData.rot_y =
	frameData.rot_z = 0 ;
	frameData.center_x = (rect.width()) / 2 ;
	frameData.center_y = (rect.height()) / 2 ;
	frameData.frame = frameNum ;
	frameData.fScaleX = frameData.fScaleY = 1.0f ;
	frameData.setRect(rect);
	frameData.nImage = imageIndex ;
	frameData.bUVAnime = false ;
	frameData.rgba[0] =
	frameData.rgba[1] =
	frameData.rgba[2] =
	frameData.rgba[3] = 255 ;

	QList<QWidget *> updateWidget ;
	updateWidget << m_pGlWidget ;
	updateWidget << m_pDataMarker ;
	m_pEditData->cmd_addNewLayer(index, newItem, frameData, updateWidget) ;

	QList<CObjectModel::typeID> list ;
	list << newItem ;
	m_pEditData->setSelectLayer(list);
	slot_selectLayerChanged(list) ;
//	slot_setUI(frameData);
#else
	pParentItem->appendRow(newItem);

	// データを追加
	CObjectModel::FrameData frameData ;
	frameData.x = pos.x() ;
	frameData.y = pos.y() ;
	frameData.z = 0 ;
	frameData.center_x = (rect.width()) / 2 ;
	frameData.center_y = (rect.height()) / 2 ;
	frameData.frame = frameNum ;
	frameData.fScaleX = frameData.fScaleY = 1.0f ;
	frameData.setRect(rect);

	CObjectModel::LayerGroup layerGroup ;
	layerGroup.first = newItem ;
	layerGroup.second.append(frameData) ;
	pLayerGroupList->append(layerGroup);

	m_pEditData->setSelectLayer(newItem);
	slot_setUI(frameData);

	m_pGlWidget->update();
	m_pDataMarker->update();
#endif
}

// 現在フレーム変更
void AnimationForm::slot_frameChanged(int frame)
{
	bool bChange = (m_pEditData->getSelectFrame() != frame) ;
	m_pEditData->setSelectFrame( frame ) ;
	if ( bChange ) {
		QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
		if ( Datas.size() ) {
			slot_setUI(*(Datas[0]));
		}
#if 1
		else {
			if ( m_pEditData->getSelectLayerNum() ) {
				CObjectModel::typeID objID		= m_pEditData->getSelectObject() ;
				CObjectModel::typeID layerID	= m_pEditData->getSelectLayer() ;
				if ( objID && layerID ) {
					CObjectModel *pModel = m_pEditData->getObjectModel() ;
					CObjectModel::FrameData *pPrev = pModel->getFrameDataFromPrevFrame(objID, layerID, frame, false) ;
					CObjectModel::FrameData *pNext = pModel->getFrameDataFromNextFrame(objID, layerID, frame) ;
					if ( pPrev ) {
						CObjectModel::FrameData data = pPrev->getInterpolation(pNext, frame) ;
						slot_setUI(data) ;
					}
				}
			}
		}
#endif
		m_pEditData->updateSelectData();
	}
	m_pGlWidget->update();
}

// 選択レイヤ変更
void AnimationForm::slot_selectLayerChanged( QList<CObjectModel::typeID> layerIDs )
{
	m_pEditData->setSelectLayer(layerIDs);
	if ( layerIDs.size() <= 0 ) { return ; }

	QStandardItem *item = (QStandardItem *)layerIDs[0] ;
	if ( item ) {
		ui->treeView->setCurrentIndex(item->index());
	}

	m_pGlWidget->update();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( Datas.size() ) {
		slot_setUI(*(Datas[0]));
	}
}

// ＵＩ数値セット
void AnimationForm::slot_setUI(CObjectModel::FrameData data)
{
	m_bDontSetData = true ;
	ui->spinBox_pos_x->setValue(data.pos_x);
	ui->spinBox_pos_y->setValue(data.pos_y);
	ui->spinBox_pos_z->setValue(data.pos_z);
	ui->spinBox_rot_x->setValue(data.rot_x);
	ui->spinBox_rot_y->setValue(data.rot_y);
	ui->spinBox_rot_z->setValue(data.rot_z);
	ui->doubleSpinBox_scale_x->setValue(data.fScaleX) ;
	ui->doubleSpinBox_scale_y->setValue(data.fScaleY) ;
	ui->spinBox_uv_left->setValue(data.left);
	ui->spinBox_uv_right->setValue(data.right);
	ui->spinBox_uv_top->setValue(data.top);
	ui->spinBox_uv_bottom->setValue(data.bottom);
	ui->spinBox_center_x->setValue(data.center_x);
	ui->spinBox_center_y->setValue(data.center_y);
	ui->comboBox_image_no->setCurrentIndex(data.nImage);
	ui->checkBox_uv_anime->setChecked(data.bUVAnime);
	ui->spinBox_r->setValue(data.rgba[0]);
	ui->spinBox_g->setValue(data.rgba[1]);
	ui->spinBox_b->setValue(data.rgba[2]);
	ui->spinBox_a->setValue(data.rgba[3]);
	m_bDontSetData = false ;

	if ( data.getRect() != m_pEditData->getCatchRect() ) {
		QRect rect = data.getRect() ;
		m_pEditData->setCatchRect(rect);
		emit sig_imageRepaint() ;
	}
}

// pos x 変更
void AnimationForm::slot_changePosX( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->pos_x = val ;
	addCommandEdit(Datas) ;
}

// pos y 変更
void AnimationForm::slot_changePosY( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->pos_y = val ;
	addCommandEdit(Datas) ;
}

// pos z 変更
void AnimationForm::slot_changePosZ( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->pos_z = val ;
	addCommandEdit(Datas) ;
}

// rot x 変更
void AnimationForm::slot_changeRotX( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->rot_x = val ;
	addCommandEdit(Datas) ;
}

// rot y 変更
void AnimationForm::slot_changeRotY( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->rot_y = val ;
	addCommandEdit(Datas) ;
}

// rot z 変更
void AnimationForm::slot_changeRotZ( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->rot_z = val ;
	addCommandEdit(Datas) ;
}

// scale x 変更
void AnimationForm::slot_changeScaleX( double val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->fScaleX = val ;
	addCommandEdit(Datas) ;
}

// scale y 変更
void AnimationForm::slot_changeScaleY( double val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->fScaleY = val ;
	addCommandEdit(Datas) ;
}

// uv left 変更
void AnimationForm::slot_changeUvLeft( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->left = val ;
	addCommandEdit(Datas) ;

	QRect rect = Datas[0]->getRect() ;
	m_pEditData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// uv right 変更
void AnimationForm::slot_changeUvRight( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->right = val ;
	addCommandEdit(Datas) ;

	QRect rect = Datas[0]->getRect() ;
	m_pEditData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// uv top 変更
void AnimationForm::slot_changeUvTop( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->top = val ;
	addCommandEdit(Datas) ;

	QRect rect = Datas[0]->getRect() ;
	m_pEditData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// uv bottom 変更
void AnimationForm::slot_changeUvBottom( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->bottom = val ;
	addCommandEdit(Datas) ;

	QRect rect = Datas[0]->getRect() ;
	m_pEditData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// center x 変更
void AnimationForm::slot_changeCenterX( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->center_x = val ;

	addCommandEdit(Datas) ;
}

// center y 変更
void AnimationForm::slot_changeCenterY( int val )
{
	if ( m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None ) { return ; }
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->center_y = val ;

	addCommandEdit(Datas) ;
}

// ツリービュー メニューリクエスト
void AnimationForm::slot_treeViewMenuReq(QPoint treeViewLocalPos)
{
	QMenu menu(this) ;
	menu.addAction(m_pActTreeViewAdd) ;
	menu.addAction(m_pActTreeViewDel) ;
	if ( ui->treeView->currentIndex().internalPointer() != m_pEditData->getTreeModel()->invisibleRootItem() ) {
		// レイヤ選択中だったら
		menu.addAction(m_pActTreeViewLayerDisp) ;
	}
	else {
		// オブジェクト選択中だったら
		menu.addAction(m_pActTreeViewCopy) ;
	}
	menu.exec(ui->treeView->mapToGlobal(treeViewLocalPos) + QPoint(0, ui->treeView->header()->height())) ;
}

// ツリービュー ダブルクリック
void AnimationForm::slot_treeViewDoubleClicked(QModelIndex index)
{
	QStandardItemModel *pTreeModel = m_pEditData->getTreeModel() ;
	QStandardItem *pItem = pTreeModel->itemFromIndex(index) ;

	if ( !pItem ) { return ; }
	if ( index.internalPointer() == pTreeModel->invisibleRootItem() ) { return ; }	// オブジェクト選択中

	QVariant flag = pItem->data(Qt::CheckStateRole) ;
	pItem->setData(!flag.toBool(), Qt::CheckStateRole);
	m_pGlWidget->update();
}

// 選択オブジェクト変更
void AnimationForm::slot_changeSelectObject(QModelIndex index)
{
	QList<CObjectModel::typeID> list ;
	m_pEditData->setSelectObject(0);
	m_pEditData->setSelectLayer(list);

	if ( !index.isValid() ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;

	if ( index.internalPointer() == m_pEditData->getTreeModel()->invisibleRootItem() ) {	// オブジェクト選択
		qDebug("select Object:%d", index.row()) ;
		m_pEditData->setSelectObject(m_pEditData->getTreeModel()->itemFromIndex(index));
	}
	else {		// レイヤ選択
		qDebug("select Layer:%d parent:%d", index.row(), index.parent().row()) ;
		list << m_pEditData->getTreeModel()->itemFromIndex(index) ;
		m_pEditData->setSelectObject(m_pEditData->getTreeModel()->itemFromIndex(index.parent()));
		m_pEditData->setSelectLayer(list);

		CObjectModel::FrameData *pData = pModel->getFrameDataFromIDAndFrame(m_pEditData->getSelectObject(),
																			m_pEditData->getSelectLayer(),
																			m_pEditData->getSelectFrame()) ;
		if ( pData ) {
			slot_setUI(*pData);
		}
	}

	// ループ回数設定
	CObjectModel::ObjectGroup *pObjGroup = pModel->getObjectGroupFromID(m_pEditData->getSelectObject()) ;
	if ( pObjGroup ) {
		ui->spinBox_loop->setValue(pObjGroup->nLoop);
	}

	m_pDataMarker->repaint();
	m_pGlWidget->update();
}

// アニメ再生
void AnimationForm::slot_playAnimation( void )
{
	if ( !m_pEditData->isPauseAnime() ) {	// ポーズ中じゃなかったら０フレームから再生
		ui->horizontalSlider_nowSequence->setValue(0);
		m_pEditData->setCurrLoopNum(0);
	}

	m_pEditData->setPlayAnime(true) ;

	// 最大フレーム設定
	m_nMaxFrameNum = m_pEditData->getObjectModel()->getMaxFrameFromSelectObject(m_pEditData->getSelectObject());

	m_pEditData->setPauseAnime(false);
	disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation())) ;
	connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_pauseAnimation())) ;

	QIcon icon;
	icon.addFile(QString::fromUtf8(":/root/Resources/images/Button Pause_32.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->pushButton_play->setIcon(icon);

	m_pTimer->start();	// タイマースタート
	m_pGlWidget->update();
}


// アニメ一時停止
void AnimationForm::slot_pauseAnimation( void )
{
	m_pEditData->setPlayAnime(false) ;
	m_pEditData->setPauseAnime(true);
	disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_pauseAnimation())) ;
	connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation())) ;

	QIcon icon;
	icon.addFile(QString::fromUtf8(":/root/Resources/images/Button Play_32.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->pushButton_play->setIcon(icon);

	m_pTimer->stop();			// タイマー停止
	m_pGlWidget->update();
}

// アニメ停止
void AnimationForm::slot_stopAnimation( void )
{
	disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation())) ;
	disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_pauseAnimation())) ;
	connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation())) ;

	m_pEditData->setPlayAnime(false) ;
	m_pEditData->setPauseAnime(false);
	ui->horizontalSlider_nowSequence->setValue(0);

	QIcon icon;
	icon.addFile(QString::fromUtf8(":/root/Resources/images/Button Play_32.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->pushButton_play->setIcon(icon);

	m_pTimer->stop();			// タイマー停止
	m_pGlWidget->update();
}

// 前フレームのフレームデータに変更
void AnimationForm::slot_backwardFrameData( void )
{
#if 1
	// 選択中のレイヤだけ
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	CObjectModel::typeID layerID = m_pEditData->getSelectLayer() ;
	if ( !objID || !layerID ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	const CObjectModel::FrameDataList *pFrameDataList = pModel->getFrameDataListFromID(objID, layerID) ;
	if ( !pFrameDataList ) {
		return ;
	}
	for ( int i = m_pEditData->getSelectFrame()-1 ; i >= 0 ; i -- ) {
		if ( setSelectFrameDataFromFrame(i, layerID, *pFrameDataList) ) {
			break ;
		}
	}
#else
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	if ( !objID ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	const CObjectModel::LayerGroupList *pLayerGroupList = pModel->getLayerGroupListFromID(objID) ;
	if ( !pLayerGroupList ) { return ; }

	for ( int i = m_pEditData->getSelectFrame()-1 ; i >= 0 ; i -- ) {
		if ( setSelectFrameDataFromFrame(i, *pLayerGroupList) ) {
			break ;
		}
	}
#endif
}

// 次フレームのフレームデータに変更
void AnimationForm::slot_forwardFrameData( void )
{
#if 1
	// 選択中のレイヤだけ
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	CObjectModel::typeID layerID = m_pEditData->getSelectLayer() ;
	if ( !objID || !layerID ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	const CObjectModel::FrameDataList *pFrameDataList = pModel->getFrameDataListFromID(objID, layerID) ;
	if ( !pFrameDataList ) {
		return ;
	}
	for ( int i = m_pEditData->getSelectFrame()+1 ; i <= CEditData::kMaxFrame ; i ++ ) {
		if ( setSelectFrameDataFromFrame(i, layerID, *pFrameDataList) ) {
			break ;
		}
	}
#else
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	if ( !objID ) { return ; }
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	const CObjectModel::LayerGroupList *pLayerGroupList = pModel->getLayerGroupListFromID(objID) ;
	if ( !pLayerGroupList ) { return ; }

	for ( int i = m_pEditData->getSelectFrame()+1 ; i <= CEditData::kMaxFrame ; i ++ ) {
		if ( setSelectFrameDataFromFrame(i, *pLayerGroupList) ) {
			break ;
		}
	}
#endif
}

// タイマイベント。フレームを進める
void AnimationForm::slot_timerEvent( void )
{
	int frame = ui->horizontalSlider_nowSequence->value() ;

	// PNG吐き出し中
	if ( m_pEditData->isExportPNG() ) {
		if ( m_pEditData->getExportEndFrame() < frame ) {
			return ;
		}
	}

	frame ++ ;

	if ( frame > m_nMaxFrameNum ) {
		if ( m_pEditData->isExportPNG() || m_pEditData->addCurrLoopNum(1) ) {
			// ループ終了
			m_pEditData->endExportPNG() ;
			slot_stopAnimation() ;
			ui->horizontalSlider_nowSequence->setValue(frame-1);
			return ;
		}
		frame = 0 ;
	}

	ui->horizontalSlider_nowSequence->setValue(frame);
}

// フレームデータ追加
void AnimationForm::slot_addNewFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, CObjectModel::FrameData data )
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::FrameDataList *pFDList = pModel->getFrameDataListFromID(objID, layerID) ;

	if ( !pFDList ) { return ; }
	for ( int i = 0 ; i < pFDList->size() ; i ++ ) {
		const CObjectModel::FrameData &Data = pFDList->at(i) ;
		if ( Data.frame == frame ) { return ; }		// 同フレームのデータがあったら終わる
	}
#if 1
	data.frame = frame ;
	QList<QWidget *> update ;
	update << m_pDataMarker << m_pGlWidget ;
	m_pEditData->cmd_addNewFrameData(objID, layerID, data, update) ;
#else
	data.frame = frame ;
	pFDList->append(data);

	m_pDataMarker->update();
#endif
}

// レイヤ表示ON/OFF
void AnimationForm::slot_changeLayerDisp( void )
{
	QModelIndex index = ui->treeView->currentIndex() ;
	if ( !index.isValid() ) { return ; }
	if ( index.internalPointer() == m_pEditData->getTreeModel()->invisibleRootItem() ) { return ; }	// オブジェクト選択中

	slot_treeViewDoubleClicked(index) ;
}

// 選択中レイヤのUV変更
void AnimationForm::slot_changeSelectLayerUV( QRect rect )
{
	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) {
		return ;
	}
	Datas[0]->setRect(rect) ;
	addCommandEdit(Datas) ;

	slot_setUI(*(Datas[0]));
}

// FPS変更
void AnimationForm::slot_changeAnimeSpeed(int index)
{
	if ( index < 0 || index > 2 ) { return ; }

	float frame[] = { 60, 30, 15 } ;
	bool bPlay = m_pTimer->isActive() ;
	m_pTimer->stop();
	m_pTimer->setInterval((int)(1000.0f/frame[index]));
	if ( bPlay ) {
		m_pTimer->start();
	}
}

// イメージ追加
void AnimationForm::slot_addImage( int imageNo )
{
	qDebug() << "AnimationForm::slot_addImage:" << imageNo ;

	ui->comboBox_image_no->addItem(tr("%1").arg(imageNo));

	if ( !m_pEditData->getTexObj(imageNo) ) {
		GLuint obj = m_pGlWidget->bindTexture(m_pEditData->getImage(imageNo)) ;
		m_pEditData->setTexObj(imageNo, obj);
	}
}

// イメージ削除
void AnimationForm::slot_delImage( int imageNo )
{
	ui->comboBox_image_no->removeItem(imageNo);
	for ( int i = 0 ; i < ui->comboBox_image_no->count() ; i ++ ) {
		ui->comboBox_image_no->setItemText(i, tr("%1").arg(i));
	}

	m_pEditData->removeImageData(imageNo) ;
	m_pGlWidget->update();
}

// イメージ番号変更
void AnimationForm::slot_changeImageIndex(int index)
{
	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) {
		return ;
	}

	for ( int i = 0 ; i < Datas.size() ; i ++ ) {
		Datas[i]->nImage = index ;
	}
	m_pGlWidget->update();
	addCommandEdit(Datas) ;
}

// UVアニメON/OFF
void AnimationForm::slot_changeUVAnime( bool flag )
{
	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) {
		return ;
	}
	for ( int i = 0 ; i < Datas.size() ; i ++ ) {
		Datas[i]->bUVAnime = flag ;
	}
	addCommandEdit(Datas) ;
}

// イメージ更新
void AnimationForm::slot_modifiedImage(int index)
{
	QImage &image = m_pEditData->getImage(index) ;
	if ( m_pEditData->getTexObj(index) ) {
		m_pGlWidget->deleteTexture(m_pEditData->getTexObj(index)) ;
	}
	GLuint obj = m_pGlWidget->bindTexture(image) ;
	m_pEditData->setTexObj(index, obj);
	m_pGlWidget->update();
}

// オプションダイアログ終了時
void AnimationForm::slot_endedOption( void )
{
	m_pGlWidget->setBackImage(m_pEditData->getBackImagePath()) ;
	m_pGlWidget->update();
}

// マウスでのデータ編集終了時
void AnimationForm::slot_frameDataMoveEnd( void )
{
	int i ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( i = 0 ; i < m_pEditData->getSelectLayerNum() ; i ++ ) {
		CObjectModel::FrameData *p = pModel->getFrameDataFromIDAndFrame(m_pEditData->getSelectObject(),
																		m_pEditData->getSelectLayer(i),
																		m_pEditData->getSelectFrame()) ;
		if ( !p ) { continue ; }
		if ( !m_pEditData->getSelectFrameData(i) ) { continue ; }
		if ( (*p) != *m_pEditData->getSelectFrameData(i) ) {
			break ;
		}
	}
	if ( i == m_pEditData->getSelectLayerNum() ) { return ; }	// データが変わっていない

	QList<CObjectModel::FrameData *> data_ptrs ;
	for ( int i = 0 ; i < m_pEditData->getSelectFrameDataNum() ; i ++ ) {
		data_ptrs.insert(i, m_pEditData->getSelectFrameData(i));
	}

	addCommandEdit(data_ptrs);
}

// ラジオボタン POS クリック
void AnimationForm::slot_clickedRadioPos( bool flag )
{
	if ( flag ) {
		m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Pos) ;
	}
	m_pGlWidget->update();
}

// ラジオボタン ROT クリック
void AnimationForm::slot_clickedRadioRot( bool flag )
{
	if ( flag ) {
		m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Rot) ;
	}
	m_pGlWidget->update();
}

// ラジオボタン CENTER クリック
void AnimationForm::slot_clickedRadioCenter( bool flag )
{
	if ( flag ) {
		m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Center) ;
	}
	m_pGlWidget->update();
}

// ラジオボタン SCALE クリック
void AnimationForm::slot_clickedRadioScale( bool flag )
{
	if ( flag ) {
		m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Scale) ;
	}
	m_pGlWidget->update();
}

// ループ回数変更
void AnimationForm::slot_changeLoop( int val )
{
	CObjectModel *pObjModel = m_pEditData->getObjectModel() ;
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	if ( !objID ) { return ; }

	CObjectModel::ObjectGroup *p = pObjModel->getObjectGroupFromID(objID) ;
	if ( !p ) { return ; }
	p->nLoop = val ;
}

// 色R変更
void AnimationForm::slot_changeColorR( int val )
{
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->rgba[0] = val ;
	addCommandEdit(Datas) ;
}

// 色G変更
void AnimationForm::slot_changeColorG( int val )
{
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->rgba[1] = val ;
	addCommandEdit(Datas) ;
}

// 色B変更
void AnimationForm::slot_changeColorB( int val )
{
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->rgba[2] = val ;
	addCommandEdit(Datas) ;
}

// 色A変更
void AnimationForm::slot_changeColorA( int val )
{
	if ( m_bDontSetData ) { return ; }

	addNowSelectLayerAndFrame();
	QList<CObjectModel::FrameData *> Datas = getNowSelectFrameData() ;
	if ( !Datas.size() ) { return ; }

	Datas[0]->rgba[3] = val ;
	addCommandEdit(Datas) ;
}


// 現在選択しているフレームデータ取得
QList<CObjectModel::FrameData *> AnimationForm::getNowSelectFrameData( void )
{
	QList<CObjectModel::FrameData *> ret ;

	for ( int i = 0 ; i < m_pEditData->getSelectFrameDataNum() ; i ++ ) {
		ret << m_pEditData->getSelectFrameData(i) ;
	}
	return ret ;
}

// 選択オブジェクトをコピー
void AnimationForm::slot_copyObject( void )
{
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	if ( !objID ) { return ; }

#if 1
	QList<QWidget *> updateWidget ;
	updateWidget << m_pGlWidget << this ;
	m_pEditData->cmd_copyObject(objID, updateWidget) ;
#else
	QStandardItem *pObjItem = objID ;
	QString newName = pObjItem->text() + "_copy" ;

	addNewObject(newName) ;
	CObjectModel::typeID newObjID = m_pEditData->getSelectObject() ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::ObjectGroup *pSrcObj = pModel->getObjectGroupFromID(objID) ;
	CObjectModel::ObjectGroup *pDstObj = pModel->getObjectGroupFromID(newObjID) ;
	if ( !pSrcObj || !pDstObj ) {
		qDebug() << "copy failed!!" ;
		return ;
	}
	for ( int i = 0 ; i < pSrcObj->layerGroupList.size() ; i ++ ) {
		CObjectModel::FrameDataList *pList = &pSrcObj->layerGroupList[i].second ;
		CObjectModel::FrameDataList newFrameDataList ;
		for ( int j = 0 ; j < pList->size() ; j ++ ) {
			newFrameDataList.append(pList[j]);
		}
		QString name = pSrcObj->layerGroupList[i].first->text() ;
		QStandardItem *pItem = new QStandardItem(name) ;
		CObjectModel::LayerGroup layerGroup = qMakePair(pItem, newFrameDataList) ;
		pDstObj->layerGroupList.append(layerGroup);
	}
#endif
}

void AnimationForm::slot_changeDrawFrame(bool flag)
{
	m_pSetting->setDrawFrame(flag);
	m_pGlWidget->update();
}

// オブジェクト追加
void AnimationForm::addNewObject( QString str )
{
	if ( str.isEmpty() ) { return ; }

	QStandardItem *newItem = new QStandardItem(str) ;
	QStandardItem *item = m_pEditData->getTreeModel()->invisibleRootItem() ;
	item->appendRow(newItem) ;
	qDebug("addNewObject row:%d col:%d ptr:%p root:%p", newItem->index().row(), newItem->index().column(), newItem->index().internalPointer(), item) ;

	CObjectModel::ObjectGroup obj ;
	obj.id = newItem ;
	obj.nCurrentLoop = 0 ;
	obj.nLoop = 0 ;
	m_pEditData->getObjectModel()->addObject(obj) ;

	qDebug("newItem valid:%d row:%d col:%d", newItem->index().isValid(), newItem->row(), newItem->column()) ;
	ui->treeView->setCurrentIndex(newItem->index());
	m_pEditData->setSelectObject(newItem);

	ui->spinBox_loop->setValue(0);
}

// フレームデータ編集コマンド
void AnimationForm::addCommandEdit( QList<CObjectModel::FrameData *> &rData )
{
	CObjectModel::typeID	objID	= m_pEditData->getSelectObject() ;
	int						frame	= m_pEditData->getSelectFrame() ;
	QList<QWidget *> update ;
	QList<CObjectModel::FrameData> datas ;

	if ( rData.size() <= 0 ) { return ; }

	for ( int i = 0 ; i < rData.size() ; i ++ ) {
		datas.insert(i, *(rData[i])) ;
	}

	update << m_pGlWidget << this ;
	m_pEditData->cmd_editFrameData(objID, m_pEditData->getSelectLayers(), frame, datas, update);
}

// 指定フレームのフレームデータを選択する
#if 1
bool AnimationForm::setSelectFrameDataFromFrame( int frame, CObjectModel::typeID layerID, const CObjectModel::FrameDataList &frameDataList )
{
	for ( int i = 0 ; i < frameDataList.size() ; i ++ ) {
		if ( frame == frameDataList[i].frame ) {
			QList<CObjectModel::typeID> layers ;
			layers << layerID ;
			m_pEditData->setSelectFrame(frame) ;
			m_pEditData->setSelectLayer(layers);
			ui->spinBox_nowSequence->setValue(frame);
			QList<CObjectModel::FrameData *> datas = getNowSelectFrameData() ;
			if ( datas.size() <= 0 ) {
				qDebug() << "setSelectFrameDataFromFrame:frame data not found!!" ;
			}
			slot_setUI(*(datas[0]));
			m_pGlWidget->update();
			return true ;
		}
	}
	return false ;
}
#else
bool AnimationForm::setSelectFrameDataFromFrame( int frame, const CObjectModel::LayerGroupList &layerGroupList )
{
	for ( int i = 0 ; i < layerGroupList.size() ; i ++ ) {
		const CObjectModel::LayerGroup &layerGroup = layerGroupList.at(i) ;
		CObjectModel::typeID layerID = layerGroup.first ;
		const CObjectModel::FrameDataList &frameDataList = layerGroup.second ;
		for ( int j = 0 ; j < frameDataList.size() ; j ++ ) {
			if ( frame == frameDataList[j].frame ) {
				QList<CObjectModel::typeID> layers ;
				layers << layerID ;
				m_pEditData->setSelectFrame(frame) ;
				m_pEditData->setSelectLayer(layers);
				ui->spinBox_nowSequence->setValue(frame);
				QList<CObjectModel::FrameData *> datas = getNowSelectFrameData() ;
				if ( datas.size() <= 0 ) {
					qDebug() << "setSelectFrameDataFromFrame:frame data not found!!" ;
				}
				slot_setUI(*(datas[0]));
				m_pGlWidget->update();
				return true ;
			}
		}
	}
	return false ;
}
#endif

// 選択レイヤの選択フレームにフレームデータを追加
void AnimationForm::addNowSelectLayerAndFrame( void )
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	int frame = m_pEditData->getSelectFrame() ;

	if ( !objID ) { return ; }

	for ( int i = 0 ; i < m_pEditData->getSelectLayerNum() ; i ++ ) {
		CObjectModel::typeID layerID = m_pEditData->getSelectLayer(i) ;
		if ( !layerID ) { continue ; }
		if ( pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ) { continue ; }	// すでにデータある

		CObjectModel::FrameData *pPrev = pModel->getFrameDataFromPrevFrame(objID, layerID, frame, true) ;
		if ( !pPrev ) { continue ; }
		CObjectModel::FrameData *pNext = pModel->getFrameDataFromNextFrame(objID, layerID, frame) ;
		CObjectModel::FrameData data = pPrev->getInterpolation(pNext, frame) ;

		slot_addNewFrameData(objID, layerID, frame, data) ;	// フレームデータ追加
	}
	m_pEditData->updateSelectData();
}

// キー押しイベント
void AnimationForm::keyPressEvent(QKeyEvent *event)
{
	if ( event->key() == Qt::Key_Control ) {
		m_pGlWidget->setPressCtrl(true) ;
		m_pGlWidget->update() ;
	}

	if ( m_pGlWidget->getPressCtrl() ) {
		if ( event->key() == Qt::Key_C ) {	// copy
			copyFrameData() ;
		}
		if ( event->key() == Qt::Key_V ) {	// paste
			pasteFrameData() ;
		}
	}
}

// キー離しイベント
void AnimationForm::keyReleaseEvent(QKeyEvent *event)
{
	if ( event->key() == Qt::Key_Control ) {
		m_pGlWidget->setPressCtrl(false) ;
		m_pGlWidget->update() ;
	}
}

// フレームデータ コピー
void AnimationForm::copyFrameData( void )
{
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	CObjectModel::typeID layerID = m_pEditData->getSelectLayer() ;

	if ( !objID || !layerID ) {
		return ;
	}

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	int frame = m_pEditData->getSelectFrame() ;
	CObjectModel::FrameData data ;
	CObjectModel::FrameData *pData = pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
	if ( pData ) {
		data = *pData ;
	}
	else {
		pData = pModel->getFrameDataFromPrevFrame(objID, layerID, frame, false) ;
		CObjectModel::FrameData *pNext = pModel->getFrameDataFromNextFrame(objID, layerID, frame) ;
		if ( !pData ) { return ; }
		data = pData->getInterpolation(pNext, frame) ;
	}
	m_pEditData->setCopyFrameData(data);
}

// フレームデータ ペースト
void AnimationForm::pasteFrameData( void )
{
	if ( !m_pEditData->isCopyData() ) {
		return ;
	}

	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	CObjectModel::typeID layerID = m_pEditData->getSelectLayer() ;

	if ( !objID || !layerID ) {
		return ;
	}
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	int frame = m_pEditData->getSelectFrame() ;
	CObjectModel::FrameData *pData = pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
	if ( pData ) {
		QList<CObjectModel::FrameData *> data ;
		*pData = m_pEditData->getCopyFrameData() ;
		data << pData ;
		addCommandEdit( data ) ;
	}
	else {
		slot_addNewFrameData(objID, layerID, frame, m_pEditData->getCopyFrameData()) ;	// フレームデータ追加
	}
}

