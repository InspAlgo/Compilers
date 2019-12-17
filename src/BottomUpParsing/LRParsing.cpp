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

    BuildItemsSets();

    BuildLR0ParsingTable();

    if (m_LR0)
    {
        CopyParsingTable(m_parsing_table_LR0);
        return;
    }

    BuildSLR1ParsingTable();

    if (m_SLR1)
    {
        CopyParsingTable(m_parsing_table_SLR1);
        return;
    }

    BuildItem2sSets();

    CreateItemsSet1Map2();

    BuildLR1ParsingTable();

    if (m_LR1)
    {
        CopyParsingTable(m_parsing_table_LR1);
        BuildLALR1ParsingTable();
    }

    if (m_LALR1)
    {
        CopyParsingTable(m_parsing_table_LALR1);
    }
}

int M6::LRParsing::GetGrammarType()
{
    return m_LR0 ? 1 : (m_SLR1 ? 2 : (m_LALR1 ? 3 : (m_LR1 ? 4 : 0)));
}

std::set<M6::LRParsing::Token> M6::LRParsing::GetLookAheadTokens(const std::set<M6::LRParsing::Item2> &items_set, const M6::LRParsing::Item &item)
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

        if (production.begin()->length())  // 如果右部不是 epsilon
            production.push_back(m_dot);  // 规约项目即产生式最后面加个 dot 符号
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

void M6::LRParsing::Closure(std::set<Item> &items_set)
{
    auto count = static_cast<size_t>(0);

    for (auto new_set = items_set; count != items_set.size();)
    {
        count = items_set.size();
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

void M6::LRParsing::Closure(std::set<Item2> &items_set)
{
    auto count = static_cast<size_t>(1);
    auto items_set_temp = std::set<Item2>();
    auto item_tokens = std::map<Item, std::set<std::wstring>>();  // LR(1) 项目中 [心] 与 [向前搜索符] 的映射，方便合并 [向前搜索符]

    for (auto new_set = items_set; count != items_set_temp.size();)
    {
        count = items_set_temp.size();
        auto temp_set = std::set<Item2>();

        for (auto &i : new_set)  // 这里的 i 即是一个项目 [A->α·Bβ,a]
        {
            auto left = std::get<0>(i);  // A
            auto production = std::get<1>(i);  // α·Bβ
            auto look_ahead_token = std::get<2>(i);  // 向前查看符 a

            // 合并向前查看符
            item_tokens[std::make_tuple(left, production)].insert(look_ahead_token.begin(), look_ahead_token.end());
            
            items_set_temp.insert(i);

            // 遍历 α·Bβ 定位到 B 的位置
            for (auto k = static_cast<size_t>(0), size = production.size(); k < size; k++)
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

std::set<M6::LRParsing::Item2> M6::LRParsing::Go(const std::set<Item2> &items_set, const Token &x)
{
    auto re = std::set<Item2>();

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
    auto I0 = std::set<Item>{ std::make_tuple(m_new_start_token, std::vector<Token>{m_dot, m_start_token}) };
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
    m_LR0_action_table.clear();
    m_LR0_goto_table.clear();

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
    m_LR0_action_table.clear();
    m_LR0_goto_table.clear();

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

void M6::LRParsing::BuildItem2sSets()
{
    // 1. 求 Closure({[S'->·S,$]}) 得到初态项目集
    auto I0 = std::set<Item2>{ std::make_tuple(m_new_start_token, std::vector<Token>{m_dot, m_start_token}, std::set<Token>{m_end_of_file}) };
    Closure(I0);

    auto count = static_cast<size_t>(0);
    m_items_sets_2.push_back(I0);
    m_items_sets_2_map[I0] = count++;

    // 2. 对初态项目集或其他已构造的项目集，应用状态转移函数 GO(I,x) 求出新的项目集
    // 3. 重复 2 直到不出现新的项目集为止
    auto Q = std::queue<std::set<Item2>>();
    Q.push(I0);

    while (!Q.empty())
    {
        auto I = Q.front();
        Q.pop();

        auto index_I = m_items_sets_2_map[I];

        for (auto x : m_alltokens)
        {
            auto D = Go(I, x);

            if (!D.size()) // D 为空则说明 I 不能通过 x 转换，故跳过
                continue;

            if (m_items_sets_2_map.find(D) == m_items_sets_2_map.end())
            {
                m_items_sets_2.push_back(D);
                m_items_sets_2_map[D] = count++;
                Q.push(D);
            }

            m_LR0_DFA[std::make_tuple(index_I, x)] = m_items_sets_2_map[D];
        }
    }
}

void M6::LRParsing::CreateItemsSet1Map2()
{
    m_items_sets_2_map_1.clear();

    for (auto i = size_t(0); i < m_items_sets_2.size(); i++)
    {
        auto set = std::set<Item>();

        for (auto &item : m_items_sets_2[i])
            set.insert(std::make_tuple(std::get<0>(item), std::get<1>(item)));

        m_items_sets_2_map_1[i] = set;
    }
}

void M6::LRParsing::BuildLR1ParsingTable()
{
    m_LR0_action_table.clear();
    m_LR0_goto_table.clear();

    for (auto &I_k : m_items_sets_2)
    {
        auto index_I_k = m_items_sets_2_map[I_k];

        // 1. 若项目 A->a·xb 属于 I_k 且 DFA(I_k, x)=I_j 当 x 为终结符时 置 Action[k,x]=s_j
        for (auto x : m_terminals)
        {
            if (m_LR0_DFA.find(std::make_tuple(index_I_k, x)) != m_LR0_DFA.end())
            {
                auto s = std::string("s") + std::to_string(m_LR0_DFA[std::make_tuple(index_I_k, x)]);
                m_LR0_action_table[std::make_tuple(index_I_k, x)].insert(s);
            }
        }

        auto start_item = Item2();

        // 2. 若项目 A->a·属于 I_k 则对任何终结符和结束符$(统一记为c) 置 Action[k,c]=r_j (假定 A->a 为文法的第j条规则)
        for (auto &i : m_reduction_items)
        {
            auto reduction_item = i.first;

            if (!i.second)
            {
                start_item = std::make_tuple(std::get<0>(i.first), std::get<1>(i.first), std::set<Token>{m_end_of_file});
                continue;
            }

            auto I_k_temp = m_items_sets_2_map_1[index_I_k];

            if (I_k_temp.find(reduction_item) != I_k_temp.end())
            {
                auto r = std::string("r") + std::to_string(i.second);

                for (auto x : GetLookAheadTokens(I_k, reduction_item))
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

    CreateParsingTable2(m_LR1);
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

void M6::LRParsing::CreateParsingTable2(bool &grammar_flag)
{
    grammar_flag = true;
    m_parsing_table.clear();

    for (auto &i : m_items_sets_2)
    {
        auto index_i = m_items_sets_2_map[i];
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

void M6::LRParsing::CopyParsingTable(std::map<std::tuple<std::string, Token>, std::string> &parsing_table)
{
    parsing_table.clear();
    for (auto &i : m_parsing_table)
        parsing_table[i.first] = i.second;
}
