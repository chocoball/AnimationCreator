#include "helpbrowser.h"
#include <QSplitter>
#include <QtGui>

HelpBrowser::HelpBrowser(QHelpEngine *pHelpEngine, QWidget *parent)
    : QTextBrowser(parent)
{
    m_pEngine = pHelpEngine;
}

QVariant HelpBrowser::loadResource(int type, const QUrl &name)
{
    if (!m_pEngine)
    {
        return QVariant();
    }
    if (name.scheme() == "qthelp")
    {
        //		return trUtf8(m_pEngine->fileData(name)) ;
        return m_pEngine->fileData(name);
    }
    else
    {
        return QTextBrowser::loadResource(type, name);
    }
}
