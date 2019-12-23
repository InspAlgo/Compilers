#include "LRParsing.h"
#include <algorithm>
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

M6::LRParsing::~LRParsing()
{
    Clear();
}

void M6::LRParsing::Clear()
{
    m_start_token = L"";
    m_new_start_token = L"";

    m_LR0 = false;
    m_SLR1 = false;
    m_LALR1 = false;
    m_LR1 = false;

    m_original_grammar.clear();

    m_nonterminals.clear();
    m_terminals.clear();
    m_alltokens.clear();
    m_expanding_nonterminals.clear();
    m_expanding_terminals.clear();
    m_expanding_alltokens.clear();

    m_expanding_grammar.clear();
    m_original_items.clear();
    m_reduction_items.clear();

    m_nullable.clear();
    m_first_set.clear();
    m_follow_set.clear();

    m_items_sets_LR0orSLR1.clear();
    m_items_sets_LR0orSLR1_map.clear();

    m_items_sets_LR1.clear();
    m_items_sets_LR1_map.clear();
    m_items_sets_LR1_map_LR0.clear();

    m_items_sets_LALR1.clear();
    m_items_sets_LALR1_map.clear();
    m_items_sets_LALR1_map_LR0.clear();
    m_items_sets_LALR1_map_LR1.clear();
    m_items_sets_LR1_map_LALR1.clear();
   
    m_DFA.clear();
    m_DFA_LR0orSLR1.clear();
    m_DFA_LALR1.clear();
    m_DFA_LR1.clear();

    m_action_table.clear();
    m_goto_table.clear();

    m_parsing_table.clear();
    m_parsing_table_LR0.clear();
    m_parsing_table_SLR1.clear();
    m_parsing_table_LALR1.clear();
    m_parsing_table_LR1.clear();

    m_input_tokens.clear();
    m_state_stack.clear();
    m_tokens_stack.clear();
    m_parsing_process.clear();
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
    if (!production_left.length() || !production_right.size())
        return;

    m_nonterminals.insert(production_left);
    m_original_grammar[production_left].insert(production_right);

    m_alltokens.insert(production_left);
    for (auto i : production_right)
        m_alltokens.insert(i);
}

void M6::LRParsing::BuildLRParsingTable()
{
    Preprocess();

    BuildItemsSets();

    BuildLR0ParsingTable();

    CopyDFA(m_DFA_LR0orSLR1);
    CopyParsingTable(m_parsing_table, m_parsing_table_LR0);

    if (m_LR0)
    {
        m_SLR1 = m_LALR1 = m_LR1 = true;
        return;
    }

    BuildSLR1ParsingTable();

    CopyDFA(m_DFA_LR0orSLR1);
    CopyParsingTable(m_parsing_table, m_parsing_table_SLR1);

    if (m_SLR1)
    {
        m_LALR1 = m_LR1 = true;
        return;
    }

    BuildItem2sSets();

    CreateItemsSet1Map2();

    BuildLR1ParsingTable();

    CopyDFA(m_DFA_LR1);
    CopyParsingTable(m_parsing_table, m_parsing_table_LR1);

    if (m_LR1)
    {
        BuildItem3sSets();

        UpdateDFAofLALR1();

        BuildLALR1ParsingTable();

        CopyDFA(m_DFA_LALR1);
        CopyParsingTable(m_parsing_table, m_parsing_table_LALR1);
    }

    if (!m_LALR1)
        CopyParsingTable(m_parsing_table_LR1, m_parsing_table);
}

void M6::LRParsing::GetExpandingGrammar(std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> &expanding_grammar)
{
    expanding_grammar.clear();
    for (auto &i : m_expanding_grammar)
        expanding_grammar.push_back(i);
}

void M6::LRParsing::GetTerminals(std::vector<std::wstring> &terminals)
{
    terminals.clear();
    for (auto &i : m_terminals)
        terminals.push_back(i);
}

void M6::LRParsing::GetNonterminals(std::vector<std::wstring>& nonterminals)
{
    nonterminals.clear();
    for (auto &i : m_nonterminals)
        nonterminals.push_back(i);
}

std::wstring M6::LRParsing::GetGrammarType()
{
    return m_LR0 ? std::wstring(L"LR(0)") : (m_SLR1 ? std::wstring(L"SLR(1)") : (m_LALR1 ? std::wstring(L"LALR(1)") : (m_LR1 ? std::wstring(L"LR(1)") : std::wstring(L"Not LR Grammar"))));
}

void M6::LRParsing::GetItemsSets(std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &items_sets)
{
    if (!m_LR1)  // 如果是非 LR 文法，不可以调用此类型方法
        return;

    items_sets.clear();

    for (auto &i : m_items_sets_LR0orSLR1)
        items_sets.push_back(i);
}

void M6::LRParsing::GetItemsSets(std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>, std::set<std::wstring>>>> &items_sets)
{
    if (m_LR0 || m_SLR1)
        return;

    items_sets.clear();

    if (m_LALR1)
    {
        for (auto &i : m_items_sets_LALR1)
            items_sets.push_back(i);
    }
    else  // LR(1) 文法或非 LR 文法，均使用 LR(1) 文法的项目集
    {
        for (auto &i : m_items_sets_LR1)
            items_sets.push_back(i);
    }
}

