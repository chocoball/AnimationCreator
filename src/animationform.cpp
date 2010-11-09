#include <QStandardItemModel>
#include "defines.h"
#include "animationform.h"
#include "ui_animationform.h"
#include "mainwindow.h"

AnimationForm::AnimationForm(CEditImageData *pImageData, MainWindow *pMainWindow, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnimationForm)
{
	m_pEditImageData = pImageData ;
	m_pMainWindow = pMainWindow ;

	ui->setupUi(this);

	m_pGlWidget = new AnimeGLWidget(pImageData, this) ;
	ui->scrollArea_anime->setWidget(m_pGlWidget);
	m_pGlWidget->resize(1024, 1024);
	m_pGlWidget->setDrawArea(1024, 1024);
	m_pGlWidget->show();

	ui->spinBox_nowSequence->setMaximum(180);
	ui->horizontalSlider_nowSequence->setMaximum(180);
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
	ui->checkBox_grid->setChecked(true);
	ui->comboBox_fps->addItem(tr("60 fps"));
	ui->comboBox_fps->addItem(tr("30 fps"));
	ui->comboBox_fps->addItem(tr("15 fps"));
	ui->comboBox_fps->setCurrentIndex(0);

	for ( int i = 0 ; i < m_pEditImageData->getImageDataSize() ; i ++ ) {
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
		QStandardItemModel *pTreeModel = m_pEditImageData->getTreeModel() ;

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
				m_pEditImageData->setSelectObject(pItem);
			}
		}
	}

	m_pActTreeViewAdd		= new QAction(QString("Add Object"), this);
	m_pActTreeViewDel		= new QAction(QString("Delete"), this);
	m_pActTreeViewLayerDisp = new QAction(QString("Disp"), this) ;

	m_pTimer = new QTimer(this) ;
	m_pTimer->setInterval((int)(100.0f/6.0f));

	m_pDataMarker = new CDataMarkerLabel(m_pEditImageData, this) ;
	m_pDataMarker->setGeometry(ui->horizontalSlider_nowSequence->x()+5,
							   ui->horizontalSlider_nowSequence->y() + ui->horizontalSlider_nowSequence->height(),
							   ui->horizontalSlider_nowSequence->width()-10,
							   8);
	m_pDataMarker->setForegroundRole(QPalette::Highlight);
	m_pDataMarker->show();

	connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),	this, SLOT(slot_treeViewMenuReq(QPoint))) ;
