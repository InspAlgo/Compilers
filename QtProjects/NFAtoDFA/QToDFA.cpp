#include "QToDFA.h"
#include <codecvt>
#include <fstream>
#include <sstream>
#include <iostream>
#include <QProcess>
#include <QFileInfo>
#include <QFile>

QToDFA::QToDFA(QObject *parent) : QObject(parent)
{

}

QString QToDFA::get_dir()
{
    return QDir::currentPath() + QString("/");
}

void QToDFA::remove_temp_files()
{
    QFile temp_1("$NFA.txt");
    temp_1.remove();

    QFile temp_2("$NFA.txt.toDFA.bat");
    temp_2.remove();

    QFile temp_3("$NFA.txt.toDFA.dot");
    temp_3.remove();

    QFile temp_4("$NFA.txt.toDFA.bat");
    temp_4.remove();

    QFile temp_5("$NFA.txt.toDFA.svg");
    temp_5.remove();
}

QString QToDFA::to_dfa(QString nfa)
{
    remove_temp_files();

    std::locale utf8(std::locale(), new std::codecvt_utf8<wchar_t>);
    auto text = nfa.toStdWString();
    std::wfstream file_dot(L"$NFA.txt", std::ios::out);
    file_dot.imbue(utf8);
    file_dot << text;
    file_dot.close();

    QProcess p;
    p.start("cmd.exe", QStringList()<<"/c"<<"n2d $NFA.txt svg");
    if (p.waitForStarted())
    {
        p.waitForFinished();
        p.close();
    }
    else
    {
        std::cout<<"Can't run n2d"<<std::endl;
        exit(-1);
    }


    QProcess p2;
    p2.start("cmd.exe", QStringList()<<"/c"<<"$NFA.txt.toDFA.bat");
    if (p2.waitForStarted())
    {
        p2.waitForFinished();
        p2.close();
    }
    else
    {
        std::cout << "Can't run bat" << std::endl;
        exit(-1);
    }

    auto DFA_image_filepath = QString("$NFA.txt.toDFA.svg");

    QFileInfo fileInfo(DFA_image_filepath);
    while(!fileInfo.isFile())
    {
        std::cout << "Can't find svg" << std::endl;
    }

    return DFA_image_filepath;
}
