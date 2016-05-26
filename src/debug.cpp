#include "debug.h"

void WriteLogFile(QString str)
{
    QFile file("./log.txt");
    if (!file.open(QFile::Append | QFile::Text))
    {
        return;
    }
    QDate nowDate = QDate::currentDate();
    QTime nowTime = QTime::currentTime();
    str = nowDate.toString("dd/MM/yyyy") + " " + nowTime.toString("hh:mm:ss.zzz") + " " + str;
    file.write(str.toStdString().c_str());
    file.close();
}