void M6::LRParsing::GetDFA(std::map<std::tuple<size_t, std::wstring>, size_t> &DFA)
{
    DFA.clear();

    if (m_LR0 || m_SLR1)
    {
        for (auto &i : m_DFA_LR0orSLR1)
            DFA[i.first] = i.second;
    }
    else if (m_LALR1)
    {
        for (auto &i : m_DFA_LALR1)
            DFA[i.first] = i.second;
    }
    else  // LR(1) 文法或非 LR 文法，均使用 LR(1) 文法的 DFA
    {
        for (auto &i : m_DFA_LR1)
            DFA[i.first] = i.second;
    }
}

void M6::LRParsing::GetParsingTable(std::map<std::tuple<std::wstring, std::wstring>, std::wstring> &parsing_table)
{
    CopyParsingTable(m_parsing_table, parsing_table);
}

void M6::LRParsing::ParsingTokens(const std::vector<std::wstring> &input_tokens, std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>> &parsing_process)
{
    if (!(m_LR0 || m_SLR1 || m_LALR1 || m_LR1))
        return;

    m_input_tokens.clear();
    parsing_process.clear();

    m_input_tokens.push_back(m_end_of_file);

    for (auto i = input_tokens.rbegin(); i != input_tokens.rend(); ++i)
        m_input_tokens.push_back(*i);

    Control();

    for (auto &i : m_parsing_process)
        parsing_process.push_back(i);
}

std::set<M6::LRParsing::Token> M6::LRParsing::GetLookAheadTokens(const std::set<M6::LRParsing::ItemLR1> &items_set, const M6::LRParsing::ItemLR0 &item)
{
    auto re = std::set<Token>();

    for (auto &i : items_set)
    {
        if (std::make_tuple(std::get<0>(i), std::get<1>(i)) == item)
        {
            re.insert(std::get<2>(i).begin(), std::get<2>(i).end());
            break;
        }
    }

    return re;
}

void M6::LRParsing::Preprocess()
{
    CreateTerminalSet();

    ExpandGrammar();

    CreateOriginalItems();

    CreateReductionItemTable();

    CreateNullable();

    CreateFirstSet();

    CreateFollowSet();
}

void M6::LRParsing::CreateTerminalSet()
{
    m_terminals.clear();

    m_expanding_alltokens = m_alltokens;
    m_expanding_nonterminals = m_nonterminals;
    m_expanding_alltokens.insert(m_new_start_token);
    m_expanding_nonterminals.insert(m_new_start_token);

    for (auto i : m_alltokens)
    {
        // 凡不是非终结符并且不是 epsilon 的符号均为终结符
        if ((m_nonterminals.find(i) == m_nonterminals.end()) && i.length())
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

        if (production.begin()->length())
            new_production.insert(new_production.end(), production.begin(), production.end());

        m_original_items[left].insert(std::make_tuple(left, new_production));
    }
}

void M6::LRParsing::CreateReductionItemTable()
{
    m_reduction_items.clear();

    for (auto i = size_t(0); i < m_expanding_grammar.size(); i++)
    {
        auto left = std::get<0>(m_expanding_grammar[i]);
        auto production = std::get<1>(m_expanding_grammar[i]);

        if (production.begin()->length())  // 如果右部不是 epsilon
            production.push_back(m_dot);  // 归约项目即产生式最后面加个 dot 符号
        else  // 右部是 epsilon
            (*production.begin()) = m_dot;  // 仅有项目 A->·

        m_reduction_items[std::make_tuple(left, production)] = i;
    }
}

void M6::LRParsing::CreateNullable()
{
    m_nullable.clear();
    for (auto x : m_expanding_nonterminals)
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
                if (m_expanding_nonterminals.find(x) == m_expanding_nonterminals.end() || !m_nullable[x])
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
    for (auto x : m_expanding_nonterminals)
        m_first_set[x] = std::set<Token>();

    for (auto is_changing = true; is_changing;)
    {
        is_changing = false;

        for (auto &ex_production : m_expanding_grammar)
        {
            auto left = std::get<0>(ex_production);
            auto right = std::get<1>(ex_production);

            auto count = m_first_set[left].size();

            auto epsilon_count = static_cast<size_t>(0);  // 统计产生式右部可推导出 epsilon 的非终结符数量

            for (auto x : right)
            {
                // 右部为 epsilon
                if (!x.length())
                {
                    m_first_set[left].insert(std::wstring(L""));
                    break;
                }

                // 右部以终结符开头
                if (m_expanding_nonterminals.find(x) == m_expanding_nonterminals.end())
                {
                    m_first_set[left].insert(x);
                    break;
                }

                // 右部形如 y[1]y[2]...y[n] 且 y[1]y[2]...y[i-1] 都是非终结符
                // 如果是非终结符不可空
                if (!m_nullable[x])
                {
                    m_first_set[left].insert(m_first_set[x].begin(), m_first_set[x].end());
                    break;
                }
                else // 可空，将 y[i] 非 epsilon 元素添加进来
                {
                    for (auto i : m_first_set[x])
                    {
                        if (i.length())
                            m_first_set[left].insert(i);
                    }

                    epsilon_count++;
                }
            }

            if (epsilon_count == right.size())  // 如果右部全部可推导出 epsilon 则将 epsilon 添加进来
                m_first_set[left].insert(std::wstring(L""));

            if (m_first_set[left].size() != count)
                is_changing = true;
        }
    }
}

