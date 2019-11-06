#include "LL1Parsing.h"

LL1Parsing::LL1Parsing()
{
    m_terminal = std::set<wchar_t>();
    m_nonterminal = std::set<wchar_t>();
    m_productions = std::vector<std::tuple<wchar_t, std::wstring>>();
    m_first_set = std::map<wchar_t, std::set<wchar_t>>();
    m_follow_set = std::map<wchar_t, std::set<wchar_t>>();
    m_predictive_parsing_table = std::map<std::pair<wchar_t, wchar_t>, std::set<std::wstring>>();
}

void LL1Parsing::AddProduction(wchar_t production_left, std::wstring production_right)
{
    m_productions.push_back(std::make_tuple(production_left, production_right));
    m_nonterminal.insert(production_left);
}

void LL1Parsing::GetPredictiveParsingTable()
{
    InitSet();
    GetNullable();
    GetFirstSet();
}

void LL1Parsing::InitSet()
{
    m_first_set.clear();
    m_follow_set.clear();

    for (auto nonterminal : m_nonterminal)
    {
        m_nullable[nonterminal] = false;

        m_first_set[nonterminal] = std::set<wchar_t>();
        m_first_set[nonterminal].clear();

        m_follow_set[nonterminal] = std::set<wchar_t>();
        m_follow_set[nonterminal].clear();
    }
}

void LL1Parsing::GetNullable()
{
    bool is_changing = true;  // 标记 nullable 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto production : m_productions)
        {
            // 如果此非终结符已经判断为可空则跳过
            if (m_nullable[std::get<0>(production)])
                continue;

            // 如果右部直接就是 ε，说明此非终结符可空
            if (std::get<1>(production).length() == 0)
            {
                m_nullable[std::get<0>(production)] = true;
                is_changing = true;
                continue;
            }

            // 右部不是 ε，遍历产生式右部每一个符号
            bool flag = true;
            for (auto beta_i : std::get<1>(production))
            {
                if (m_nonterminal.find(beta_i) == m_nonterminal.end()
                    || m_nullable[beta_i] == false)
                {
                    flag = false;
                    break;
                }
            }
            if (flag)
            {
                m_nullable[std::get<0>(production)] = true;
                is_changing = true;
            }
        }
    }
}

void LL1Parsing::GetFirstSet()
{
    bool is_changing = true;  // 标记 FIRST 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto production : m_productions)
        {
            // 右部直接就是 ε
            if (std::get<1>(production).length() == 0)
                continue;

            auto N = std::get<0>(production);
            int count = m_first_set[N].size();

            // 右部不是 ε，遍历产生式右部每一个符号
            for (auto beta_i : std::get<1>(production))
            {
                // 是终结符，即不在非终结符集合中
                if (m_nonterminal.find(beta_i) == m_nonterminal.end()) 
                {
                    m_first_set[N].insert(beta_i);
                    break;
                }
                else  // 是非终结符
                {
                    m_first_set[N].insert(m_first_set[beta_i].begin(), m_first_set[beta_i].end());
                    if (!m_nullable[beta_i])
                        break;
                }
            }

            if (m_first_set[N].size() != count)
                is_changing = true;
        }
    }
}

void LL1Parsing::GetFollowSet()
{
    bool is_changing = true;  // 标记 FOLLOW 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto production : m_productions)
        {
            // 右部直接就是 ε
            if (std::get<1>(production).length() == 0)
                continue;

            auto N = std::get<0>(production);
            int count = m_follow_set[N].size();

            auto temp = m_follow_set[N];

            // 右部不是 ε，遍历产生式右部每一个符号
            for (auto beta_i : std::get<1>(production))
            {
                
            }

            if (m_follow_set[N].size() != count)
                is_changing = true;
        }
    }
}