//	connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),			this, SLOT(slot_treeViewDoubleClicked(QModelIndex))) ;
	connect(ui->treeView, SIGNAL(clicked(QModelIndex)),					this, SLOT(slot_changeSelectObject(QModelIndex))) ;

	connect(m_pGlWidget, SIGNAL(sig_dropedImage(QRect, QPoint, int)),
			this,		SLOT(slot_dropedImage(QRect, QPoint, int))) ;
	connect(m_pGlWidget, SIGNAL(sig_selectLayerChanged(CObjectModel::typeID)),
			this,		SLOT(slot_selectLayerChanged(CObjectModel::typeID))) ;
	connect(m_pGlWidget, SIGNAL(sig_dragedImage(CObjectModel::FrameData)),
			this,		SLOT(slot_setUI(CObjectModel::FrameData))) ;
	connect(m_pGlWidget, SIGNAL(sig_deleteFrameData()),
			this,		SLOT(slot_deleteFrameData())) ;
	connect(m_pGlWidget, SIGNAL(sig_selectPrevLayer(CObjectModel::typeID, CObjectModel::typeID, int, CObjectModel::FrameData)),
			this,		SLOT(slot_addNewFrameData(CObjectModel::typeID, CObjectModel::typeID, int, CObjectModel::FrameData))) ;

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
	connect(m_pActTreeViewDel,			SIGNAL(triggered()),			this, SLOT(slot_deleteObject())) ;
	connect(m_pActTreeViewLayerDisp,	SIGNAL(triggered()),			this, SLOT(slot_changeLayerDisp())) ;
	connect(ui->pushButton_play,		SIGNAL(clicked()),				this, SLOT(slot_playAnimation())) ;
	connect(ui->pushButton_stop,		SIGNAL(clicked()),				this, SLOT(slot_stopAnimation())) ;
	connect(ui->checkBox_grid,			SIGNAL(clicked(bool)),			m_pGlWidget, SLOT(slot_setDrawGrid(bool))) ;
	connect(ui->checkBox_uv_anime,		SIGNAL(clicked(bool)),			this, SLOT(slot_changeUVAnime(bool))) ;
	connect(ui->comboBox_fps,			SIGNAL(activated(int)),			this, SLOT(slot_changeAnimeSpeed(int))) ;
	connect(ui->comboBox_image_no,		SIGNAL(activated(int)),			this, SLOT(slot_changeImageIndex(int))) ;
	connect(m_pTimer,					SIGNAL(timeout()),				this, SLOT(slot_timerEvent())) ;

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
	QSize add_w = QSize(add.width(), 0) ;

	if ( event->oldSize().width() < 0 || event->oldSize().height() < 0 ) {
		return ;
	}

	ui->treeView->resize(ui->treeView->size()+add_h);
	ui->scrollArea_anime->resize(ui->scrollArea_anime->size()+add);
	m_pSplitter->resize(m_pSplitter->size()+add);

	ui->comboBox_image_no->move(ui->comboBox_image_no->pos() + QPoint(add.width(), 0));

	QLabel *tmpLabel[] = {
		ui->label_pos,
		ui->label_rot,
		ui->label_center,
		ui->label_scale,
		ui->label_x,
		ui->label_y,
		ui->label_z,
		ui->label_uv,
		ui->label_uv_left,
		ui->label_uv_right,
		ui->label_uv_top,
		ui->label_uv_bottom,
		ui->label_image,
	} ;

	for ( int i = 0 ; i < ARRAY_NUM(tmpLabel) ; i ++ ) {
		tmpLabel[i]->move(tmpLabel[i]->pos() + QPoint(add.width(), 0));
	}

	QSpinBox *tmpBox[] = {
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
	} ;
	for ( int i = 0 ; i < ARRAY_NUM(tmpBox) ; i ++ ) {
		tmpBox[i]->move(tmpBox[i]->pos()+QPoint(add.width(), 0));
	}
	QDoubleSpinBox *tmpBox2[] = {
		ui->doubleSpinBox_scale_x,
		ui->doubleSpinBox_scale_y,
	} ;
	for ( int i = 0 ; i < ARRAY_NUM(tmpBox2) ; i ++ ) {
		tmpBox2[i]->move(tmpBox2[i]->pos()+QPoint(add.width(), 0));
	}

	QCheckBox *tmpCheck[] = {
		ui->checkBox_uv_anime,
	} ;
	for ( int i = 0 ; i < ARRAY_NUM(tmpCheck) ; i ++ ) {
		tmpCheck[i]->move(tmpCheck[i]->pos()+QPoint(add.width(), 0));
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
	if ( !m_pEditImageData->getObjectModel() ) { return ; }

	qDebug("Deump Object ------------------------") ;
	const CObjectModel::ObjectList &objList = m_pEditImageData->getObjectModel()->getObjectList() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		qDebug("Object [%d] ID:%p", i, objList.at(i).first) ;

		const CObjectModel::LayerGroupList &layerGroupList = objList.at(i).second ;
		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			qDebug("  LayerGroup [%d] ID:%p", j, layerGroupList.at(j).first ) ;

			const CObjectModel::FrameDataList &frameDataList = layerGroupList.at(j).second ;
			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				const CObjectModel::FrameData data = frameDataList.at(k) ;
				qDebug("    FrameData [%d] x:%d y:%d z:%d", k, data.pos_x, data.pos_y, data.pos_z) ;
				qDebug("              cx:%d cy:%d frame:%d", data.center_x, data.center_y, data.frame) ;
				qDebug("　　　　　　　　　　　　　　scaX:%f scaY:%f", data.fScaleX, data.fScaleY) ;
				qDebug("　　　　　　　　　　　　　　l:%d r:%d t:%d b:%d", data.left, data.right, data.top, data.bottom) ;
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
	QStandardItem *newItem = m_pEditImageData->cmd_addNewObject(str);
	ui->treeView->setCurrentIndex(newItem->index());
	m_pEditImageData->setSelectObject(newItem);
}