void M6::LRParsing::CreateFollowSet()
{
    m_follow_set.clear();
    for (auto x : m_expanding_nonterminals)
        m_follow_set[x] = std::set<Token>();

    // 对文法的起始符号 S，将 "$" 加到 FOLLOW(S) 中
    m_follow_set[m_new_start_token].insert(m_end_of_file);

    for (auto is_changing = true; is_changing;)
    {
        is_changing = false;

        for (auto &ex_production : m_expanding_grammar)
        {
            auto left = std::get<0>(ex_production);
            auto right = std::get<1>(ex_production);


            if (!right.begin()->length())  // 右部为 epsilon 则直接跳过
                continue;

            auto count = m_follow_set[left].size();
            auto temp = m_follow_set[left];

            for (auto iter = right.rbegin(); iter != right.rend(); ++iter)
            {
                if (m_expanding_nonterminals.find((*iter)) == m_expanding_nonterminals.end())
                    (temp = std::set<Token>()).insert((*iter));
                else
                {
                    auto count2 = m_follow_set[(*iter)].size();

                    m_follow_set[(*iter)].insert(temp.begin(), temp.end());

                    if (m_follow_set[(*iter)].size() != count2)
                        is_changing = true;

                    if (!m_nullable[(*iter)])
                        temp = std::set<Token>();

                    for (auto x : m_first_set[(*iter)])
                    {
                        if (x.length())
                            temp.insert(x);
                    }
                }
            }

            if (m_follow_set[left].size() != count)
                is_changing = true;
        }
    }
}

