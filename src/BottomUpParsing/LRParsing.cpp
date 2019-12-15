#include "LRParsing.h"
#include <queue>

/*
    LR(0) SLR(1) LR(1) LALR(1) 四种 LR 分析算法
*/

M6::LRParsing::LRParsing()
{
    m_start_token = L"";
    m_new_start_token = L"";
    m_end_of_file = L"$";
    m_dot = L"\x2022";  // 英文 「•」

    m_LR0 = false;
    m_SLR1 = false;
    m_LALR1 = false;
    m_LR1 = false;
}

void M6::LRParsing::Clear()
{
    m_start_token = L"";
    m_new_start_token = L"";

    m_LR0 = false;
    m_SLR1 = false;
    m_LALR1 = false;
    m_LR1 = false;
}

void M6::LRParsing::SetStartToken(const std::wstring &start_token, const std::wstring &new_start_token)
{
    m_start_token = start_token;
    m_new_start_token = new_start_token;
}

void M6::LRParsing::SetDot(const std::wstring &dot)
{
    m_dot = dot;
}

void M6::LRParsing::SetEndOfFile(const std::wstring &end_of_file)
{
    m_end_of_file = end_of_file;
}

void M6::LRParsing::AddProduction(const std::wstring &production_left, 
                                  const std::vector<std::wstring> &production_right)
{
    m_nonterminals.insert(production_left);
    m_original_grammar[production_left].insert(production_right);

    m_alltokens.insert(production_left);
    for (auto i : production_right)
        m_alltokens.insert(i);
}

void M6::LRParsing::BuildLRParsingTable()
{
    Preprocess();

    BuildLR0ParsingTable();
    
    if (m_LR0)
        return;

    BuildSLR1ParsingTable();

    if (m_SLR1)
        return;

    BuildLR1ParsingTable();

    if (m_LR1)
        BuildLALR1ParsingTable();
}

int M6::LRParsing::GetGrammarType()
{
    return m_LR0 ? 1 : (m_SLR1 ? 2 : (m_LALR1 ? 3 : (m_LR1 ? 4 : 0)));
}

void M6::LRParsing::Preprocess()
{
    CreateTerminalSet();

    ExpandGrammar();

    CreateOriginalItems();

    CreateReductionItemTable();

    CreateFirstSet();

    CreateFollowSet();
}

void M6::LRParsing::CreateTerminalSet()
{
    m_terminals.clear();

    for (auto i : m_alltokens)
    {
        if (m_nonterminals.find(i) == m_nonterminals.end()) 
            m_terminals.insert(i);
    }
}

void M6::LRParsing::ExpandGrammar()
{
    m_expanding_grammar.clear();

    // 新起始符的产生式放在第一位
    m_expanding_grammar.push_back(std::make_tuple(m_new_start_token, std::vector<Token>{m_start_token}));

    // 其余原始的产生式
    for (auto &i : m_original_grammar)
    {
        auto left = std::get<0>(i);

        for (auto &production : std::get<1>(i))
            m_expanding_grammar.push_back(std::make_tuple(left, production));
    }
}

void M6::LRParsing::CreateOriginalItems()
{
    m_original_items.clear();

    for (auto i : m_expanding_grammar)
    {
        auto left = std::get<0>(i);
        auto production = std::get<1>(i);

        auto new_production = std::vector<Token>{ m_dot };
        new_production.insert(new_production.end(), production.begin(), production.end());

        m_original_items[left].insert(std::make_tuple(left, new_production));
    }
}

void M6::LRParsing::CreateReductionItemTable()
{
    m_reduction_items.clear();

    for (auto i = static_cast<size_t>(0); i < m_expanding_grammar.size(); i++)
    {
        auto left = std::get<0>(m_expanding_grammar[i]);
        auto production = std::get<1>(m_expanding_grammar[i]);

        if ((*production.begin()).length)  // 如果右部不是 epsilon
            production.push_back(m_dot);  // 规约项目即产生式最后面加个 dot 符号
        else  // 右部是 epsilon
            (*production.begin()) = m_dot;  // 仅有项目 A->·

        m_reduction_items[std::make_tuple(left, production)] = i;
    }
}