// オブジェクト削除
void AnimationForm::slot_deleteObject( void )
{
//	QStandardItemModel *pTreeModel = m_pEditImageData->getTreeModel() ;
	QModelIndex index = ui->treeView->currentIndex() ;

	if ( !index.isValid() ) { return ; }

	m_pEditImageData->cmd_delObject(index, m_pDataMarker);
	slot_changeSelectObject(ui->treeView->currentIndex());
}

// フレームデータ削除
void AnimationForm::slot_deleteFrameData(void)
{
	CObjectModel *pModel			= m_pEditImageData->getObjectModel() ;
	CObjectModel::typeID objID		= m_pEditImageData->getSelectObject() ;
	CObjectModel::typeID layerID	= m_pEditImageData->getSelectLayer() ;
	int frame						= m_pEditImageData->getSelectFrame() ;
#if 1
	QList<QWidget *> update ;
	update << m_pDataMarker << m_pGlWidget ;
	m_pEditImageData->cmd_delFrameData(objID, layerID, frame, update) ;

	CObjectModel::FrameDataList *p = pModel->getFrameDataListFromID(objID, layerID) ;
	if ( !p || p->size() == 0 ) {
		QStandardItem *item = (QStandardItem *)layerID ;
		m_pEditImageData->cmd_delObject(item->index(), m_pDataMarker);
		slot_changeSelectObject(ui->treeView->currentIndex());
	}
#else
	if ( pModel->delLayerData(objID, layerID, frame) ) {
		CObjectModel::FrameDataList *p = pModel->getFrameDataListFromID(objID, layerID) ;
		if ( !p || p->size() == 0 ) {
			QStandardItem *item = (QStandardItem *)layerID ;
			m_pEditImageData->getTreeModel()->removeRow(item->index().row(), item->parent()->index()) ;
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
	CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
	int frameNum  = ui->horizontalSlider_nowSequence->value() ;
	QModelIndex index = ui->treeView->currentIndex() ;

	qDebug() << "AnimationForm::slot_dropedImage" ;
	qDebug("row:%d col:%d ptr:%p root:%p", index.row(), index.column(), index.internalPointer(), m_pEditImageData->getTreeModel()->invisibleRootItem()) ;

	if ( !index.isValid() ) {
		qWarning() << "slot_dropedImage current index invalid 0" ;
		return ;
	}

	while ( index.internalPointer() != m_pEditImageData->getTreeModel()->invisibleRootItem() ) {
		index = index.parent() ;
		if ( !index.isValid() ) {
			qWarning() << "slot_dropedImage current index invalid 1" ;
			return ;
		}
	}

	QStandardItem *pParentItem = m_pEditImageData->getTreeModel()->itemFromIndex(index) ;
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

	QList<QWidget *> updateWidget ;
	updateWidget << m_pGlWidget ;
	updateWidget << m_pDataMarker ;
	m_pEditImageData->cmd_addNewLayer(index, newItem, frameData, updateWidget) ;

	m_pEditImageData->setSelectLayer(newItem);
	slot_setUI(frameData);
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

	m_pEditImageData->setSelectLayer(newItem);
	slot_setUI(frameData);

	m_pGlWidget->update();
	m_pDataMarker->update();
#endif
}

// 現在フレーム変更
void AnimationForm::slot_frameChanged(int frame)
{
	bool bChange = (m_pEditImageData->getSelectFrame() != frame) ;
	m_pEditImageData->setSelectFrame( frame ) ;
	if ( bChange ) {
		CObjectModel::FrameData *pData = getNowSelectFrameData() ;
		if ( pData ) {
			slot_setUI(*pData);
		}
	}
	m_pGlWidget->update();
}

// 選択レイヤ変更
void AnimationForm::slot_selectLayerChanged( CObjectModel::typeID layerID )
{
	m_pEditImageData->setSelectLayer(layerID);

	QStandardItem *item = (QStandardItem *)layerID ;
	if ( item ) {
		ui->treeView->setCurrentIndex(item->index());
	}

	m_pGlWidget->update();
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( pData ) {
		slot_setUI(*pData);
	}
}

// ＵＩ数値セット
void AnimationForm::slot_setUI(CObjectModel::FrameData data)
{
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

	if ( data.getRect() != m_pEditImageData->getCatchRect() ) {
		QRect rect = data.getRect() ;
		m_pEditImageData->setCatchRect(rect);
		emit sig_imageRepaint() ;
	}
}

// pos x 変更
void AnimationForm::slot_changePosX( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->pos_x == val ) { return ; }

	pData->pos_x = val ;
	addCommandEdit(pData) ;
}

// pos y 変更
void AnimationForm::slot_changePosY( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->pos_y == val ) { return ; }

	pData->pos_y = val ;
	addCommandEdit(pData) ;
}

