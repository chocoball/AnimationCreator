#ifndef TEXTURECACHEMANAGER_H
#define TEXTURECACHEMANAGER_H

#include <QList>
#include <QPair>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>

class TextureCacheManager : protected QOpenGLFunctions
{
public:
    TextureCacheManager();
    virtual ~TextureCacheManager();

    GLuint Add(QOpenGLTexture *pTex);
    void Remove(GLuint id);
    QOpenGLTexture *Get(GLuint id);

private:
    GLuint m_currentId;
    QList<QPair<GLuint, QOpenGLTexture *> > m_caches;
};

#endif // TEXTURECACHEMANAGER_H
