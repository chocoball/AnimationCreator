#include "AnimationWindowSplitter.h"

AnimationWindowSplitter::AnimationWindowSplitter(QWidget *parent) :
	QSplitter(parent)
{
}

AnimationWindowSplitter::~AnimationWindowSplitter()
{
}

void AnimationWindowSplitter::MoveSplitter(int pos, int index)
{
//	qDebug("MoveSplitter pos:%d index:%d", pos, index) ;

	if ( index < 0 || pos < 0 ) { return ; }
	moveSplitter(pos, index) ;
}
