#ifndef ANM2DTYPES_H
#define ANM2DTYPES_H

/*!
	@brief anm2d format

	Anm2DHeader
	Block * Anm2DHeader::nBlockNum

	・ブロックが４バイト境界で終了しない場合はパディングで４バイトにそろえる。
 */

#define ANM2D_VERSION	0x00000001				///< バージョン

#define ANM2D_ID_HEADER		'ANM2'				///< ヘッダID
#define ANM2D_ID_OBJECT		'OBJE'				///< オブジェクトID
#define ANM2D_ID_LAYER		'LAYR'				///< レイヤID
#define ANM2D_ID_FRAMEDATA	'FRAM'				///< フレームデータID
#define ANM2D_ID_IMAGE		'IMAG'				///< イメージID

// @brief ブロック構造
typedef struct {
	unsigned int		nID ;					///< ID
	unsigned int		nSize ;					///< サイズ
} Anm2DBlockHeader ;

// @brief 名前
typedef union {
	char				name[16] ;
	int					id[4] ;
} Anm2DName ;

// @brief ヘッダブロック
typedef struct {
	Anm2DBlockHeader	header ;				///< nID = 'ANM2'
												///< nSize = sizeof(Anm2DHeader) + (nBlockNum-1) * unsigned int
	unsigned int		nVersion ;				///< バージョン
	unsigned int		nFileSize ;				///< ファイルサイズ
	unsigned short		nBlockNum ;				///< ブロック数
	unsigned short		pad[1] ;
	unsigned int		nBlockOffset[1] ;		///< ブロックデータへのオフセット
} Anm2DHeader ;

// @brief オブジェクトブロック
typedef struct {
	Anm2DBlockHeader	header ;				///< nID = 'OBJE'
												///< nSize = sizeof(Anm2DObject) + (nLayerNum-1) * unsigned int
	Anm2DName			objName ;				///< オブジェクト名
	unsigned short		nLayerNum ;				///< レイヤ数
	unsigned short		pad[1] ;
	unsigned int		nLayerNo[1] ;			///< レイヤ番号
} Anm2DObject ;

// @brief レイヤブロック
typedef struct {
	Anm2DBlockHeader	header ;				///< nID = 'LAYR'
												///< nSize = sizeof(Anm2DLayer) + (nFrameDataNum-1) * unsigned int
	Anm2DName			layerName ;				///< レイヤ名
	unsigned short		nLayerNo ;				///< レイヤ番号
	unsigned short		nFrameDataNum ;			///< フレームデータ数
	unsigned int		nFrameDataNo[1] ;		///< フレームデータ番号
} Anm2DLayer ;

// @brief フレームデータブロック
typedef struct {
	Anm2DBlockHeader	header ;				///< nID = 'FRAM'
												///< nSize = sizeof(Anm2DFrameData)
	unsigned short		nFrameDataNo ;			///< フレームデータ番号
	unsigned short		nFrame ;				///< このデータのフレーム数
	short				pos_x, pos_y, pos_z ;	///< 位置
	short				rot_x, rot_y, rot_z ;	///< 回転角
	short				cx, cy ;				///< センター位置
	unsigned short		nImageNo ;				///< 参照するイメージ番号
	short				bFlag ;					///< フラグ 0x0001:UVアニメする
	short				uv[4] ;					///< UV [0]:left [1]:right [2]:top [3]:bottom
	float				fScaleX, fScaleY ;		///< スケール
} Anm2DFrameData ;

// @brief イメージブロック
typedef struct {
	Anm2DBlockHeader	header ;				///< nID = 'IMAG'
												///< nSize = sizeof(Anm2DImage) + nWidth * nHeight * 4 * unsigned char - 1
	unsigned short		nWidth ;				///< 幅
	unsigned short		nHeight ;				///< 高さ
	unsigned short		nImageNo ;				///< イメージ番号
	unsigned short		pad[1] ;
	unsigned char		data[1] ;				///< RGBAバイト列(nWidth*nHeight)
} Anm2DImage ;

#endif // ANM2DTYPES_H