// pos z 変更
void AnimationForm::slot_changePosZ( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->pos_z == val ) { return ; }

	pData->pos_z = val ;
	addCommandEdit(pData) ;
}

// rot x 変更
void AnimationForm::slot_changeRotX( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->rot_x == val ) { return ; }

	pData->rot_x = val ;
	addCommandEdit(pData) ;
}

// rot y 変更
void AnimationForm::slot_changeRotY( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->rot_y == val ) { return ; }

	pData->rot_y = val ;
	addCommandEdit(pData) ;
}

// rot z 変更
void AnimationForm::slot_changeRotZ( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->rot_z == val ) { return ; }

	pData->rot_z = val ;
	addCommandEdit(pData) ;
}

// scale x 変更
void AnimationForm::slot_changeScaleX( double val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->fScaleX == val ) { return ; }

	pData->fScaleX = val ;
	addCommandEdit(pData) ;
}

// scale y 変更
void AnimationForm::slot_changeScaleY( double val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->fScaleY == val ) { return ; }

	pData->fScaleY = val ;
	addCommandEdit(pData) ;
}

// uv left 変更
void AnimationForm::slot_changeUvLeft( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->left == val ) { return ; }

	pData->left = val ;
	addCommandEdit(pData) ;

	QRect rect = pData->getRect() ;
	m_pEditImageData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// uv right 変更
void AnimationForm::slot_changeUvRight( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->right == val ) { return ; }

	pData->right = val ;
	addCommandEdit(pData) ;

	QRect rect = pData->getRect() ;
	m_pEditImageData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// uv top 変更
void AnimationForm::slot_changeUvTop( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->top == val ) { return ; }

	pData->top = val ;
	addCommandEdit(pData) ;

	QRect rect = pData->getRect() ;
	m_pEditImageData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// uv bottom 変更
void AnimationForm::slot_changeUvBottom( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->bottom == val ) { return ; }

	pData->bottom = val ;
	addCommandEdit(pData) ;

	QRect rect = pData->getRect() ;
	m_pEditImageData->setCatchRect(rect);
	emit sig_imageRepaint() ;
}

// center x 変更
void AnimationForm::slot_changeCenterX( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->center_x == val ) { return ; }

	pData->center_x = val ;

	addCommandEdit(pData) ;
}

// center y 変更
void AnimationForm::slot_changeCenterY( int val )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) { return ; }
//	if ( pData->center_y == val ) { return ; }

	pData->center_y = val ;

	addCommandEdit(pData) ;
}

