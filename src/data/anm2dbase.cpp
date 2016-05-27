#include "anm2dbase.h"
#include <QObject>
#include <QDir>

QString Anm2DBase::getErrorString()
{
    const char *str[] = {
        "エラーはありません",
        "ブロック数が違います",
        "ファイルサイズが違います",
        "バージョンが違います",
        "不正なIDです",
        "キャンセルされました",
        "ノードが不正です",
        "オブジェクト数が不正です",
        "イメージ数が不正です",
        "レイヤ数が不正です",
        "フレームデータ数が不正です",
        "イメージデータが不正です",
        "イメージのファイルパスが不正です",
        "オブジェクト名が不正です",
        "レイヤ名が不正です",
        "登録されていない画像が使用されています"};
    if (m_nError >= 0 && m_nError < kErrorNo_Max)
    {
        return QObject::trUtf8(str[m_nError]);
    }
    return QObject::trUtf8("エラー番号が不正です:%1").arg(m_nError);
}

// srcからdestへの相対パスを取得
QString Anm2DBase::getRelativePath(QString &src, QString &dest)
{
    QString path = src;
    if (path.at(path.count() - 1) != '/')
    {
        path.chop(path.count() - path.lastIndexOf("/") - 1);
    }
    QDir dir(path);
    return dir.relativeFilePath(dest);
}

// srcからdestへの相対パスを絶対パスに変換
QString Anm2DBase::getAbsolutePath(QString &src, QString &dest)
{
    QString path = src;
    if (path.at(path.count() - 1) != '/')
    {
        path.chop(path.count() - path.lastIndexOf("/") - 1);
    }
    QDir dir(path);
    return dir.absoluteFilePath(dest);
}
