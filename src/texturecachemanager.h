#ifndef TEXTURECACHEMANAGER_H
#define TEXTURECACHEMANAGER_H

#include <QList>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QPair>

class TextureCacheManager : protected QOpenGLFunctions
{
public:
    TextureCacheManager();
    virtual ~TextureCacheManager();

    GLuint add(QOpenGLTexture *pTex);
    void remove(GLuint id);
    QOpenGLTexture *get(GLuint id);

private:
    GLuint m_currentId;
    QList<QPair<GLuint, QOpenGLTexture *> > m_caches;
};

#endif // TEXTURECACHEMANAGER_H