// ツリービュー メニューリクエスト
void AnimationForm::slot_treeViewMenuReq(QPoint treeViewLocalPos)
{
	QMenu menu(this) ;
	menu.addAction(m_pActTreeViewAdd) ;
	menu.addAction(m_pActTreeViewDel) ;
	// レイヤ選択中だったら
	if ( ui->treeView->currentIndex().internalPointer() != m_pEditImageData->getTreeModel()->invisibleRootItem() ) {
		menu.addAction(m_pActTreeViewLayerDisp) ;
	}
	menu.exec(ui->treeView->mapToGlobal(treeViewLocalPos) + QPoint(0, ui->treeView->header()->height())) ;
}

// ツリービュー ダブルクリック
void AnimationForm::slot_treeViewDoubleClicked(QModelIndex index)
{
	QStandardItemModel *pTreeModel = m_pEditImageData->getTreeModel() ;
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
	m_pEditImageData->setSelectObject(0);
	m_pEditImageData->setSelectLayer(0);

	if ( !index.isValid() ) { return ; }

	if ( index.internalPointer() == m_pEditImageData->getTreeModel()->invisibleRootItem() ) {	// オブジェクト選択
		qDebug("select Object:%d", index.row()) ;
		m_pEditImageData->setSelectObject(m_pEditImageData->getTreeModel()->itemFromIndex(index));
	}
	else {		// レイヤ選択
		qDebug("select Layer:%d parent:%d", index.row(), index.parent().row()) ;
		m_pEditImageData->setSelectObject(m_pEditImageData->getTreeModel()->itemFromIndex(index.parent()));
		m_pEditImageData->setSelectLayer(m_pEditImageData->getTreeModel()->itemFromIndex(index));

		CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
		CObjectModel::FrameData *pData = pModel->getFrameDataFromIDAndFrame(m_pEditImageData->getSelectObject(),
																			m_pEditImageData->getSelectLayer(),
																			m_pEditImageData->getSelectFrame()) ;
		if ( pData ) {
			slot_setUI(*pData);
		}
	}

	m_pGlWidget->update();
}

// アニメ再生
void AnimationForm::slot_playAnimation( void )
{
	if ( !m_pEditImageData->isPauseAnime() ) {	// ポーズ中じゃなかったら０フレームから再生
		ui->horizontalSlider_nowSequence->setValue(0);
	}

	m_pEditImageData->setPlayAnime(true) ;

	// 最大フレーム設定
	m_nMaxFrameNum = m_pEditImageData->getObjectModel()->getMaxFrame() ;

	m_pEditImageData->setPauseAnime(false);
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
	m_pEditImageData->setPlayAnime(false) ;
	m_pEditImageData->setPauseAnime(true);
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

	m_pEditImageData->setPlayAnime(false) ;
	m_pEditImageData->setPauseAnime(false);
	ui->horizontalSlider_nowSequence->setValue(0);

	QIcon icon;
	icon.addFile(QString::fromUtf8(":/root/Resources/images/Button Play_32.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->pushButton_play->setIcon(icon);

	m_pTimer->stop();			// タイマー停止
	m_pGlWidget->update();
}

// タイマイベント。フレームを進める
void AnimationForm::slot_timerEvent( void )
{
	int frame = ui->horizontalSlider_nowSequence->value() ;
	frame ++ ;

	if ( frame > m_nMaxFrameNum ) {
		frame = 0 ;
	}

	ui->horizontalSlider_nowSequence->setValue(frame);
}

// フレームデータ追加
void AnimationForm::slot_addNewFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, CObjectModel::FrameData data )
{
	CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
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
	m_pEditImageData->cmd_addNewFrameData(objID, layerID, data, update) ;
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
	if ( index.internalPointer() == m_pEditImageData->getTreeModel()->invisibleRootItem() ) { return ; }	// オブジェクト選択中

	slot_treeViewDoubleClicked(index) ;
}

// 選択中レイヤのUV変更
void AnimationForm::slot_changeSelectLayerUV( QRect rect )
{
	CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
	CObjectModel::typeID objID = m_pEditImageData->getSelectObject() ;
	CObjectModel::typeID layerID = m_pEditImageData->getSelectLayer() ;
	int frame = m_pEditImageData->getSelectFrame() ;

	CObjectModel::FrameData *pData = pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
	if ( !pData ) {
		return ;
	}
	pData->setRect(rect) ;
	addCommandEdit(pData) ;

	slot_setUI(*pData);
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

	if ( !m_pEditImageData->getTexObj(imageNo) ) {
		GLuint obj = m_pGlWidget->bindTexture(m_pEditImageData->getImage(imageNo)) ;
		m_pEditImageData->setTexObj(imageNo, obj);
	}
}

// イメージ削除
void AnimationForm::slot_delImage( int imageNo )
{
	ui->comboBox_image_no->removeItem(imageNo);
	for ( int i = 0 ; i < ui->comboBox_image_no->count() ; i ++ ) {
		ui->comboBox_image_no->setItemText(i, tr("%1").arg(i));
	}

	m_pEditImageData->removeImageData(imageNo) ;
	m_pGlWidget->update();
}

// イメージ番号変更
void AnimationForm::slot_changeImageIndex(int index)
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) {
		return ;
	}
	pData->nImage = index ;
	m_pGlWidget->update();
}