void M6::LRParsing::Closure(std::set<ItemLR0> &items_set)
{
    auto count = static_cast<size_t>(0);

    for (auto new_set = items_set; count != items_set.size();)
    {
        count = items_set.size();
        auto temp_set = std::set<ItemLR0>();

        for (auto &i : new_set)
        {
            auto left = std::get<0>(i);
            auto production = std::get<1>(i);

            for (auto k = size_t(0), size = production.size(); k < size; k++)
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

void M6::LRParsing::Closure(std::set<ItemLR1> &items_set)
{
    auto count = static_cast<size_t>(1);
    auto items_set_temp = std::set<ItemLR1>();
    auto item_tokens = std::map<ItemLR0, std::set<std::wstring>>();  // LR(1) 项目中 [心] 与 [向前搜索符] 的映射，方便合并 [向前搜索符]

    for (auto new_set = items_set; count != items_set_temp.size();)
    {
        count = items_set_temp.size();
        auto temp_set = std::set<ItemLR1>();

        for (auto &i : new_set)  // 这里的 i 即是一个项目 [A->α·Bβ,a]
        {
            auto left = std::get<0>(i);  // A
            auto production = std::get<1>(i);  // α·Bβ
            auto look_ahead_token = std::get<2>(i);  // 向前查看符 a

            // 合并向前查看符
            item_tokens[std::make_tuple(left, production)].insert(look_ahead_token.begin(), look_ahead_token.end());
            
            items_set_temp.insert(i);

            // 遍历 α·Bβ 定位到 B 的位置
            for (auto k = size_t(0), size = production.size(); k < size; k++)
            {
                if (k == size - size_t(1))
                    break;

                if (production[k] == m_dot &&
                    m_nonterminals.find(production[k + size_t(1)]) != m_nonterminals.end())
                {
                    auto set = m_original_items[production[k + size_t(1)]];
                    auto new_look_ahead_token = std::set<std::wstring>();

                    // 求 FIRST(βa)
                    auto temp_flag = false;
                    for (auto m = k + size_t(2); m < size; m++)
                    {
                        if (m_expanding_nonterminals.find(production[m]) == m_expanding_nonterminals.end())
                        {
                            new_look_ahead_token.insert(production[m]);
                            temp_flag = true;
                            break;
                        }

                        if(!m_nullable[production[m]])
                        {
                            auto x = m_first_set[production[m]];
                            new_look_ahead_token.insert(x.begin(), x.end());
                            temp_flag = true;
                            break;
                        }
                        else
                        {
                            for (auto x : m_first_set[production[m]])
                            {
                                if (x.length())
                                    new_look_ahead_token.insert(x);
                            }
                        }
                    }
                    if (!temp_flag)  // 如果 β =*> ε，则将 a 中的元素添加进来
                        new_look_ahead_token.insert(look_ahead_token.begin(), look_ahead_token.end());

                    for (auto &item : set)
                        temp_set.insert(std::make_tuple(std::get<0>(item), std::get<1>(item), new_look_ahead_token));

                    break;
                }
            }
        }

        new_set = temp_set;
    }

    items_set.clear();
    for (auto &i : item_tokens)
        items_set.insert(std::make_tuple(std::get<0>(i.first), std::get<1>(i.first), i.second));
}

std::set<M6::LRParsing::ItemLR0> M6::LRParsing::Go(const std::set<ItemLR0> &items_set, const Token &x)
{
    auto re = std::set<ItemLR0>();

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

std::set<M6::LRParsing::ItemLR1> M6::LRParsing::Go(const std::set<ItemLR1> &items_set, const Token &x)
{
    auto re = std::set<ItemLR1>();

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

            re.insert(std::make_tuple(std::get<0>(i), new_item, std::get<2>(i)));
            break;
        }
    }

    Closure(re);
    return re;
}

void M6::LRParsing::BuildItemsSets()
{
    // 1. 求 Closure({S'->·S}) 得到初态项目集
    auto I0 = std::set<ItemLR0>{ std::make_tuple(m_new_start_token, std::vector<Token>{m_dot, m_start_token}) };
    Closure(I0);

    auto count = static_cast<size_t>(0);
    m_items_sets_LR0orSLR1.push_back(I0);
    m_items_sets_LR0orSLR1_map[I0] = count++;

    // 2. 对初态项目集或其他已构造的项目集，应用状态转移函数 GO(I,x) 求出新的项目集
    // 3. 重复 2 直到不出现新的项目集为止
    auto Q = std::queue<std::set<ItemLR0>>();
    Q.push(I0);

    while (!Q.empty())
    {
        auto I = Q.front();
        Q.pop();

        auto index_I = m_items_sets_LR0orSLR1_map[I];

        for (auto x : m_alltokens)
        {
            auto D = Go(I, x);

            if (!D.size()) // D 为空则说明 I 不能通过 x 转换，故跳过
                continue;

            if (m_items_sets_LR0orSLR1_map.find(D) == m_items_sets_LR0orSLR1_map.end())
            {
                m_items_sets_LR0orSLR1.push_back(D);
                m_items_sets_LR0orSLR1_map[D] = count++;
                Q.push(D);
            }

            m_DFA[std::make_tuple(index_I, x)] = m_items_sets_LR0orSLR1_map[D];
        }
    }
}

void M6::LRParsing::BuildLR0ParsingTable()
{
    m_action_table.clear();
    m_goto_table.clear();

    for (auto &I_k : m_items_sets_LR0orSLR1)
    {
        auto index_I_k = m_items_sets_LR0orSLR1_map[I_k];

        // 1. 若项目 A->a·xb 属于 I_k 且 DFA(I_k, x)=I_j 当 x 为终结符时 置 Action[k,x]=s_j
        for (auto x : m_terminals)
        {
            if (m_DFA.find(std::make_tuple(index_I_k, x)) != m_DFA.end())
            {
                auto s = std::wstring(L"s") + std::to_wstring(m_DFA[std::make_tuple(index_I_k, x)]);
                m_action_table[std::make_tuple(index_I_k, x)].insert(s);
            }
        }

        auto start_item = ItemLR0();

        // 2. 若项目 A->a·属于 I_k 则对任何终结符和结束符$(统一记为c) 置 Action[k,c]=r_j (假定 A->a 为文法的第j条规则)
        for (auto &i : m_reduction_items)
        {
            auto reduction_item = i.first;

            if (!i.second)  // 找到 S'->S· 项目
            {
                start_item = i.first;
                continue;
            }

            if (I_k.find(reduction_item) != I_k.end())
            {
                auto r = std::wstring(L"r") + std::to_wstring(i.second);

                for (auto x : m_terminals)
                    m_action_table[std::make_tuple(index_I_k, x)].insert(r);

                m_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert(r);
            }
        }

        // 3. 若 DFA(I_k, x)=I_j 且 x 为非终结符 置 Goto[k,x]=g_j
        for (auto x : m_nonterminals)
        {
            if (m_DFA.find(std::make_tuple(index_I_k, x)) != m_DFA.end())
            {
                auto s = std::wstring(L"g") + std::to_wstring(m_DFA[std::make_tuple(index_I_k, x)]);
                m_goto_table[std::make_tuple(index_I_k, x)] = s;
            }
        }

        // 4. 若项目 S'->S· 属于 I_k 置 Action[k,$]=acc
        if (I_k.find(start_item) != I_k.end())
        {
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].clear();
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert(std::wstring(L"acc"));
        }

        // 5. 其余置错，为了清晰，不在 ACTION 和 GOTO 中表示
    }

    CreateParsingTable(m_LR0);
}

void M6::LRParsing::BuildSLR1ParsingTable()
{
    m_action_table.clear();
    m_goto_table.clear();

    for (auto &I_k : m_items_sets_LR0orSLR1)
    {
        auto index_I_k = m_items_sets_LR0orSLR1_map[I_k];

        // 1. 若项目 A->a·xb 属于 I_k 且 DFA(I_k, x)=I_j 当 x 为终结符时 置 Action[k,x]=s_j
        for (auto x : m_terminals)
        {
            if (m_DFA.find(std::make_tuple(index_I_k, x)) != m_DFA.end())
            {
                auto s = std::wstring(L"s") + std::to_wstring(m_DFA[std::make_tuple(index_I_k, x)]);
                m_action_table[std::make_tuple(index_I_k, x)].insert(s);
            }
        }

        auto start_item = ItemLR0();

        // 2. 若项目 A->a·属于 I_k 则对任何终结符 a∈FOLLOW(A) 置 Action[k,c]=r_j (假定 A->a 为文法的第j条规则)
        for (auto &i : m_reduction_items)
        {
            auto reduction_item = i.first;

            if (!i.second)  // 找到 S'->S· 项目
            {
                start_item = i.first;
                continue;
            }

            if (I_k.find(reduction_item) != I_k.end())
            {
                auto r = std::wstring(L"r") + std::to_wstring(i.second);

                for (auto x : m_follow_set[std::get<0>(reduction_item)])
                    m_action_table[std::make_tuple(index_I_k, x)].insert(r);
            }
        }

        // 3. 若 DFA(I_k, x)=I_j 且 x 为非终结符 置 Goto[k,x]=g_j
        for (auto x : m_nonterminals)
        {
            if (m_DFA.find(std::make_tuple(index_I_k, x)) != m_DFA.end())
            {
                auto s = std::wstring(L"g") + std::to_wstring(m_DFA[std::make_tuple(index_I_k, x)]);
                m_goto_table[std::make_tuple(index_I_k, x)] = s;
            }
        }

        // 4. 若项目 S'->S· 属于 I_k 置 Action[k,$]=acc
        if (I_k.find(start_item) != I_k.end())
        {
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].clear();
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert(std::wstring(L"acc"));
        }

        // 5. 其余置错，为了清晰，不在 ACTION 和 GOTO 中表示
    }

    CreateParsingTable(m_SLR1);
}

