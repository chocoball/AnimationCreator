#include "loupewindow.h"
#include "mainwindow.h"
#include <QComboBox>
#include <QGridLayout>

LoupeWindow::LoupeWindow(EditData *pEditData, Settings *pSetting, QWidget *parent)
    : QWidget(parent)
{
    m_pEditData = pEditData;
    m_pSetting = pSetting;

    QLabel *pLabelScale = new QLabel(trUtf8("倍率"), this);

    QComboBox *pComboBox = new QComboBox(this);
    pComboBox->addItem(tr("2"));
    pComboBox->addItem(tr("4"));
    pComboBox->addItem(tr("8"));
    pComboBox->addItem(tr("16"));

    m_pLabel = new QLabel(this);
    m_pLabel->setGeometry(QRect(0, 0, 200, 200));
    m_pLabel->setScaledContents(true);
    m_pLabel->setMinimumSize(200, 200);

    //	m_pLabelCheck = new QLabel(this) ;
    //	m_pLabelCheck->setText(trUtf8("カーソルの移動にあわせる"));

    m_pCheckBox_Cursor = new QCheckBox(this);
    m_pCheckBox_Cursor->setChecked(true);
    m_pCheckBox_Cursor->setText(trUtf8("カーソルの移動にあわせる"));

    m_pCheckBox_Center = new QCheckBox(this);
    m_pCheckBox_Center->setChecked(true);
    m_pCheckBox_Center->setText(trUtf8("カーソル位置表示"));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(m_pCheckBox_Cursor, 0, 0);
    layout->addWidget(m_pCheckBox_Center, 0, 1);
    layout->addWidget(pLabelScale, 1, 0);
    layout->addWidget(pComboBox, 1, 1);
    layout->addWidget(m_pLabel, 2, 0, 2, 2);
    setLayout(layout);

    m_pTimer = new QTimer(this);
    m_pTimer->setInterval(100 / 3);
    m_pTimer->start();

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_cursorScreenShort()));
    connect(pComboBox, SIGNAL(activated(QString)), this, SLOT(slot_changeScale(QString)));

    m_Scale = 2;

    setWindowTitle(trUtf8("ルーペ"));

    m_CenterPos = QApplication::desktop()->cursor().pos();
}

bool LoupeWindow::keyPress(QKeyEvent *event)
{
    int key = event->key();
    if (event->modifiers() & Qt::ShiftModifier)
    {
        key |= Qt::SHIFT;
    }
    if (event->modifiers() & Qt::ControlModifier)
    {
        key |= Qt::CTRL;
    }
    if (event->modifiers() & Qt::MetaModifier)
    {
        key |= Qt::META;
    }
    if (event->modifiers() & Qt::AltModifier)
    {
        key |= Qt::ALT;
    }
    QKeySequence ks(key);

    if (ks == m_pSetting->getShortcutLockLoupe())
    {
        toggleLock();
        return true;
    }
    return false;
}

void LoupeWindow::slot_cursorScreenShort()
{
    if (m_pEditData->getDraggingImage())
    {
        return;
    }

    if (m_pCheckBox_Cursor->isChecked())
    {
        m_CenterPos = QApplication::desktop()->cursor().pos();
    }

    QSize size = m_pLabel->size();
    fixImage(size);
}

void LoupeWindow::slot_changeScale(QString str)
{
    bool bOk;
    int val = str.toInt(&bOk);
    if (!bOk)
    {
        return;
    }

    m_Scale = val;
}

void LoupeWindow::resizeEvent(QResizeEvent * /*event*/)
{
    QSize size = m_pLabel->size();
    fixImage(size);
}

void LoupeWindow::fixImage(QSize &size)
{
    int imgWidth = size.width();
    int imgHeight = size.height();
    if (imgWidth < 0 || imgHeight < 0)
    {
        return;
    }

    QPoint pos = m_CenterPos;
    int width = imgWidth / m_Scale;
    int height = imgHeight / m_Scale;
    int x = pos.x() - width / 2;
    int y = pos.y() - height / 2;
    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap pix = screen->grabWindow(QApplication::desktop()->winId(),
                                     x,
                                     y,
                                     width,
                                     height);
    QImage image = pix.toImage();
    int i, j;
    QPoint cursorPos = QPoint(0, 0);
    if (!m_pCheckBox_Cursor->isChecked())
    {
        cursorPos = pos - QApplication::desktop()->cursor().pos();
    }

    // デスクトップ範囲外を黒に。
    for (i = x; i < 0; i++)
    {
        for (j = 0; j < height; j++)
        {
            int xx = i - x;
            if (xx >= 0 && xx < width && j >= 0 && j < height)
            {
                image.setPixel(xx, j, 0);
            }
        }
    }
    for (i = y; i < 0; i++)
    {
        for (j = 0; j < width; j++)
        {
            int yy = i - y;
            if (j >= 0 && j < width && yy >= 0 && yy < height)
            {
                image.setPixel(j, yy, 0);
            }
        }
    }
    if (QApplication::desktop()->width() > 0)
    {
        for (i = x + width; i > QApplication::desktop()->width(); i--)
        {
            for (j = 0; j < height; j++)
            {
                int xx = width - (i - QApplication::desktop()->width());
                if (xx >= 0 && xx < width && j >= 0 && j < height)
                {
                    image.setPixel(xx, j, 0);
                }
            }
        }
    }
    if (QApplication::desktop()->height() > 0)
    {
        for (i = y + height; i > QApplication::desktop()->height(); i--)
        {
            for (j = 0; j < width; j++)
            {
                int yy = height - (i - QApplication::desktop()->height());
                if (j >= 0 && j < width && yy >= 0 && yy < height)
                {
                    image.setPixel(j, yy, 0);
                }
            }
        }
    }

    // 中心
    if (m_pCheckBox_Center->isChecked())
    {
        for (i = width / 2 - 3 - cursorPos.x(); i <= width / 2 + 3 - cursorPos.x(); i++)
        {
            j = height / 2 - cursorPos.y();
            if (i >= 0 && i < width && j >= 0 && j < height)
            {
                image.setPixel(i, j, QColor(255, 0, 0).rgba());
            }
        }
        for (i = height / 2 - 3 - cursorPos.y(); i <= height / 2 + 3 - cursorPos.y(); i++)
        {
            j = width / 2 - cursorPos.x();
            if (i >= 0 && i < height && j >= 0 && j < width)
            {
                image.setPixel(j, i, QColor(255, 0, 0).rgba());
            }
        }
    }

    image = image.scaled(imgWidth, imgHeight);
    pix = QPixmap::fromImage(image);
    m_pLabel->setPixmap(pix);
}

void LoupeWindow::toggleLock(void)
{
    m_pCheckBox_Cursor->setChecked(!m_pCheckBox_Cursor->isChecked());
}
