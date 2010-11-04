#ifndef SETTING_H
#define SETTING_H

#include <QtGui>

class CSettings
{
public:
	void setCurrentDir( QString &str )		{ mCurrDir = str ; }
	QString &getCurrentDir()				{ return mCurrDir ; }

	void setCurrentSaveDir( QString &str )	{ mCurrSaveDir = str ; }
	QString &getCurrentSaveDir()			{ return mCurrSaveDir ; }

	void setImgWinPos( QPoint &pos )		{ mImgWinPos = pos ; }
	QPoint &getImgWinPos()					{ return mImgWinPos ; }

	void setImgWinSize( QSize &size )		{ mImgWinSize = size ; }
	QSize &getImgWinSize()					{ return mImgWinSize ; }

private:
	QString		mCurrDir ;
	QString		mCurrSaveDir ;
	QPoint		mImgWinPos ;
	QSize		mImgWinSize ;
};

#endif // SETTING_H
