#include "LL1Parsing.h"

M6::LL1Parsing::LL1Parsing()
{

}

void M6::LL1Parsing::AddProduction(const token &production_left, const std::vector<token> &production_right)
{
    m_productions.push_back(std::make_tuple(production_left, production_right));
    m_nonterminal.insert(production_left);
    m_first_s_set[std::make_tuple(production_left, production_right)] = std::set<token>();
}

void M6::LL1Parsing::RunParsing()
{
    InitSet();
    CalcNullable();
    CalcFirstSet();
    CalcFollowSet();
    CalcFirstSSet();
    CalcPredictiveParsingTable();
}

void M6::LL1Parsing::GetPredictiveParsingTable(std::map<std::tuple<token, token>,
    std::set<std::tuple<token, std::vector<token>>>> &predictive_parsing_table)
{
    for (auto i : m_predictive_parsing_table)
    {
        predictive_parsing_table[i.first] = i.second;
    }
}

void M6::LL1Parsing::Clear()
{
    m_nonterminal.clear();
    m_productions.clear();
    m_nullable.clear();
    m_first_set.clear();
    m_follow_set.clear();
    m_first_s_set.clear();
    m_predictive_parsing_table.clear();
}

void M6::LL1Parsing::InitSet()
{
    m_first_set.clear();
    m_follow_set.clear();

    for (auto nonterminal : m_nonterminal)
    {
        m_nullable[nonterminal] = false;

        m_first_set[nonterminal] = std::set<token>();
        m_first_set[nonterminal].clear();

        m_follow_set[nonterminal] = std::set<token>();
        m_follow_set[nonterminal].clear();
    }
}

void M6::LL1Parsing::CalcNullable()
{
    auto is_changing = true;  // 标记 nullable 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto production : m_productions)
        {
            // 如果此非终结符已经判断为可空则跳过
            if (m_nullable[std::get<0>(production)])
                continue;

            // 如果右部直接就是 ε，说明此非终结符可空
            if (std::get<1>(production)[0].length() == 0)
            {
                m_nullable[std::get<0>(production)] = true;
                is_changing = true;
                continue;
            }

            // 右部不是 ε，遍历产生式右部每一个符号
            auto flag = true;
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

void M6::LL1Parsing::CalcFirstSet()
{
    auto is_changing = true;  // 标记 FIRST 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto production : m_productions)
        {
            // 右部直接就是 ε
            if (std::get<1>(production)[0].length() == 0)
                continue;

            auto N = std::get<0>(production);
            auto count = m_first_set[N].size();

            // 右部不是 ε，遍历产生式右部每一个符号
            for (auto beta_i : std::get<1>(production))
            {
                // 是终结符，即不在非终结符集合中
                if (m_nonterminal.find(beta_i) == m_nonterminal.end()) 
                {
                    m_first_set[N].insert(beta_i);
                    break;
                }
                else
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

void M6::LL1Parsing::CalcFollowSet()
{
    auto is_changing = true;  // 标记 FOLLOW 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto production : m_productions)
        {
            auto production_right = std::get<1>(production);
            
            if (std::get<1>(production)[0].length() == 0)
                continue;

            auto N = std::get<0>(production);
            auto count = m_follow_set[N].size();
            auto temp = m_follow_set[N];

            // 右部不是 ε，逆序遍历产生式右部每一个符号
            for (auto iter = production_right.rbegin(); iter != production_right.rend(); ++iter)
            {
                if (m_nonterminal.find((*iter)) == m_nonterminal.end())
                    (temp = std::set<token>()).insert((*iter));
                else
                {
                    auto count2 = m_follow_set[(*iter)].size();

                    m_follow_set[(*iter)].insert(temp.begin(), temp.end());

                    if (m_follow_set[(*iter)].size() != count2)
                        is_changing = true;

                    if (!m_nullable[(*iter)])
                        temp = m_first_set[(*iter)];
                    else
                        temp.insert(m_first_set[(*iter)].begin(), m_first_set[(*iter)].end());
                }
            }

            if (m_follow_set[N].size() != count)
                is_changing = true;
        }
    }
}

void M6::LL1Parsing::CalcFirstSSet()
{
    for (auto production : m_productions)
    {
        auto flag = true;

        for (auto beta_i : std::get<1>(production))
        {
            if (beta_i.length() == 0)  // 右部是 ε 则跳过
                continue;

            if (m_nonterminal.find(beta_i) == m_nonterminal.end())
            {
                m_first_s_set[production].insert(beta_i);
                flag = false;
                break;
            }
            else
            {
                m_first_s_set[production].insert(m_first_set[beta_i].begin(), m_first_set[beta_i].end());
                if (!m_nullable[beta_i])
                {
                    flag = false;
                    break;
                }
            }
        }

        if (flag)
        {
            auto temp = m_follow_set[std::get<0>(production)];
            m_first_s_set[production].insert(temp.begin(), temp.end());
        }
    }
}

void M6::LL1Parsing::CalcPredictiveParsingTable()
{
    for (auto i : m_first_s_set)
    {
        for (auto j : i.second)
        {
            auto N = std::get<0>(i.first);
            auto T = j;
            auto production = i.first;

            m_predictive_parsing_table[std::make_tuple(N, T)].insert(production);
        }
    }
}
