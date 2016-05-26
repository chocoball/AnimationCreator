#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include "helpbrowser.h"
#include <QSplitter>
#include <QWidget>
#include <QtGui>

class HelpWindow : public QWidget
{
    Q_OBJECT
public:
    explicit HelpWindow(QWidget *parent = 0);

    bool isLoaded() { return (m_pEngine) ? true : false; }

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QHelpEngine *m_pEngine;
    HelpBrowser *m_pBrowser;
    QSplitter *m_pSplitter;
};

#endif // HELPWINDOW_H
