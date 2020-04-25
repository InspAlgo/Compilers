#include "lr0parser.h"
#include <QDebug>

LR0Parser::LR0Parser(QObject *parent):
    QObject (parent)
{
    m_dot = L"\x2022";  // 英文「•」
    m_start_token = L"";
    m_end_of_file = L"$";
}

void LR0Parser::set_start_token(QString start_token)
{
    m_start_token = start_token.toStdWString();
}

bool LR0Parser::add_production(QString production_left, QStringList production_right)
{
    auto p_right = std::vector<std::wstring>();

    for(auto i : production_right)
        p_right.push_back(i.toStdWString());

    auto grammar_size = m_grammar.size();
    m_grammar.insert(std::make_tuple(production_left.toStdWString(), p_right));

    return m_grammar.size() != grammar_size? true : false;
}

void LR0Parser::running()
{
    m_parser.Clear();

    m_parser.SetStartToken(m_start_token);
    m_parser.SetDot(m_dot);
    m_parser.SetEndOfFile(m_end_of_file);

    for(auto i: m_grammar)
        m_parser.AddProduction(std::get<0>(i), std::get<1>(i));

    m_parser.RunParsing();

    GetData();
}

QStringList LR0Parser::get_grammar_expanding()
{
    QStringList re;
    int count = 0;

    for(auto i : m_grammar_expanding)
    {
        QString production = QString::number(count) + ". " + QString::fromStdWString(std::get<0>(i));
        production += " ->";
        for(auto t : std::get<1>(i))
            production += " " + QString::fromStdWString(t);
        re.append(production);
        count++;
    }

    return re;
}

QStringList LR0Parser::get_states()
{
    QStringList re;
    int count = 0;

    for(auto item_set: m_states)
    {
        QString temp_item = QString::number(count) + ". ";

        for(auto item: item_set)
        {
            temp_item += QString::fromStdWString(std::get<0>(item)) + " ->";
            for(auto t : std::get<1>(item))
                temp_item += " " + QString::fromStdWString(t);
            temp_item += QString::fromStdWString(L"\xff1b");  // 中文分号「；」
        }

        re.append(temp_item);

        count++;
    }

    return re;
}

QStringList LR0Parser::get_table_rowheader()
{
    QStringList re;

    for(auto i = size_t(0); i < m_states.size(); i++)
        re.append(QString::number(i));

    return re;
}

QStringList LR0Parser::get_table_colheader()
{
    QStringList re;

    for(auto i : m_columns_header)
        re.append(QString::fromStdWString(i));

    return re;
}

QVariantMap LR0Parser::get_parsing_table()
{
    QVariantMap re;

    for(auto i : m_parsing_table)
    {
        QString state_index = QString::number(std::get<0>(i.first));
        QString token = QString::fromStdWString(std::get<1>(i.first));
        QString cell_data = QString::fromStdWString(i.second);

        re.insert(state_index+token, cell_data);
    }

    return re;
}

void LR0Parser::clear()
{
    m_parser.Clear();

    m_grammar.clear();

    m_grammar_expanding.clear();
    m_states.clear();
    m_columns_header.clear();
    m_parsing_table.clear();
}

void LR0Parser::GetData()
{
    m_grammar_expanding.clear();
    m_states.clear();
    m_columns_header.clear();
    m_parsing_table.clear();

    m_parser.GetGrammar(m_grammar_expanding);
    m_parser.GetStates(m_states);
    m_parser.GetColumnsHeader(m_columns_header);
    m_parser.GetParsingTable(m_parsing_table);
}

void LR0Parser::set_input_tokens(QStringList input_tokens)
{
    auto input_tokens_vec = std::vector<std::wstring>();

    for(auto i : input_tokens)
        input_tokens_vec.push_back(i.toStdWString());

    m_parser.SetNextTokenInput(input_tokens_vec);
}

void LR0Parser::AddStepData(std::tuple<int, std::wstring, std::wstring, std::wstring, std::wstring> &data)
{
    auto re = std::wstring(L"");
    re += std::wstring(L"步骤：") + std::to_wstring(std::get<0>(data)) + std::wstring(L"\n");
    re += std::wstring(L"状态栈：") + std::get<1>(data) + std::wstring(L"\n");
    re += std::wstring(L"符号栈：") + std::get<2>(data) + std::wstring(L"\n");
    re += std::wstring(L"输入符号串：") + std::get<3>(data) + std::wstring(L"\n");
    re += std::wstring(L"分析动作：") + std::get<4>(data);

    m_parsing_reslut.push_back(re);
}

void LR0Parser::run_tokens_parsing()
{
    m_parsing_reslut.clear();
    m_parser.LR0ParsingInit();
    auto re = std::make_tuple(0, std::wstring(L""), std::wstring(L""), std::wstring(L""), std::wstring(L""));
    m_parser.ReNextStep(re);
    AddStepData(re);
    while (m_parser.RunCurStep())
    {
        m_parser.ReNextStep(re);
        AddStepData(re);
    }
    m_parser.ReNextStep(re);
    AddStepData(re);
}

QString LR0Parser::get_step_data()
{
    QString re = "";
    if(m_parsing_reslut.size())
    {
        re = QString::fromStdWString(m_parsing_reslut.front());
        m_parsing_reslut.pop_front();
    }
    return re;
}
