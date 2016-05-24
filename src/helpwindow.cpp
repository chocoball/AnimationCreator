#include <QtHelp/QHelpContentWidget>
#include <QMessageBox>
#include <QGridLayout>
#include "helpwindow.h"


HelpWindow::HelpWindow(QWidget *parent)
	: QWidget(parent)
{
	QString helpPath = qApp->applicationDirPath()+"/help/AnimationCreator.qhc" ;
	m_pEngine = new QHelpEngine(helpPath, this) ;
	if ( !m_pEngine->setupData() ) {
		QMessageBox::warning(this, trUtf8("エラー:%1").arg(m_pEngine->error()), trUtf8("ヘルプファイルを開けません:%1").arg(helpPath)) ;

		delete m_pEngine ;
		m_pEngine = NULL ;
		return ;
	}
	m_pBrowser = new HelpBrowser(m_pEngine, this) ;

	QHelpContentWidget *pHelpWidget = m_pEngine->contentWidget() ;

	m_pSplitter = new QSplitter(Qt::Horizontal, this) ;
	m_pSplitter->addWidget(pHelpWidget);
	m_pSplitter->addWidget(m_pBrowser);

	QGridLayout *pLayout = new QGridLayout(this) ;
	pLayout->addWidget(m_pSplitter);

	setObjectName("HelpWindow");
	setWindowTitle("Animation Creator Help");

	connect(pHelpWidget, SIGNAL(linkActivated(const QUrl &)), m_pBrowser, SLOT(setSource(QUrl))) ;
	show() ;
}

void HelpWindow::resizeEvent(QResizeEvent *event)
{
	QSize sub = event->size() - event->oldSize() ;

}