void M6::LRParsing::CreateNullable()
{
    m_nullable.clear();
    for (auto x : m_nonterminals)
        m_nullable[x] = false;

    auto is_changing = true; // 标记 nullable 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto ex_production : m_expanding_grammar)
        {
            auto left = std::get<0>(ex_production);
            auto right = std::get<1>(ex_production);

            // 如果此非终结符已经判断为可空则跳过
            if (m_nullable[left])
                continue;

            // 如果右部直接就是 epsilon
            if (!right.begin()->length())
            {
                m_nullable[left] = true;
                is_changing = true;
                continue;
            }

            // 右部不是 epsilon，遍历产生式右部每一个符号
            auto flag = true;
            for (auto x : right)
            {
                // 遇到终结符，则此产生式的左部符号是不可空的
                // 遇到非终结符，则判断此非终结符是否可空
                if (m_nonterminals.find(x) == m_nonterminals.end() || !m_nullable[x])
                {
                    flag = false;
                    break;
                }
            }
            if (flag)
            {
                m_nullable[left] = true;
                is_changing = true;
            }
        }
    }
}

void M6::LRParsing::CreateFirstSet()
{
    m_first_set.clear();
    for (auto x : m_nonterminals)
        m_first_set[x] = std::set<Token>();

    for (auto &ex_production : m_expanding_grammar)
    {
        auto left = std::get<0>(ex_production);
        auto right = std::get<1>(ex_production);

        for (auto x : right)
        {
            // 右部为 epsilon
            if (!x.length())
            {
                m_first_set[left].insert(std::wstring(L""));
                break;
            }

            // 右部以终结符开头
            if (m_nonterminals.find(x) == m_nonterminals.end())
            {
                m_first_set[left].insert(x);
                break;
            }

            // 右部为多个产生式

        }
    }
}

void M6::LRParsing::CreateFollowSet()
{
    m_follow_set.clear();
    for (auto x : m_nonterminals)
        m_follow_set[x] = std::set<Token>();
}

void M6::LRParsing::Closure(std::set<Item> &items_set)
{
    for (auto new_set = items_set; new_set.size();)
    {
        auto temp_set = std::set<Item>();

        for (auto &i : new_set)
        {
            auto left = std::get<0>(i);
            auto production = std::get<1>(i);

            for (auto k = static_cast<size_t>(0), size = production.size(); k < size; k++)
            {
                if (k == size - size_t(1))
                    break;

                if (production[k] == m_dot &&
                    m_nonterminals.find(production[k + size_t(1)]) != m_nonterminals.end())
                {
                    auto set = m_original_items[production[k + size_t(1)]];

                    temp_set.insert(set.begin(), set.end());
                    items_set.insert(set.begin(), set.end());

                    break;
                }
            }
        }

        new_set = temp_set;
    }
}

std::set<M6::LRParsing::Item> M6::LRParsing::Go(const std::set<Item> &items_set, const Token &x)
{
    auto re = std::set<Item>();

    for (auto i : items_set)
    {
        auto item = std::get<1>(i);
        for (auto k = size_t(0), size = item.size(); k < size; k++)
        {
            if (k == size - size_t(1))
                break;
            if (item[k] != m_dot)
                continue;
            if (item[k + size_t(1)] != x)
                break;

            auto new_item = item;  // 深拷贝
            new_item[k] = new_item[k + size_t(1)];
            new_item[k + size_t(1)] = m_dot;

            re.insert(std::make_tuple(std::get<0>(i), new_item));
            break;
        }
    }

    Closure(re);
    return re;
}