void M6::LRParsing::BuildItem2sSets()
{
    // 1. 求 Closure({[S'->·S,$]}) 得到初态项目集
    auto I0 = std::set<ItemLR1>{ std::make_tuple(m_new_start_token, std::vector<Token>{m_dot, m_start_token}, std::set<Token>{m_end_of_file}) };
    Closure(I0);

    auto count = static_cast<size_t>(0);
    m_items_sets_LR1.push_back(I0);
    m_items_sets_LR1_map[I0] = count++;

    // 2. 对初态项目集或其他已构造的项目集，应用状态转移函数 GO(I,x) 求出新的项目集
    // 3. 重复 2 直到不出现新的项目集为止
    auto Q = std::queue<std::set<ItemLR1>>();
    Q.push(I0);

    while (!Q.empty())
    {
        auto I = Q.front();
        Q.pop();

        auto index_I = m_items_sets_LR1_map[I];

        for (auto x : m_alltokens)
        {
            auto D = Go(I, x);

            if (!D.size()) // D 为空则说明 I 不能通过 x 转换，故跳过
                continue;

            if (m_items_sets_LR1_map.find(D) == m_items_sets_LR1_map.end())
            {
                m_items_sets_LR1.push_back(D);
                m_items_sets_LR1_map[D] = count++;
                Q.push(D);
            }

            m_DFA[std::make_tuple(index_I, x)] = m_items_sets_LR1_map[D];
        }
    }
}

void M6::LRParsing::CreateItemsSet1Map2()
{
    m_items_sets_LR1_map_LR0.clear();

    for (auto i = size_t(0); i < m_items_sets_LR1.size(); i++)
    {
        auto set = std::set<ItemLR0>();

        for (auto &item : m_items_sets_LR1[i])
            set.insert(std::make_tuple(std::get<0>(item), std::get<1>(item)));

        m_items_sets_LR1_map_LR0[i] = set;
    }
}

void M6::LRParsing::BuildLR1ParsingTable()
{
    m_action_table.clear();
    m_goto_table.clear();

    for (auto &I_k : m_items_sets_LR1)
    {
        auto index_I_k = m_items_sets_LR1_map[I_k];

        // 1. 若项目 A->a·xb 属于 I_k 且 DFA(I_k, x)=I_j 当 x 为终结符时 置 Action[k,x]=s_j
        for (auto x : m_terminals)
        {
            if (m_DFA.find(std::make_tuple(index_I_k, x)) != m_DFA.end())
            {
                auto s = std::wstring(L"s") + std::to_wstring(m_DFA[std::make_tuple(index_I_k, x)]);
                m_action_table[std::make_tuple(index_I_k, x)].insert(s);
            }
        }

        auto start_item = ItemLR1();

        // 2. 若归约项目 [A->α·,a] 属于 I_k 则向前查看符 a 置 Action[k,a]=r_j (假定 A->α 为文法的第j条规则)
        for (auto &i : m_reduction_items)
        {
            auto reduction_item = i.first;

            if (!i.second)  // 找到 S'->S· 项目
            {
                start_item = std::make_tuple(std::get<0>(i.first), std::get<1>(i.first), std::set<Token>{m_end_of_file});
                continue;
            }

            auto I_k_temp = m_items_sets_LR1_map_LR0[index_I_k];

            if (I_k_temp.find(reduction_item) != I_k_temp.end())
            {
                auto r = std::wstring(L"r") + std::to_wstring(i.second);

                for (auto x : GetLookAheadTokens(I_k, reduction_item))
                    m_action_table[std::make_tuple(index_I_k, x)].insert(r);
            }
        }

        // 3. 若 DFA(I_k, x)=I_j 且 x 为非终结符 置 Goto[k,x]=g_j
        for (auto x : m_nonterminals)
        {
            if (m_DFA.find(std::make_tuple(index_I_k, x)) != m_DFA.end())
            {
                auto s = std::wstring(L"g") + std::to_wstring(m_DFA[std::make_tuple(index_I_k, x)]);
                m_goto_table[std::make_tuple(index_I_k, x)] = s;
            }
        }

        // 4. 若项目 S'->S· 属于 I_k 置 Action[k,$]=acc
        if (I_k.find(start_item) != I_k.end())
        {
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].clear();
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert(std::wstring(L"acc"));
        }

        // 5. 其余置错，为了清晰，不在 ACTION 和 GOTO 中表示
    }

    CreateParsingTable(m_LR1, m_items_sets_LR1, m_items_sets_LR1_map);
}

