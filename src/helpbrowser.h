#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include <QTextBrowser>
#include <QtHelp/QHelpEngine>

class HelpBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit HelpBrowser(QHelpEngine *pHelpEngine, QWidget *parent = 0);

signals:

public slots:

private:
    QVariant loadResource(int type, const QUrl &name);

private:
    QHelpEngine *m_pEngine;
};

#endif // HELPBROWSER_H
