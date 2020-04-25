#include "ll1parsingtoqml.h"
#include <QDebug>
#include <iostream>

LL1ParsingToQML::LL1ParsingToQML(QObject *parent):
    QObject (parent)
{

}

void LL1ParsingToQML::test()
{
    m_parser.RunParsing();

    // Get FIRST_S Set
    m_firsts.clear();
    auto first_set = std::map<std::wstring, std::set<std::wstring>>();
    m_parser.GetFirstSet(first_set);
//    qDebug()<<QString("FIRST Set:");
    for(auto x : first_set)
    {
//        qDebug()<<QString::fromStdWString(x.first);
        std::wstring first =  std::wstring(L"FIRST(") + x.first + std::wstring(L")={");
        for(auto i : x.second)
            first += (i.length() > 0? i: L"\x3b5") + L",";
        first[first.length()-size_t(1)] = L'}';

        m_firsts.push_back(QString::fromStdWString(first + L"\n"));
//        qDebug()<<QString::fromStdWString(first + L"\n");
    }

    // Get FOLLOW Set
    m_follow.clear();
    auto follow_set = std::map<std::wstring, std::set<std::wstring>>();
    m_parser.GetFollowSet(follow_set);
//    qDebug()<<QString("FOLLOW Set:");
    for(auto ts:follow_set)
    {
        std::wstring follow = std::wstring(L"FOLLOW(") + ts.first + L")={";
        for(auto i : ts.second)
            follow += (i.length() > 0? i + L",": L"");
        if(!ts.second.size())
            follow += L",";
        follow[follow.length()-size_t(1)] = L'}';

        m_follow.push_back(QString::fromStdWString(follow + L"\n"));
//        qDebug()<<QString::fromStdWString(follow + L"\n");
    }

    // Get Table
    auto parsing_table = std::map<std::tuple<std::wstring, std::wstring>,
            std::set<std::tuple<std::wstring, std::vector<std::wstring>>>>();

    m_parser.GetPredictiveParsingTable(parsing_table);
    m_re.clear();
    m_table.clear();
//    qDebug()<<QString("Parsing Table:");
    for (auto NT : parsing_table)
    {
        std::wstring pstr =   std::get<0>(NT.first) + L" - " + std::get<1>(NT.first) + L": ";

        QString N = std::get<0>(NT.first).length()>0?QString::fromStdWString(std::get<0>(NT.first)):QString("ε");
        QString T = std::get<1>(NT.first).length()>0?QString::fromStdWString(std::get<1>(NT.first)):QString("ε");

        std::wstring production_str = L"";
        for (auto production : NT.second)
        {
            auto left = std::get<0>(production);
            auto right = std::get<1>(production);

            production_str += left + L" ->";
            pstr += left + L" ->";
            for (auto t : right)
            {
                t = L" " + (t.length() > 0? t : L"\x3b5");

                production_str += t;
                pstr += t;
            }

            production_str += L" ";
            pstr += L"; ";
        }
        m_re.push_back(QString::fromStdWString(pstr+L"\n"));
//        qDebug()<<QString::fromStdWString(pstr+L"\n");
        m_table[N + T] = QString::fromStdWString(production_str);
    }
}

void LL1ParsingToQML::add_token(QString token)
{
    m_production.push_back(token);
}

void LL1ParsingToQML::add_token_left(QString token)
{
    m_production_left = token.toStdWString();
}

bool LL1ParsingToQML::add_production()
{
    auto production = std::vector<std::wstring>();
    for(auto i:m_production)
        production.push_back(i.toStdWString());
    m_production.clear();

    auto size = m_productions_map[m_production_left].size();
    m_productions_map[m_production_left].insert(production);

    if(m_productions_map[m_production_left].size() != size)  // 利用 set 的特性进行判断是否有重复
    {
        m_parser.AddProduction(m_production_left, production);
        return true;
    }
    else
        return false;
}

void LL1ParsingToQML::set_start_token(QString start_token)
{
    m_start_token = start_token.toStdWString();
    m_parser.SetStartToken(start_token.toStdWString());
}

void LL1ParsingToQML::clear()
{
    m_production.clear();
    m_productions_map.clear();
    m_parser.Clear();
    m_re.clear();
    m_firsts.clear();
    m_follow.clear();
    m_table.clear();
}

QString LL1ParsingToQML::get_first_s_set_str()
{
    QString re = "";
    for(auto i:m_firsts)
        re += i;
    return re;
}

QString LL1ParsingToQML::get_follow_set_str()
{
    QString re = "";
    for(auto i:m_follow)
        re += i;
    return re;
}

QString LL1ParsingToQML::get_parsing_table_str()
{
    QString re = "";
    for(auto i:m_re)
        re+=i;
    return re;
}

QStringList LL1ParsingToQML::get_nonterminal()
{
    QStringList re;
    auto nonterminal = std::set<std::wstring>();
    m_parser.GetNonterminalSet(nonterminal);
    for(auto i : nonterminal)
        re.append(QString::fromStdWString(i));
    return re;
}

// 返回不带 ε 的终结符集合
QStringList LL1ParsingToQML::get_terminal()
{
    QStringList re;
    auto terminal = std::set<std::wstring>();
    m_parser.GetTerminalSet(terminal);
    for(auto i : terminal)
        if(i != L"\x3b5")
            re.append(QString::fromStdWString(i));
    re.append("#");
    return re;
}

QVariantMap LL1ParsingToQML::get_table()
{
    QVariantMap re;
    for(auto i : m_table)
    {
        re.insert(i.first, i.second);
    }
    return re;
}
