#ifndef ANM2DBASE_H
#define ANM2DBASE_H

#include <QString>

class Anm2DBase
{
public:
    enum
    {
        kErrorNo_NoError = 0,         ///< [ 0]エラーなし
        kErrorNo_BlockNumNotSame,     ///< [ 1]ブロック数が違う
        kErrorNo_InvalidFileSize,     ///< [ 2]ファイルサイズが違う
        kErrorNo_InvalidVersion,      ///< [ 3]バージョンが違う
        kErrorNo_InvalidID,           ///< [ 4]不正なID
        kErrorNo_Cancel,              ///< [ 5]キャンセルされた
        kErrorNo_InvalidNode,         ///< [ 6]ノードが不正
        kErrorNo_InvalidObjNum,       ///< [ 7]オブジェ数が不正
        kErrorNo_InvalidImageNum,     ///< [ 8]イメージ数が不正
        kErrorNo_InvalidLayerNum,     ///< [ 9]レイヤ数が不正
        kErrorNo_InvalidFrameDataNum, ///< [10]フレームデータ数が不正
        kErrorNo_InvalidImageData,    ///< [11]イメージデータが不正
        kErrorNo_InvalidFilePath,     ///< [12]イメージのファイルパスが不正
        kErrorNo_InvalidObjectName,   ///< [13]オブジェクト名が不正
        kErrorNo_InvalidLayerName,    ///< [14]レイヤ名が不正
        kErrorNo_InvalidImageNo,      ///< [15]画像番号が不正

        kErrorNo_Max
    };

    Anm2DBase()
    {
        m_nError = kErrorNo_NoError;
    }
    virtual ~Anm2DBase() {}

    int getErrorNo() const { return m_nError; }
    void setFilePath(QString &str) { m_filePath = str; }

    QString getErrorString();
    // srcからdestへの相対パスを取得
    static QString getRelativePath(QString &src, QString &dest);
    // srcからdestへの相対パスを絶対パスに変換
    static QString getAbsolutePath(QString &src, QString &dest);

protected:
    int m_nError;
    QString m_filePath;
};

#endif // ANM2DBASE_H
