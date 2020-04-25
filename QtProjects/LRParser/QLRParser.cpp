#include "QLRParser.h"

#include <QDebug>
#include <QRegion>

QLRParser::QLRParser(QObject *parent) : QObject(parent)
{
    m_end_of_file = QString("$");
}

void QLRParser::build_parsing_table(QString start_token, QString new_start_token, QString input_grammar)
{
    m_LR0 = m_SLR1 = m_LALR1 = m_LR1 = false;
    m_original_grammar.clear();
    m_parser.Clear();

    // 添加文法

    // 设置起始符号
    start_token = start_token.simplified();
    new_start_token = new_start_token.simplified();

    if (start_token == new_start_token)
        return;

    m_parser.SetStartToken(start_token.toStdWString(), new_start_token.toStdWString());

    auto line_list = input_grammar.split('\n');  // 得到行字符串

    // 添加产生式
    for (auto i : line_list) {
        i = i.simplified();  // 去掉首末空格
        if (!i.length())
            continue;

        if (!i.contains("->"))  // 如果不含有 "->"
            continue;

        auto production = i.split("->");

        auto left = production.front().trimmed();

        if (!left.length())
            continue;

        auto right_string = production.back().trimmed();

        if (!right_string.length())
            continue;

        auto right = right_string.split(' ');

        if (right.length() == size_t(1) &&
                (right.front() == QString::fromStdWString(L"\x3b5") || right.front() == QString("epsilon")))
            right[size_t(0)] = QString("");

        auto production_left = left.toStdWString();
        auto production_right = std::vector<std::wstring>();

        for (auto k : right) {
            production_right.push_back(k.toStdWString());
            //            qDebug() << k;

            if (k == new_start_token)
                return;
        }

        if (!production_left.length() || !production_right.size())
            continue;

        m_parser.AddProduction(production_left, production_right);

        m_original_grammar.insert(std::make_tuple(production_left,production_right));
    }

    if (!m_original_grammar.size())
        return;

    // 构建 LR 分析表
    m_parser.BuildLRParsingTable();

    // 获取文法类型
    auto type = m_parser.GetGrammarType();
    if (type == std::wstring(L"LR(0)"))
        m_LR0 = true;
    else if (type == std::wstring(L"SLR(1)"))
        m_SLR1 = true;
    else if (type == std::wstring(L"LALR(1)"))
        m_LALR1 = true;
    else if (type == std::wstring(L"LR(1)"))
        m_LR1 = true;

    GetItemsSets();
}

QString QLRParser::get_grammar_type()
{
    auto re = QString("");

    if (m_LR0)
        re = QString("LR(0)");
    else if (m_SLR1)
        re = QString("SLR(1)");
    else if (m_LALR1)
        re = QString("LALR(1)");
    else if (m_LR1)
        re = QString("LR(1)");
    else
        re = QString("Not");

    return re;
}

QStringList QLRParser::get_expanding_grammar()
{
    // 获取拓广文法
    std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> expanding_grammar;
    QStringList re_expanding_grammar;

    m_parser.GetExpandingGrammar(expanding_grammar);

    int production_count = 0;

    for (auto &i : expanding_grammar) {
        auto production = QString::number(production_count++) + QString(". ");
        production += QString::fromStdWString(std::get<0>(i)) + QString(" -> ");

        for(auto &x : std::get<1>(i))
            production += QString::fromStdWString(x.length()? x : L"\x3b5") + QString(" ");

        re_expanding_grammar.append(production.trimmed());
    }

    return re_expanding_grammar;
}

