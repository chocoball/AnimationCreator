#include "colorpickerform.h"
#include "ui_colorpickerform.h"

ColorPickerForm::ColorPickerForm(CEditData *pEditData, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ColorPickerForm)
{
    ui->setupUi(this);
    m_pEditData = pEditData;

    m_selectColor = 0;
    m_selectBright = QPoint();
    m_editMode = 0;

    setArrowPos();
    makeColor();
    makeBright();
    ui->label_bright->setSelectPos(m_selectBright);
}

ColorPickerForm::~ColorPickerForm()
{
    delete ui;
}

void ColorPickerForm::mousePressEvent(QMouseEvent *event)
{
    qDebug() << event->pos();
    if (event->button() == Qt::LeftButton)
    {
        QRect rect;
        rect = QRect(ui->label_color->x(), ui->label_color->y(), ui->label_color->width(), ui->label_color->height());
        if (rect.contains(event->pos()))
        {
            m_selectColor = event->pos().y() - ui->label_color->y();
            setArrowPos();
            makeBright();
            setColorToFrameData();
            m_editMode = 1;
        }

        rect = QRect(ui->label_bright->x(), ui->label_bright->y(), ui->label_bright->width(), ui->label_bright->height());
        if (rect.contains(event->pos()))
        {
            m_selectBright = QPoint(event->pos() - ui->label_bright->pos());
            ui->label_bright->setSelectPos(m_selectBright);
            setColorToFrameData();
            repaint();
            m_editMode = 2;
        }
    }
}

void ColorPickerForm::mouseMoveEvent(QMouseEvent *event)
{
    if (m_editMode == 1)
    {
        QRect rect = QRect(ui->label_color->x(), ui->label_color->y(), ui->label_color->width(), ui->label_color->height());
        if (!rect.contains(event->pos()))
        {
            return;
        }

        m_selectColor = event->pos().y() - ui->label_color->y();
        setArrowPos();
        makeBright();
        setColorToFrameData();
    }
    else if (m_editMode == 2)
    {
        QRect rect = QRect(ui->label_bright->x(), ui->label_bright->y(), ui->label_bright->width(), ui->label_bright->height());
        if (!rect.contains(event->pos()))
        {
            return;
        }

        m_selectBright = QPoint(event->pos() - ui->label_bright->pos());
        ui->label_bright->setSelectPos(m_selectBright);
        setColorToFrameData();
        repaint();
    }
}

void ColorPickerForm::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    m_editMode = 0;
}

void ColorPickerForm::paintEvent(QPaintEvent * /*event*/)
{
}

void ColorPickerForm::makeColor()
{
    QSize size = ui->label_color->size();
    QImage img = QImage(size, QImage::Format_ARGB32);

    int div = size.height() / 6;
    for (int i = 0; i < size.height(); i++)
    {
        QColor col = QColor(0, 0, 0);
        switch (i / div)
        {
            case 0:
                col.setRedF(1);
                col.setBlueF(1.0 - (float)(div - (i - div * 0)) / div);
                break;
            case 1:
                col.setBlueF(1);
                col.setRedF((float)(div - (i - div * 1)) / div);
                break;
            case 2:
                col.setBlueF(1);
                col.setGreenF(1.0 - (float)(div - (i - div * 2)) / div);
                break;
            case 3:
                col.setGreenF(1);
                col.setBlueF((float)(div - (i - div * 3)) / div);
                break;
            case 4:
                col.setGreenF(1);
                col.setRedF(1.0 - (float)(div - (i - div * 4)) / div);
                break;
            case 5:
                col.setRedF(1);
                col.setGreenF((float)(div - (i - div * 5)) / div);
                break;
        }
        for (int j = 0; j < size.width(); j++)
        {
            img.setPixel(j, i, col.rgba());
        }
    }
    ui->label_color->setPixmap(QPixmap::fromImage(img));
}

void ColorPickerForm::makeBright()
{
    QSize size = ui->label_bright->size();
    QImage image = QImage(size, QImage::Format_ARGB32);

    const QPixmap *pixColor = ui->label_color->pixmap();
    if (!pixColor)
    {
        return;
    }
    QImage imgColor = pixColor->toImage();
    QColor col = QColor(imgColor.pixel(0, m_selectColor >= imgColor.height() ? imgColor.height() - 1 : m_selectColor));

    for (int y = 0; y < size.height(); y++)
    {
        for (int x = 0; x < size.width(); x++)
        {
            QColor c, col_x, col_y;

            float per_x = (float)x / (size.width() - 1);
            float per_y = (float)y / (size.height() - 1);

            col_x.setRedF(1.0 - per_x * (1 - col.redF()));
            col_x.setGreenF(1.0 - per_x * (1 - col.greenF()));
            col_x.setBlueF(1.0 - per_x * (1 - col.blueF()));

            col_y.setRedF(1.0 - per_y);
            col_y.setGreenF(1.0 - per_y);
            col_y.setBlueF(1.0 - per_y);

            c.setRedF(col_x.redF() * col_y.redF());
            c.setGreenF(col_x.greenF() * col_y.greenF());
            c.setBlueF(col_x.blueF() * col_y.blueF());

            image.setPixel(x, y, c.rgba());
        }
    }

    ui->label_bright->setPixmap(QPixmap::fromImage(image));
}

void ColorPickerForm::setArrowPos()
{
    ui->label_arrow->move(ui->label_color->x() - ui->label_arrow->fontInfo().pixelSize(), ui->label_color->y() + m_selectColor - ui->label_arrow->height() / 2);
}

void ColorPickerForm::setColorToFrameData()
{
    const QPixmap *pix = ui->label_bright->pixmap();
    if (!pix)
    {
        return;
    }
    QImage image = pix->toImage();

    int x = m_selectBright.x() >= image.width() ? image.width() - 1 : m_selectBright.x();
    int y = m_selectBright.y() >= image.height() ? image.height() - 1 : m_selectBright.y();

    emit sig_setColorToFrameData(image.pixel(x, y));
}
