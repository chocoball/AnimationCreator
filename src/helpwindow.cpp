#include <QtHelp/QHelpContentWidget>
#include "helpwindow.h"


HelpWindow::HelpWindow(QWidget *parent)
	: QWidget(parent)
{
	QString helpPath = qApp->applicationDirPath()+"/help/AnimationCreator.qhc" ;
	m_pEngine = new QHelpEngine(helpPath, this) ;
	if ( !m_pEngine->setupData() ) {
		QMessageBox::warning(this, trUtf8("エラー"), trUtf8("ヘルプファイルを開けません:%1").arg(helpPath)) ;

		delete m_pEngine ;
		m_pEngine = NULL ;
		return ;
	}
	m_pBrowser = new HelpBrowser(m_pEngine, this) ;

	QHelpContentWidget *pHelpWidget = m_pEngine->contentWidget() ;

	QSplitter *pSplitter = new QSplitter(Qt::Horizontal, this) ;
	pSplitter->addWidget(pHelpWidget);
	pSplitter->addWidget(m_pBrowser);

	QGridLayout *pLayout = new QGridLayout(this) ;
	pLayout->addWidget(pSplitter);

	setObjectName("HelpWindow");
	setWindowTitle("Animation Creator Help");

	connect(pHelpWidget, SIGNAL(linkActivated(const QUrl &)), m_pBrowser, SLOT(setSource(QUrl))) ;
	show() ;
}