void M6::LRParsing::MergeLookAheadTokens(const std::set<size_t> &same_cores_set, const size_t &index)
{
    auto item_map_tokens = std::map<ItemLR0, std::set<Token>>();
    auto items_set = std::set<ItemLR0>();
    auto items_set_3 = std::set<ItemLR1>();

    for (auto i : same_cores_set)
    {
        m_items_sets_LR1_map_LALR1[i] = index;

        for (auto &item2 : m_items_sets_LR1[i])
        {
            auto left = std::get<0>(item2);
            auto item = std::get<1>(item2);
            auto look_ahead_tokens = std::get<2>(item2);

            item_map_tokens[std::make_tuple(left,item)].insert(look_ahead_tokens.begin(),look_ahead_tokens.end());
        }
    }

    for (auto &i : item_map_tokens)
    {
        auto left = std::get<0>(i.first);
        auto item = std::get<1>(i.first);
        auto look_ahead_tokens = i.second;

        items_set.insert(std::make_tuple(left, item));
        items_set_3.insert(std::make_tuple(left, item, look_ahead_tokens));
    }

    m_items_sets_LALR1.push_back(items_set_3);  // index 是按从小到大的顺序的的，所以可以直接添加
    m_items_sets_LALR1_map[items_set_3] = index;
    m_items_sets_LALR1_map_LR0[index] = items_set;
    m_items_sets_LALR1_map_LR1[index] = same_cores_set;
}

void M6::LRParsing::BuildItem3sSets()
{
    m_items_sets_LALR1.clear();
    m_items_sets_LALR1_map.clear();
    m_items_sets_LALR1_map_LR0.clear();
    m_items_sets_LALR1_map_LR1.clear();
    m_items_sets_LR1_map_LALR1.clear();

    auto temp_sets_map = std::map<std::set<ItemLR0>, std::set<size_t>>();  // 同心集对应的 LR(1) 项目集
    auto temp_sets = std::vector<std::set<size_t>>();  // LALR(1) 项目集，以 LR(1) 项目集编号形式表示
    auto temp_map = std::map<size_t, std::set<size_t>>();  // temp_sets 中 LALR(1) 的位置映射
    auto temp_sort = std::vector<size_t>();  // 供 LALR(1) 项目集排序使用，结果以从小到大表示

    for (auto &i : m_items_sets_LR1_map_LR0)
        temp_sets_map[i.second].insert(i.first);

    // 向 temp_sort 中添加同心集中 LR(1) 项目集编号最小的那个编号
    for (auto &i : temp_sets_map)
    {
        auto min_index = *i.second.begin();

        for (auto index : i.second)
        {
            if (min_index > index)
                min_index = index;
        }

        temp_map[min_index].insert(i.second.begin(), i.second.end());
        temp_sort.push_back(min_index);
    }

    std::sort(temp_sort.begin(), temp_sort.end());  // 从小到大排序

    for (auto i = size_t(0); i < temp_sort.size(); i++)
    {
        MergeLookAheadTokens(temp_map[temp_sort[i]], i);
    }
}

void M6::LRParsing::UpdateDFAofLALR1()
{
    for (auto &i : m_DFA)
    {
        auto index_from = m_items_sets_LR1_map_LALR1[std::get<0>(i.first)];
        auto token = std::get<1>(i.first);

        m_DFA_LALR1[std::make_tuple(index_from, token)] = m_items_sets_LR1_map_LALR1[i.second];
    }
}