void M6::LRParsing::BuildItemsSets()
{
    // 1. 求 Closure({S'->·S}) 得到初态项目集
    auto I0 = std::set<Item>{std::make_tuple(m_new_start_token, std::vector<Token>{m_dot, m_start_token})};
    Closure(I0);

    auto count = static_cast<size_t>(0);
    m_items_sets_1.push_back(I0);
    m_items_sets_1_map[I0] = count++;

    // 2. 对初态项目集或其他已构造的项目集，应用状态转移函数 GO(I,x) 求出新的项目集
    // 3. 重复 2 直到不出现新的项目集为止
    auto Q = std::queue<std::set<Item>>();
    Q.push(I0);

    while (!Q.empty())
    {
        auto I = Q.front();
        Q.pop();

        auto index_I = m_items_sets_1_map[I];

        for (auto x : m_alltokens)
        {
            auto D = Go(I, x);

            if (!D.size()) // D 为空则说明 I 不能通过 x 转换，故跳过
                continue;

            if (m_items_sets_1_map.find(D) == m_items_sets_1_map.end())
            {
                m_items_sets_1.push_back(D);
                m_items_sets_1_map[D] = count++;
                Q.push(D);
            }

            m_LR0_DFA[std::make_tuple(index_I, x)] = m_items_sets_1_map[D];
        }
    }
}

void M6::LRParsing::BuildLR0ParsingTable()
{
    for (auto &I_k : m_items_sets_1)
    {
        auto index_I_k = m_items_sets_1_map[I_k];

        // 1. 若项目 A->a·xb 属于 I_k 且 DFA(I_k, x)=I_j 当 x 为终结符时 置 Action[k,x]=s_j
        for (auto x : m_terminals)
        {
            if (m_LR0_DFA.find(std::make_tuple(index_I_k, x)) != m_LR0_DFA.end())
            {
                auto s = std::string("s") + std::to_string(m_LR0_DFA[std::make_tuple(index_I_k, x)]);
                m_LR0_action_table[std::make_tuple(index_I_k, x)].insert(s);
            }
        }

        auto start_item = Item();

        // 2. 若项目 A->a·属于 I_k 则对任何终结符和结束符$(统一记为c) 置 Action[k,c]=r_j (假定 A->a 为文法的第j条规则)
        for (auto &i : m_reduction_items)
        {
            auto reduction_item = i.first;

            if (!i.second)
            {
                start_item = i.first;
                continue;
            }

            if (I_k.find(reduction_item) != I_k.end())
            {
                auto r = std::string("r") + std::to_string(i.second);

                for (auto x : m_terminals)
                    m_LR0_action_table[std::make_tuple(index_I_k, x)].insert(r);

                m_LR0_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert(r);
            }
        }

        // 3. 若 DFA(I_k, x)=I_j 且 x 为非终结符 置 Goto[k,x]=g_j
        for (auto x : m_nonterminals)
        {
            if (m_LR0_DFA.find(std::make_tuple(index_I_k, x)) != m_LR0_DFA.end())
            {
                auto s = std::string("g") + std::to_string(m_LR0_DFA[std::make_tuple(index_I_k, x)]);
                m_LR0_goto_table[std::make_tuple(index_I_k, x)] = s;
            }
        }

        // 4. 若项目 S'->S· 属于 I_k 置 Action[k,$]=acc
        if (I_k.find(start_item) != I_k.end())
        {
            m_LR0_action_table[std::make_tuple(index_I_k, m_end_of_file)].clear();
            m_LR0_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert("acc");
        }

        // 5. 其余置错，为了清晰，不在 ACTION 和 GOTO 中表示
    }

    CreateParsingTable1(m_LR0);
}

