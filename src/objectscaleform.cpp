#include "objectscaleform.h"
#include "ui_objectscaleform.h"

ObjectScaleForm::ObjectScaleForm(EditData *pEditData, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ObjectScaleForm)
{
    ui->setupUi(this);
    m_pEditData = pEditData;

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(slot_clickedExec()));
}

ObjectScaleForm::~ObjectScaleForm()
{
    delete ui;
}

void ObjectScaleForm::slot_clickedExec()
{
    double scale = ui->doubleSpinBox->value();
    int ret = QMessageBox::question(this, trUtf8("確認"), trUtf8("オブジェクトを%2倍しますか？").arg(scale), QMessageBox::Ok, QMessageBox::Cancel);
    if (ret == QMessageBox::Ok)
    {
        m_pEditData->cmd_changeUvScale(scale);
    }
}