void QLRParser::GetItemsSets()
{
    m_items_sets_count = 0;
    m_items_sets.clear();

    if (m_LR0 || m_SLR1) {
        std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> items_sets;
        m_parser.GetItemsSets(items_sets);
        for (auto &items_set : items_sets) {
            auto items_set_str = QString("I") + QString::number(m_items_sets_count++) + QString(" = { ");

            for (auto &item : items_set) {
                items_set_str += QString::fromStdWString(std::get<0>(item)) + QString(" -> ");

                for(auto &x : std::get<1>(item))
                    items_set_str += QString::fromStdWString(x.length()? x : L"\x3b5") + QString(" ");

                items_set_str.chop(1);
                items_set_str+=QString("; ");
            }

            items_set_str.chop(2);
            items_set_str += QString(" }");
            m_items_sets.append(items_set_str);
        }
    }
    else {
        std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>, std::set<std::wstring>>>> items_sets;
        m_parser.GetItemsSets(items_sets);
        for (auto &items_set : items_sets) {
            auto items_set_str = QString("I") + QString::number(m_items_sets_count++) + QString(" = { ");

            for (auto &item : items_set) {
                // 项目左部
                items_set_str += QString::fromStdWString(std::get<0>(item)) + QString(" -> ");
                // 项目右部
                for(auto &x : std::get<1>(item))
                    items_set_str += QString::fromStdWString(x.length()? x : L"\x3b5") + QString(" ");
                // 向前查看符
                items_set_str += QString(", ");
                for (auto &a : std::get<2>(item))
                    items_set_str += QString::fromStdWString(a) + QString("/");
                items_set_str.chop(1);  // 删除最后一个 "/" 符号
                items_set_str += QString("; ");
            }

            items_set_str.chop(2);
            items_set_str += QString(" }");
            m_items_sets.append(items_set_str);
        }
    }
}

QStringList QLRParser::get_items_sets()
{
    // 获取项目集
    QStringList re_items_sets;

    for (auto &i : m_items_sets)
        re_items_sets.append(i);

    return re_items_sets;
}

QStringList QLRParser::get_dfa()
{
    QStringList re_DFA;
    std::map<std::tuple<size_t, std::wstring>, size_t> DFA;

    m_parser.GetDFA(DFA);

    for (auto &i : DFA) {
        auto dfa = QString("GO(I") + QString::number(std::get<0>(i.first)) + QString(", ");
        dfa += QString::fromStdWString(std::get<1>(i.first)) + QString(") = I");
        dfa += QString::number(i.second);
        re_DFA.append(dfa);
    }

    return re_DFA;
}

QStringList QLRParser::get_col_header()
{
    QStringList re_col_header;
    std::vector<std::wstring> terminals, nonterminals;

    m_parser.GetTerminals(terminals);
    m_parser.GetNonterminals(nonterminals);

    for (auto &i : terminals)
        re_col_header.append(QString::fromStdWString(i));

    re_col_header.append(m_end_of_file);

    for (auto &i : nonterminals)
        re_col_header.append(QString::fromStdWString(i));

    return  re_col_header;
}

QStringList QLRParser::get_row_header()
{
    QStringList re_row_header;

    for (auto i = 0; i < m_items_sets_count; i++)
        re_row_header.append(QString::number(i));

    return re_row_header;
}

QVariantMap QLRParser::get_parsing_table()
{
    QVariantMap re_parsing_table;
    std::map<std::tuple<std::wstring, std::wstring>, std::wstring> parsing_table;

    m_parser.GetParsingTable(parsing_table);

    for (auto &i : parsing_table) {
        auto first = QString::fromStdWString(std::get<0>(i.first)) + QString::fromStdWString(std::get<1>(i.first));
        auto second = QString::fromStdWString(i.second);
        re_parsing_table.insert(first, second);
    }

    return re_parsing_table;
}

QVariantMap QLRParser::parsing_tokens(QString tokens)
{
    QVariantMap re_table;
    std::vector<std::wstring> input_tokens;
    std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>> parsing_process;

    for (auto &i : tokens.simplified().split(' ')) {
        if(i.length())
            input_tokens.push_back(i.toStdWString());
    }

    m_parser.ParsingTokens(input_tokens, parsing_process);

    auto header = std::vector<QString>{
        QString("step_count"), QString("state_stack"), QString("tokens_stack"), QString("input_tokens"), QString("parsing_action")
    };

    m_parsing_process_step_count.clear();

    for (auto &i : parsing_process) {
        auto first = QString::fromStdWString(std::get<0>(i));
        auto second1 = QString::fromStdWString(std::get<1>(i));
        auto second2 = QString::fromStdWString(std::get<2>(i));
        auto second3 = QString::fromStdWString(std::get<3>(i));
        auto second4 = QString::fromStdWString(std::get<4>(i));

        m_parsing_process_step_count.append(first);
        re_table.insert(first + header[1], second1);
        re_table.insert(first + header[2], second2);
        re_table.insert(first + header[3], second3);
        re_table.insert(first + header[4], second4);
    }

    return re_table;
}

QStringList QLRParser::get_parsing_process_step_count()
{
    QStringList re_step_count;

    for (auto &i: m_parsing_process_step_count)
        re_step_count.append(i);

    return re_step_count;
}
