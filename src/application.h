#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include <QApplication>
#include <QStringList>

enum
{
    kExecType_Normal = 0, // 通常起動
    kExecType_ArgFile,    // ファイル指定で起動
    kExecType_OutputAsm   // 指定ファイルをasm形式に吐き出して終了
};

class Application : public QApplication
{
    Q_OBJECT
public:
    explicit Application(int argc, char **argv);

    bool checkOpt();
    void print_usage();
    int execute();

    int getExecType() { return m_execType; }
    const QString &getInputFile() { return m_inputFile; }
    const QString &getOutputFile() { return m_outputFile; }

signals:

public slots:

private:
    int m_execType;
    QString m_inputFile;
    QString m_outputFile;
    QStringList m_args;
};

#endif // CAPPLICATION_H