void M6::LRParsing::BuildSLR1ParsingTable()
{
    for (auto &I_k : m_items_sets_1)
    {
        auto index_I_k = m_items_sets_1_map[I_k];

        // 1. 若项目 A->a·xb 属于 I_k 且 DFA(I_k, x)=I_j 当 x 为终结符时 置 Action[k,x]=s_j
        for (auto x : m_terminals)
        {
            if (m_LR0_DFA.find(std::make_tuple(index_I_k, x)) != m_LR0_DFA.end())
            {
                auto s = std::string("s") + std::to_string(m_LR0_DFA[std::make_tuple(index_I_k, x)]);
                m_LR0_action_table[std::make_tuple(index_I_k, x)].insert(s);
            }
        }

        auto start_item = Item();

        // 2. 若项目 A->a·属于 I_k 则对任何终结符 a∈FOLLOW(A) 置 Action[k,c]=r_j (假定 A->a 为文法的第j条规则)
        for (auto &i : m_reduction_items)
        {
            auto reduction_item = i.first;

            if (!i.second)
            {
                start_item = i.first;
                continue;
            }

            if (I_k.find(reduction_item) != I_k.end())
            {
                auto r = std::string("r") + std::to_string(i.second);

                for (auto x : m_follow_set[std::get<0>(reduction_item)])
                    m_LR0_action_table[std::make_tuple(index_I_k, x)].insert(r);

                m_LR0_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert(r);
            }
        }

        // 3. 若 DFA(I_k, x)=I_j 且 x 为非终结符 置 Goto[k,x]=g_j
        for (auto x : m_nonterminals)
        {
            if (m_LR0_DFA.find(std::make_tuple(index_I_k, x)) != m_LR0_DFA.end())
            {
                auto s = std::string("g") + std::to_string(m_LR0_DFA[std::make_tuple(index_I_k, x)]);
                m_LR0_goto_table[std::make_tuple(index_I_k, x)] = s;
            }
        }

        // 4. 若项目 S'->S· 属于 I_k 置 Action[k,$]=acc
        if (I_k.find(start_item) != I_k.end())
        {
            m_LR0_action_table[std::make_tuple(index_I_k, m_end_of_file)].clear();
            m_LR0_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert("acc");
        }

        // 5. 其余置错，为了清晰，不在 ACTION 和 GOTO 中表示
    }

    CreateParsingTable1(m_SLR1);
}

void M6::LRParsing::BuildLR1ParsingTable()
{

}

void M6::LRParsing::BuildLALR1ParsingTable()
{

}

void M6::LRParsing::CreateParsingTable1(bool &grammar_flag)
{
    grammar_flag = true;
    m_parsing_table.clear();

    for (auto &i : m_items_sets_1)
    {
        auto index_i = m_items_sets_1_map[i];
        auto index_i_str = std::to_string(index_i);

        for (auto x : m_terminals)
        {
            auto i_x = std::make_tuple(index_i, x);

            if (m_LR0_action_table.find(i_x) == m_LR0_action_table.end())
            {
                m_parsing_table[std::make_tuple(index_i_str, x)] = std::string("");
                continue;
            }

            if (m_LR0_action_table[i_x].size() > size_t(1))  // 大于 1 则说明有冲突
                grammar_flag = false;  // 不是该类型文法

            auto str = std::string("");
            for (auto s : m_LR0_action_table[i_x])
                str += s;

            m_parsing_table[std::make_tuple(index_i_str, x)] = str;
        }

        for (auto x : m_nonterminals)
        {
            auto i_x = std::make_tuple(index_i, x);

            if (m_LR0_goto_table.find(i_x) == m_LR0_goto_table.end())
            {
                m_parsing_table[std::make_tuple(index_i_str, x)] = std::string("");
                continue;
            }

            auto str = std::string("");
            for (auto s : m_LR0_goto_table[i_x])
                str += s;

            m_parsing_table[std::make_tuple(index_i_str, x)] = str;
        }

        auto str = std::string("");
        for (auto s : m_LR0_action_table[std::make_tuple(index_i, m_end_of_file)])
            str += s;
        m_parsing_table[std::make_tuple(index_i_str, m_end_of_file)] = str;
    }
}
