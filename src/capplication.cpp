#include <QStringList>
#include <stdio.h>
#include <QDebug>
#include "capplication.h"
#include "mainwindow.h"
#include "editdata.h"
#include "canm2d.h"

CApplication::CApplication(int argc, char **argv) :
    QApplication(argc, argv),
    m_execType(kExecType_Normal)
{
    for ( int i = 0 ; i < argc ; i ++ ) {
        m_args.append(argv[i]);
    }
#if 0
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);
#endif
}

bool CApplication::checkOpt()
{
    bool bNextInput = false ;
    bool bNextOutput = false ;
    for ( int i = 1 ; i < m_args.size() ; i ++ ) {
        const QString str = m_args.at(i) ;
        if ( str == "-h" || str == "--help" ) {
            return false ;
        }
        else if ( str == "-asm" ) {
            m_execType = kExecType_OutputAsm ;
        }
        else if ( str == "-i" ) {
            bNextInput = true ;
        }
        else if ( str == "-o" ) {
            bNextOutput = true ;
        }
        else {
            if ( bNextInput ) {
                m_inputFile = m_args.at(i) ;
                m_inputFile = m_inputFile.replace("\\", "/") ;
                bNextInput = false ;
            }
            else if ( bNextOutput ) {
                m_outputFile = m_args.at(i) ;
                m_outputFile = m_outputFile.replace("\\", "/") ;
                bNextOutput = false ;
            }
        }
    }

    switch ( m_execType ) {
        case kExecType_Normal:
            if ( m_args.size() > 1 ) {
                m_execType = kExecType_ArgFile ;
                m_inputFile = m_args.at(1) ;
                m_inputFile = m_inputFile.replace("\\", "/") ;
            }
            break ;
        case kExecType_OutputAsm:
            if ( m_inputFile.isEmpty() || m_outputFile.isEmpty() ) { return false ; }
            break ;
    }

    return true ;
}

void CApplication::print_usage()
{
    qDebug() << trUtf8("usage : asm吐き出し") ;
    qDebug() << trUtf8("        %1 -asm -i input_file -o output_file").arg(this->applicationName()) ;
}

int CApplication::execute()
{
    switch ( m_execType ) {
        case kExecType_Normal:
        case kExecType_ArgFile:
            {
                Q_INIT_RESOURCE(Resource) ;
                QTranslator translator;
                translator.load(":/root/Resources/lang/linguist_ja.qm");
                installTranslator(&translator) ;

                MainWindow w;
                w.show();

                if ( !m_inputFile.isEmpty() ) {
                    w.fileOpen(m_inputFile) ;
                }

                return this->exec() ;
            }
        case kExecType_OutputAsm:
            {
                CEditData editData ;
                {	// 入力ファイル読み込み
                    CAnm2DXml data(false) ;
                    QFile file(m_inputFile) ;
                    if ( !file.open(QFile::ReadOnly) ) {
                        qDebug() << trUtf8("ファイルオープン失敗[") << m_inputFile << "]" ;
                        print_usage();
                        return 1 ;
                    }
                    QDomDocument xml ;
                    xml.setContent(&file) ;
                    data.setFilePath(m_inputFile);
                    if ( !data.makeFromFile(xml, editData) ) {
                        qDebug() << trUtf8("ファイル読み込み失敗[") << m_inputFile << "]:" << data.getErrorString() ;
                        print_usage();
                        return 1 ;
                    }
                }
                {	// 出力ファイル書き込み
                    CAnm2DAsm	data(false) ;
                    if ( !data.makeFromEditData(editData) ) {
                        if ( data.getErrorNo() != CAnm2DBase::kErrorNo_Cancel ) {
                            qDebug() << trUtf8("コンバート失敗[") << m_outputFile << "]:" << data.getErrorString() ;
                            print_usage();
                        }
                        return 1 ;
                    }

                    QFile file(m_outputFile) ;
                    if ( !file.open(QFile::WriteOnly) ) {
                        qDebug() << trUtf8("ファイル書き込み失敗[") << m_outputFile << "]:" << data.getErrorString() ;
                        print_usage();
                        return 1 ;
                    }
                    file.write(data.getData().toAscii()) ;

                    QString		incFileName = m_outputFile;
                    incFileName.replace(QString(".asm"), QString(".inc"));
                    CAnm2DAsm	dataInc(false);
                    if(!dataInc.makeFromEditData2Inc(editData)){
                        if(dataInc.getErrorNo() != CAnm2DBase::kErrorNo_Cancel){
                            qDebug() << trUtf8("コンバート失敗[") << incFileName << "]:" << data.getErrorString() ;
                            print_usage();
                        }
                        return 1 ;
                    }
                    QFile	fileInc(incFileName);
                    if(!fileInc.open(QFile::WriteOnly)){
                        qDebug() << trUtf8("ファイル書き込み失敗[") << incFileName << "]:" << data.getErrorString() ;
                        return 1 ;
                    }
                    fileInc.write(dataInc.getData().toAscii());
                }
            }
            break ;
    }
    return 0 ;
}
