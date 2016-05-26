#ifndef ANIMATIONWINDOWSPLITTER_H
#define ANIMATIONWINDOWSPLITTER_H

#include <QSplitter>

class AnimationWindowSplitter : public QSplitter
{
public:
    AnimationWindowSplitter(QWidget *parent = 0);
    ~AnimationWindowSplitter();

    void moveSplitter(int pos, int index);
};

#endif // ANIMATIONWINDOWSPLITTER_H