// UVアニメON/OFF
void AnimationForm::slot_changeUVAnime( bool flag )
{
	CObjectModel::FrameData *pData = getNowSelectFrameData() ;
	if ( !pData ) {
		return ;
	}
	pData->bUVAnime = flag ;
}

// イメージ更新
void AnimationForm::slot_modifiedImage(int index)
{
	QImage &image = m_pEditImageData->getImage(index) ;
	if ( m_pEditImageData->getTexObj(index) ) {
		m_pGlWidget->deleteTexture(m_pEditImageData->getTexObj(index)) ;
	}
	GLuint obj = m_pGlWidget->bindTexture(image) ;
	m_pEditImageData->setTexObj(index, obj);
	m_pGlWidget->update();
}

// オプションダイアログ終了時
void AnimationForm::slot_endedOption( void )
{
	m_pGlWidget->update();
}

// 現在選択しているフレームデータ取得
CObjectModel::FrameData *AnimationForm::getNowSelectFrameData( void )
{
	CObjectModel::typeID	objID	= m_pEditImageData->getSelectObject() ;
	CObjectModel::typeID	layerID	= m_pEditImageData->getSelectLayer() ;
	int						frame	= m_pEditImageData->getSelectFrame() ;

	if ( !objID )		{ return NULL ; }
	if ( !layerID )		{ return NULL ; }
	if ( frame < 0 )	{ return NULL ; }

	CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
	return pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
}

// オブジェクト追加
void AnimationForm::addNewObject( QString str )
{
	if ( str.isEmpty() ) { return ; }

	QStandardItem *newItem = new QStandardItem(str) ;
	QStandardItem *item = m_pEditImageData->getTreeModel()->invisibleRootItem() ;
	item->appendRow(newItem) ;
	qDebug("addNewObject row:%d col:%d ptr:%p root:%p", newItem->index().row(), newItem->index().column(), newItem->index().internalPointer(), item) ;

	CObjectModel::ObjectGroup obj = qMakePair( newItem, CObjectModel::LayerGroupList()) ;
	m_pEditImageData->getObjectModel()->addObject(obj) ;

	qDebug("newItem valid:%d row:%d col:%d", newItem->index().isValid(), newItem->row(), newItem->column()) ;
	ui->treeView->setCurrentIndex(newItem->index());
	m_pEditImageData->setSelectObject(newItem);
}

// フレームデータ編集コマンド
void AnimationForm::addCommandEdit( CObjectModel::FrameData *pData )
{
	CObjectModel::typeID	objID	= m_pEditImageData->getSelectObject() ;
	CObjectModel::typeID	layerID	= m_pEditImageData->getSelectLayer() ;
	int						frame	= m_pEditImageData->getSelectFrame() ;
	QList<QWidget *> update ;

	update << m_pGlWidget ;
	m_pEditImageData->cmd_editFrameData(objID, layerID, frame, *pData, update);
}