void M6::LRParsing::BuildLALR1ParsingTable()
{
    m_action_table.clear();
    m_goto_table.clear();

    for (auto &I_k : m_items_sets_LALR1)
    {
        auto index_I_k = m_items_sets_LALR1_map[I_k];

        // 1. 若项目 A->a·xb 属于 I_k 且 DFA(I_k, x)=I_j 当 x 为终结符时 置 Action[k,x]=s_j
        for (auto x : m_terminals)
        {
            if (m_DFA_LALR1.find(std::make_tuple(index_I_k, x)) != m_DFA_LALR1.end())
            {
                auto s = std::wstring(L"s") + std::to_wstring(m_DFA_LALR1[std::make_tuple(index_I_k, x)]);
                m_action_table[std::make_tuple(index_I_k, x)].insert(s);
            }
        }

        auto start_item = ItemLR1();

        // 2. 若归约项目 [A->α·,a] 属于 I_k 则向前查看符 a 置 Action[k,a]=r_j (假定 A->α 为文法的第j条规则)
        for (auto &i : m_reduction_items)
        {
            auto reduction_item = i.first;

            if (!i.second)  // 找到 S'->S· 项目
            {
                start_item = std::make_tuple(std::get<0>(i.first), std::get<1>(i.first), std::set<Token>{m_end_of_file});
                continue;
            }

            auto I_k_temp = m_items_sets_LALR1_map_LR0[index_I_k];

            if (I_k_temp.find(reduction_item) != I_k_temp.end())
            {
                auto r = std::wstring(L"r") + std::to_wstring(i.second);

                for (auto x : GetLookAheadTokens(I_k, reduction_item))
                    m_action_table[std::make_tuple(index_I_k, x)].insert(r);
            }
        }

        // 3. 若 DFA(I_k, x)=I_j 且 x 为非终结符 置 Goto[k,x]=g_j
        for (auto x : m_nonterminals)
        {
            if (m_DFA_LALR1.find(std::make_tuple(index_I_k, x)) != m_DFA_LALR1.end())
            {
                auto s = std::wstring(L"g") + std::to_wstring(m_DFA_LALR1[std::make_tuple(index_I_k, x)]);
                m_goto_table[std::make_tuple(index_I_k, x)] = s;
            }
        }

        // 4. 若项目 S'->S· 属于 I_k 置 Action[k,$]=acc
        if (I_k.find(start_item) != I_k.end())
        {
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].clear();
            m_action_table[std::make_tuple(index_I_k, m_end_of_file)].insert(std::wstring(L"acc"));
        }

        // 5. 其余置错，为了清晰，不在 ACTION 和 GOTO 中表示
    }

    CreateParsingTable(m_LALR1, m_items_sets_LALR1, m_items_sets_LALR1_map);
}

void M6::LRParsing::CreateParsingTable(bool &grammar_flag)
{
    grammar_flag = true;
    m_parsing_table.clear();

    for (auto &i : m_items_sets_LR0orSLR1)
    {
        auto index_i = m_items_sets_LR0orSLR1_map[i];
        auto index_i_str = std::to_wstring(index_i);

        for (auto x : m_terminals)
        {
            auto i_x = std::make_tuple(index_i, x);

            if (m_action_table.find(i_x) == m_action_table.end())
            {
                // 为了分析表清晰，error 部分使用空格填充
                m_parsing_table[std::make_tuple(index_i_str, x)] = std::wstring(L" ");
                continue;
            }

            if (m_action_table[i_x].size() > size_t(1))  // 大于 1 则说明有冲突
                grammar_flag = false;  // 不是该类型文法

            auto str = std::wstring(L"");
            for (auto s : m_action_table[i_x])
                str += s;

            m_parsing_table[std::make_tuple(index_i_str, x)] = str;
        }

        for (auto x : m_nonterminals)
        {
            auto i_x = std::make_tuple(index_i, x);

            if (m_goto_table.find(i_x) == m_goto_table.end())
            {
                // 为了分析表清晰，error 部分使用空格填充
                m_parsing_table[std::make_tuple(index_i_str, x)] = std::wstring(L" ");
                continue;
            }

            auto str = std::wstring(L"");
            for (auto s : m_goto_table[i_x])
                str += s;

            m_parsing_table[std::make_tuple(index_i_str, x)] = str;
        }

        auto str = std::wstring(L"");
        for (auto s : m_action_table[std::make_tuple(index_i, m_end_of_file)])
            str += s;
        
        if (m_action_table[std::make_tuple(index_i, m_end_of_file)].size() > size_t(1))  // 大于 1 则说明有冲突
            grammar_flag = false;  // 不是该类型文法

        m_parsing_table[std::make_tuple(index_i_str, m_end_of_file)] = str;
    }
}

void M6::LRParsing::CreateParsingTable(bool &grammar_flag, const std::vector<std::set<ItemLR1>> &items_sets, const std::map<std::set<ItemLR1>, size_t> &items_sets_map)
{
    grammar_flag = true;
    m_parsing_table.clear();

    auto items_sets_temp = items_sets;
    auto items_sets_map_temp = items_sets_map;

    for (auto &i : items_sets_temp)
    {
        auto index_i = items_sets_map_temp[i];
        auto index_i_str = std::to_wstring(index_i);

        for (auto x : m_terminals)
        {
            auto i_x = std::make_tuple(index_i, x);

            if (m_action_table.find(i_x) == m_action_table.end())
            {
                // 为了分析表清晰，error 部分使用空格填充
                m_parsing_table[std::make_tuple(index_i_str, x)] = std::wstring(L" ");
                continue;
            }

            if (m_action_table[i_x].size() > size_t(1))  // 大于 1 则说明有冲突
                grammar_flag = false;  // 不是该类型文法

            auto str = std::wstring(L"");
            for (auto s : m_action_table[i_x])
                str += s;

            m_parsing_table[std::make_tuple(index_i_str, x)] = str;
        }

        for (auto x : m_nonterminals)
        {
            auto i_x = std::make_tuple(index_i, x);

            if (m_goto_table.find(i_x) == m_goto_table.end())
            {
                // 为了分析表清晰，error 部分使用空格填充
                m_parsing_table[std::make_tuple(index_i_str, x)] = std::wstring(L" ");
                continue;
            }

            auto str = std::wstring(L"");
            for (auto s : m_goto_table[i_x])
                str += s;

            m_parsing_table[std::make_tuple(index_i_str, x)] = str;
        }

        auto str = std::wstring(L"");
        for (auto s : m_action_table[std::make_tuple(index_i, m_end_of_file)])
            str += s;

        if (m_action_table[std::make_tuple(index_i, m_end_of_file)].size() > size_t(1))  // 大于 1 则说明有冲突
            grammar_flag = false;  // 不是该类型文法

        m_parsing_table[std::make_tuple(index_i_str, m_end_of_file)] = str;
    }
}

