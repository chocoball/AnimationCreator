#include "framedatascaleform.h"
#include "ui_framedatascaleform.h"

FrameDataScaleForm::FrameDataScaleForm(CEditData *pEditData, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FrameDataScaleForm)
{
    ui->setupUi(this);
    m_pEditData = pEditData;

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(slot_clickedExec()));
}

FrameDataScaleForm::~FrameDataScaleForm()
{
    delete ui;
}

void FrameDataScaleForm::slot_clickedExec()
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    ObjectItem *pItem = pModel->getObject(index);
    if (!pItem)
    {
        QMessageBox::warning(this, trUtf8("エラー"), trUtf8("オブジェクトが選択されていません"));
        return;
    }

    double scale = ui->doubleSpinBox->value();
    int ret;
    if (scale - (int)scale > 0)
    {
        ret = QMessageBox::question(this, trUtf8("確認"), trUtf8("フレームデータを%2倍しますか？少数が含まれています。自己責任でお願いします！").arg(scale), QMessageBox::Ok, QMessageBox::Cancel);
    }
    else
    {
        ret = QMessageBox::question(this, trUtf8("確認"), trUtf8("フレームデータを%2倍しますか？").arg(scale), QMessageBox::Ok, QMessageBox::Cancel);
    }
    if (ret == QMessageBox::Ok)
    {
        m_pEditData->cmd_changeFrameDataScale(scale);
    }
}
