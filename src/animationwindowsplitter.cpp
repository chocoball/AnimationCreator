#include "animationwindowsplitter.h"

AnimationWindowSplitter::AnimationWindowSplitter(QWidget *parent)
    : QSplitter(parent)
{
}

AnimationWindowSplitter::~AnimationWindowSplitter()
{
}

void AnimationWindowSplitter::moveSplitter(int pos, int index)
{
    //	qDebug("MoveSplitter pos:%d index:%d", pos, index) ;

    if (index < 0 || pos < 0)
    {
        return;
    }
    QSplitter::moveSplitter(pos, index);
}