void M6::LRParsing::CopyDFA(std::map<std::tuple<size_t, Token>, size_t> &DFA)
{
    DFA.clear();
    for (auto &i : m_DFA)
        DFA[i.first] = i.second;
}

void M6::LRParsing::CopyParsingTable(std::map<std::tuple<std::wstring, Token>, std::wstring> &from_parsing_table, std::map<std::tuple<std::wstring, Token>, std::wstring> &to_parsing_table)
{
    to_parsing_table.clear();
    for (auto &i : from_parsing_table)
        to_parsing_table[i.first] = i.second;
}

void M6::LRParsing::Control()
{
    m_state_stack.clear();
    m_tokens_stack.clear();
    m_parsing_process.clear();

    m_state_stack.push_back(std::wstring(L"0"));
    m_tokens_stack.push_back(m_end_of_file);

    for (auto step_count = 1; RunCurStep(step_count); step_count++)
        m_parsing_process.push_back(m_cur_parsing_data);

    m_parsing_process.push_back(m_cur_parsing_data);
}

bool M6::LRParsing::RunCurStep(int step_count)
{
    ParsingStackToString(step_count, std::wstring(L""));

    auto S = m_state_stack.back();  // 获取当前状态栈栈顶元素
    auto a = m_input_tokens.back();  // 获取当前输入串的首字符

    auto temp = m_parsing_table[std::make_tuple(S, a)];

    if (m_tokens_stack.back() == m_start_token)  // 当前符号栈栈顶元素为起始符号，说明输入符号串全部接受成功
    {
        std::get<4>(m_cur_parsing_data) = std::wstring(L"acc");
        return false;
    }

    if (!temp.length())
    {
        std::get<4>(m_cur_parsing_data) = std::wstring(L"error");
        return false;
    }
    else if (temp.front() == char('s'))  // parsing_table[S,a] == s_i 移进动作
    {
        // 将状态 i 和输入符号 a 进栈
        m_state_stack.push_back(temp.substr(size_t(1)));
        m_tokens_stack.push_back(a);

        m_input_tokens.pop_back();  // 发生移进动作，故将输入字符串的首字符弹出
        std::get<4>(m_cur_parsing_data) = temp;
    }
    else if (temp.front() == char('r'))  // parsing_table[S,a] == r_j 归约动作
    {
        // 用第 j 条规则 A->α 归约
        auto production = m_expanding_grammar[std::stoull(temp.substr(size_t(1)))];

        // 将 |α| 个状态和 |α| 个输入符号退栈
        for (auto len = std::get<1>(production).size(); len; len--)
        {
            m_state_stack.pop_back();
            m_tokens_stack.pop_back();
        }

        // 当前栈顶状态为 S'，将 A 和 parsing_table[S',A]=S" 进栈
        S = m_state_stack.back();  // S'
        m_tokens_stack.push_back(std::get<0>(production));
        m_state_stack.push_back(m_parsing_table[std::make_tuple(S, std::get<0>(production))].substr(size_t(1)));

        auto parsing_action = temp + std::wstring(L" ") + std::get<0>(production) + std::wstring(L" ->");
        for (auto i : std::get<1>(production))
            parsing_action += std::wstring(L" ") + i;
        std::get<4>(m_cur_parsing_data) = parsing_action;
    }
    else
    {
        std::get<4>(m_cur_parsing_data) = std::wstring(L"error");
        return false;
    }

    return true;
}

void M6::LRParsing::ParsingStackToString(int step_count, std::wstring parsing_action)
{
    auto state_stack_str = std::wstring(L"");
    for (auto i : m_state_stack)
        state_stack_str += i + std::wstring(L" ");

    auto tokens_stack_str = std::wstring(L"");
    for (auto i : m_tokens_stack)
        tokens_stack_str += i + std::wstring(L" ");

    auto input_tokens_str = std::wstring(L"");
    for (auto i = m_input_tokens.rbegin(); i != m_input_tokens.rend(); ++i)
        input_tokens_str += (*i) + std::wstring(L" ");

    m_cur_parsing_data = std::make_tuple(std::to_wstring(step_count), state_stack_str, tokens_stack_str, input_tokens_str, parsing_action);
}
