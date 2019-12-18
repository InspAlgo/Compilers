#include "LL1Parsing.h"

M6::LL1Parsing::LL1Parsing()
{
    m_start_token = L"";
    m_end_of_file = L"#";
    TokenNode::SetStaticData(L"$", 0);
}

void M6::LL1Parsing::SetStartToken(std::wstring start_token)
{
    if (start_token.length() > 0)
        m_start_token = start_token;
    // TODO 应有 start_token 为空的警告
}

void M6::LL1Parsing::AddProduction(const std::wstring &production_left, const std::vector<std::wstring> &production_right)
{
    // 这里不用进 m_productions 因为后面 CheckGrammar 阶段会重新进
    m_productions_map[production_left].insert(production_right);  // 使用集合，防止重复输入
    m_nonterminal.insert(production_left);
    m_select_set[std::make_tuple(production_left, production_right)] = std::set<token>();
}

void M6::LL1Parsing::RunParsing()
{
    CheckGrammar();
    InitSet();
    CalcNullable();
    CalcFirstSet();
    CalcFollowSet();
    CalcSelectSet();
    CalcPredictiveParsingTable();
}

void M6::LL1Parsing::GetFirstSet(std::map<std::wstring, std::set<std::wstring>> &first_set)
{
    first_set.clear();
    for (auto i : m_first_set)
        first_set[i.first] = i.second;
}

void M6::LL1Parsing::GetFollowSet(std::map<std::wstring, std::set<std::wstring>> &follow_set)
{
    follow_set.clear();
    for (auto i : m_follow_set)
        follow_set[i.first] = i.second;
}

void M6::LL1Parsing::GetPredictiveParsingTable(std::map<std::tuple<std::wstring, std::wstring>,
    std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &predictive_parsing_table)
{
    predictive_parsing_table.clear();
    for (auto i : m_predictive_parsing_table)
        predictive_parsing_table[i.first] = i.second;
}

void M6::LL1Parsing::GetTerminalSet(std::set<token> &terminal)
{
    terminal.clear();
    for (auto i : m_terminal)
        terminal.insert(i);
}

void M6::LL1Parsing::GetNonterminalSet(std::set<token> &nonterminal)
{
    nonterminal.clear();
    for (auto i : m_nonterminal)
        nonterminal.insert(i);
}

void M6::LL1Parsing::Clear()
{
    m_start_token = L"";
    m_nonterminal.clear();
    m_terminal.clear();
    m_productions.clear();
    m_productions_map.clear();
    m_nullable.clear();
    m_first_set.clear();
    m_follow_set.clear();
    m_select_set.clear();
    m_predictive_parsing_table.clear();
    m_predictive_parsing_table_sharp.clear();

    TokenNode::SetStaticData(L"$", 0);
}

/*
    检查语法:
        提取左因子，消除回溯
        将间接左递归转直接左递归
        消除直接左递归
        化简语法
*/
void M6::LL1Parsing::CheckGrammar()
{
    // 如果没有确定起始非终结符则直接返回
    // TODO 同时应有相关警告
    if (m_start_token.length() <= 0)
        return;

    //    ExtractLeftFactoring();
    //    RemovingIndirectLeftRecursion();
    //    RemovingDirectLeftRecursion();
    //    SimplifyGrammar();

        // 更新 m_productions m_terminal
    m_productions.clear();
    m_terminal.clear();
    auto terminal = std::set<token>();

    for (auto productions : m_productions_map)
    {
        for (auto production : productions.second)
        {
            m_productions.push_back(std::make_tuple(productions.first, production));
            for (auto t : production)  // 遍历每个产生式从而得到所有终结符，但此时是包含有部分非终结符的
                if (t.length() > 0)  // 不包含 epsilon
                    terminal.insert(t);
        }
    }

    // terminal 中是所有产生式的 token 集合，包含非终结符，故求与 nonterminal 的差集
    for (auto i : terminal)
    {
        if (m_nonterminal.find(i) == m_nonterminal.end())
            m_terminal.insert(i);
    }
}

void M6::LL1Parsing::ExtractLeftFactoring()
{
    for (auto &productions : m_productions_map)
    {
        TokenNode* head = new TokenNode(productions.first);

        // 构建公共前缀树(字典树)
        for (auto production : productions.second)
        {
            head->AddDifferentNodes(production, -1);
        }

        productions.second.clear();

        // 对公共前缀树解析
        auto productions_map = std::map<std::tuple<token, int>, std::vector<token>>();
        head->CreateNewProductions(productions.first, -1, productions_map);

        // 更新 map 和 nonterminal
        for (auto production : productions_map)
        {
            token N = std::get<0>(production.first);
            m_productions_map[N].insert(production.second);
            m_nonterminal.insert(N);
        }

        head->DestroyChildNodes();
        delete head;
        head = nullptr;
    }
}

/*
    间接左递归闭包
    如产生式 A->BaC|De, B->Ed|FgH, D->Qw, Q->Ac
    A 能够通过 A=>De=>Qwe=>Acwe 从而有间接左递归的情况
    此时 A 的间接左递归闭包为 [A,D,Q]
*/
bool M6::LL1Parsing::IndirectLeftRecursionTokenClosure(token node, std::vector<token> &token_closure)
{
    if (node == token_closure[0])
        return true;

    // 防止重复遍历
    for (auto i : token_closure)
    {
        if (i == node)
            return false;
    }

    token_closure.push_back(node);

    for (auto production : m_productions_map[node])
    {
        if (m_nonterminal.find(production[0]) != m_nonterminal.end()
            && IndirectLeftRecursionTokenClosure(production[0], token_closure))
        {
            return true;
        }
    }

    token_closure.pop_back();
    return false;
}

/*
    消除间接左递归
    不会引入新的 token
    间接左递归情况比较复杂，有多种可能的循环路径，如:
    0. 起始非终结符只有一条循环路径
       A->B->C->D->A
    1. 起始非终结符有多条路径
       A->B->C->D->A
       A->E->F->G->A
    2. 中间非终结符有多条路径
       A->B->C->D->A
       A->B->C->E->A
    3. 和其他非终结符的循环产生嵌套
       A->B->C->D->A
       B->E->F->C
       B->C->G->H->J->B
*/
void M6::LL1Parsing::RemovingIndirectLeftRecursion()
{
    // TODO
}

/*
    消除直接左递归
    直接左递归如 E->E+T|T，其只和此产生式左部 E 所推导的产生式(E->E+T,E->T)相关，不涉及其他产生式如 T->F|abD
    又本类中储存产生式的结构为 map[token] = set{vector1, vector2, ...}, vector = [token1, token2, ...]
    如 map["E"] = set{vector["E","+","T"], vector["T"]}
       map["T"] = set{vector["T"], vector["a","b","D"]}
    故此方法每次分析一个 map 键值对，其他同理循环遍历即可
    会引入新的 token
    同时直接左递归情况唯一，一个 A 的对应产生式中最多有且只有一个 A->AX，因为之前会提取左因子，
    对于 A->AB|AC 这种直接变成 A->AX,X->B|C
*/
void M6::LL1Parsing::RemovingDirectLeftRecursion()
{
    for (auto &productions : m_productions_map)
    {
        auto including_left = std::vector<token>();
        auto excluding_left = std::set<std::vector<token>>();

        for (auto production : productions.second)
        {
            if (production[0] == productions.first)  // 因为提取了左因子，故形如 A->AX 的左递归最多只有一个式子
                including_left = production;
            else
                excluding_left.insert(production);
        }

        if (!including_left.size())  // 没有左递归 A->a
            continue;
        else if (!excluding_left.size())  // 有左递归，但是没有对应的消除策略 A->Aa
        {
            // TODO 无法消除的左递归
            continue;
        }
        else  // 有左递归，同时可以消除 A->AX|a
        {
            token new_token = productions.first + L"$";  // A -> A$，用 A$ 表示 A'
            m_nonterminal.insert(new_token);  // 更新 nonterminal

            m_productions_map[new_token].insert(std::vector<token>{L""});  // A$->ε
            auto new_production = std::vector<token>();  // 准备 A$->XA$

            // 把 A->AX 变成 A$->XA$
            for (auto i = including_left.begin() + 1; i != including_left.end(); ++i)
                new_production.push_back((*i));
            new_production.push_back(new_token);
            m_productions_map[new_token].insert(new_production);

            productions.second.clear();

            // 把 A->aA$ 插入
            for (auto production : excluding_left)
            {
                if (production[0].length() == 0)
                    production.clear();

                production.push_back(new_token);
                productions.second.insert(production);
            }
        }
    }
}

/*
    化简语法，删除没有用到的产生式
    思路：通过求起始非终结符的闭包，不在闭包内的非终结符的产生式剔除
          起始非终结符，对其产生式各个 token 进行 DFS 遍历，凡能够遍历得到的
          非终结符放进闭包集合中
*/
void M6::LL1Parsing::SimplifyGrammar()
{
    auto token_closure = std::set<token>();
    TokenClosure(m_start_token, token_closure);

    auto new_productions_map = std::map<token, std::set<std::vector<token>>>();

    for (auto pronductions : m_productions_map)
    {
        if (token_closure.find(pronductions.first) != token_closure.end())
            new_productions_map[pronductions.first] = m_productions_map[pronductions.first];
    }

    m_productions_map = new_productions_map;
}

/*
    获得 token 的闭包结果
    如 A->abE|dF, B->cD|t, C->e|r, D->ac, E->E+T|T, F->g, T->t|d, nonterminal={"A","E","F","B","D","C","T"}
    则 "A" 的闭包结果为 {"E","F","T"}，"B" 的闭包结果为 {"B","D"}
*/
void M6::LL1Parsing::TokenClosure(token node, std::set<token> &token_closure)
{
    token_closure.insert(node);

    for (auto production : m_productions_map[node])
    {
        for (auto i : production)
        {
            if (i.length() == 0)  // 跳过推导为 ε 的产生式
                break;

            // 非终结符且不在闭包集合中
            if (m_nonterminal.find(i) != m_nonterminal.end()
                && token_closure.find(i) == token_closure.end())
            {
                TokenClosure(i, token_closure);
            }
        }
    }
}

void M6::LL1Parsing::InitSet()
{
    m_first_set.clear();
    m_follow_set.clear();

    for (auto nonterminal : m_nonterminal)
    {
        m_nullable[nonterminal] = false;
        m_first_set[nonterminal] = std::set<token>();
        m_follow_set[nonterminal] = std::set<token>();
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
            auto left = std::get<0>(production);
            auto right = std::get<1>(production);

            // 如果此非终结符已经判断为可空则跳过
            if (m_nullable[left])
                continue;

            // 如果右部直接就是 ε，说明此非终结符可空
            if (!right.begin()->length())
            {
                m_nullable[left] = true;
                is_changing = true;
                continue;
            }

            // 右部不是 ε，遍历产生式右部每一个符号
            auto flag = true;
            for (auto beta_i : right)
            {
                // 遇到终结符，则此产生式的左部符号是不可空的
                // 遇到非终结符，则判断此非终结符是否可空
                if (m_nonterminal.find(beta_i) == m_nonterminal.end()
                    || m_nullable[beta_i] == false)
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

void M6::LL1Parsing::CalcFirstSet()
{
    auto is_changing = true;  // 标记 FIRST 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto &production : m_productions)
        {
            auto left = std::get<0>(production);
            auto right = std::get<1>(production);

            if (!right.begin()->length()) // epsilon
            {
                m_first_set[left].insert(std::wstring(L""));
                continue;
            }

            auto count = m_first_set[left].size();

            // 右部不是 ε，遍历产生式右部每一个符号
            auto epsilon_count = static_cast<size_t>(0);
            for (auto beta_i : right)
            {
                // 是终结符，即不在非终结符集合中
                if (m_nonterminal.find(beta_i) == m_nonterminal.end())
                {
                    m_first_set[left].insert(beta_i);
                    break;
                }

                // 非终结符不可推导出 epsilon
                if (!m_nullable[beta_i])
                {
                    m_first_set[left].insert(m_first_set[beta_i].begin(), m_first_set[beta_i].end());
                    if (!m_nullable[beta_i])
                        break;
                }
                else
                {
                    epsilon_count++;

                    for (auto i : m_first_set[beta_i])
                    {
                        if (i.length())
                            m_first_set[left].insert(i);
                    }
                }
            }

            if (epsilon_count == right.size())
                m_first_set[left].insert(std::wstring(L""));

            if (m_first_set[left].size() != count)
                is_changing = true;
        }
    }
}

void M6::LL1Parsing::CalcFollowSet()
{
    m_follow_set[m_start_token].insert(m_end_of_file);
    auto is_changing = true;  // 标记 FOLLOW 在一轮迭代中是否发生改变
    while (is_changing)
    {
        is_changing = false;

        for (auto production : m_productions)
        {
            auto right = std::get<1>(production);

            // 右部为 epsilon 直接跳过
            if (!right.begin()->length())
                continue;

            auto left = std::get<0>(production);
            auto count = m_follow_set[left].size();
            auto temp = m_follow_set[left];

            // 右部不是 epsilon，逆序遍历产生式右部每一个符号
            for (auto iter = right.rbegin(); iter != right.rend(); ++iter)
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
                        temp = std::set<token>();

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

void M6::LL1Parsing::CalcSelectSet()
{
    for (auto &production : m_productions)
    {
        auto flag = true;
        auto right = std::get<1>(production);

        for (auto beta_i : right)
        {
            if (!right.begin()->length())
                break;

            if (m_nonterminal.find(beta_i) == m_nonterminal.end())
            {
                m_select_set[production].insert(beta_i);
                flag = false;
                break;
            }

            // 非终结符不可推导出 epsilon
            if (!m_nullable[beta_i])
            {
                m_select_set[production].insert(m_first_set[beta_i].begin(), m_first_set[beta_i].end());
                flag = false;
                break;
            }
            else
            {
                for (auto x : m_first_set[beta_i])
                {
                    if (x.length())
                        m_select_set[production].insert(x);
                }
            }
        }

        if (flag)
        {
            auto temp = m_follow_set[std::get<0>(production)];
            m_select_set[production].insert(temp.begin(), temp.end());
        }
    }
}

void M6::LL1Parsing::CalcPredictiveParsingTable()
{
    for (auto i : m_select_set)
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

M6::LL1Parsing::token M6::LL1Parsing::TokenNode::m_temp_token = L"$";
int M6::LL1Parsing::TokenNode::m_production_idx = 0;

M6::LL1Parsing::TokenNode::TokenNode(token data)
{
    m_token = data;
    m_left_ptr = nullptr;
    m_right_ptr = nullptr;
}

void M6::LL1Parsing::TokenNode::SetStaticData(token temp_token, int idx)
{
    m_temp_token = temp_token;
    m_production_idx = idx;
}

M6::LL1Parsing::token M6::LL1Parsing::TokenNode::GetNewToken()
{
    token new_token = m_temp_token + std::to_wstring(m_production_idx);
    m_production_idx++;
    return new_token;
}

/*
构建公共前缀树也即是字典树
孩子兄弟树表示的产生式 A->acB|acC|bc|f
根结点为 A
    A->^(下面用 ^ 表示 nullptr）
    ↓
    a---------->b----->f->^
    ↓           ↓      ↓
    c->^        c->^   ^
    ↓           ↓
    B--->C->^   ^
    ↓    ↓
    ^    ^
*/
void M6::LL1Parsing::TokenNode::AddDifferentNodes(std::vector<token> &production, int idx)
{
    // 对 head 第一次构建做单独处理
    if (idx == -1 && m_left_ptr == nullptr)
    {
        for (auto i = production.rbegin(); i != production.rend(); ++i)
        {
            TokenNode *node = new TokenNode((*i));
            node->m_left_ptr = m_left_ptr;
            m_left_ptr = node;
        }

        return;
    }

    // 根节点跳过
    if (idx == -1 && m_left_ptr != nullptr)
    {
        m_left_ptr->AddDifferentNodes(production, idx + 1);
        return;
    }

    // production 较先遍历完，故此结点的兄弟结点中添加 ε 结点
    if (size_t(idx) >= production.size() && m_left_ptr != nullptr)
    {
        auto end_ptr = this;
        while (end_ptr->m_right_ptr != nullptr && end_ptr->m_token != L"")
            end_ptr = end_ptr->m_right_ptr;

        // 兄弟结点中未出现 ε 结点，则可以添加 ε 结点
        if (end_ptr->m_right_ptr == nullptr && end_ptr->m_token != L"")
            end_ptr->m_right_ptr = new TokenNode(L"");
    }

    // 与本结点值相同，进入子结点找不同
    if (m_token == production[size_t(idx)])
    {
        // 没有子结点
        if (m_left_ptr == nullptr)
        {
            // production 同时遍历完
            if (idx == int(production.size()) - 1)
                return;

            // production 未遍历完
            m_left_ptr = new TokenNode(L"");
            m_left_ptr->AddDifferentNodes(production, idx + 1);
            return;
        }
        else  // 有子结点
        {
            m_left_ptr->AddDifferentNodes(production, idx + 1);
            return;
        }
    }
    else  // 与本结点值不同
    {
        // 进入兄弟结点找
        for (auto ptr = m_right_ptr; ptr != nullptr; ptr = ptr->m_right_ptr)
        {
            if (ptr->m_token == production[size_t(idx)])  // 发现相同
            {
                ptr->AddDifferentNodes(production, idx);  // 兄弟结点位于同一层次，故 idx 不变
                return;
            }
        }

        // 如果兄弟结点也均不同
        // 找到最后一个兄弟结点
        auto end_ptr = this;
        while (end_ptr->m_right_ptr != nullptr)
            end_ptr = end_ptr->m_right_ptr;

        for (auto i = production.rbegin(); i != production.rend(); ++i)
        {
            if ((idx - 1 >= 0) && ((*i) == production[size_t(idx - 1)]))
                break;

            // 头插法
            TokenNode *node = new TokenNode((*i));
            node->m_left_ptr = end_ptr->m_right_ptr;
            end_ptr->m_right_ptr = node;
        }
    }
}

/*
    删除子结点，包括其兄弟结点也删除
*/
void M6::LL1Parsing::TokenNode::DestroyChildNodes()
{
    if (m_right_ptr != nullptr)
    {
        m_right_ptr->DestroyChildNodes();
        delete m_right_ptr;
        m_right_ptr = nullptr;
    }

    if (m_left_ptr != nullptr)
    {
        m_left_ptr->DestroyChildNodes();
        delete m_left_ptr;
        m_left_ptr = nullptr;
    }
}

/*
孩子兄弟树表示的产生式 A->acB|acC|bc|f
根结点为 A
    A->^(下面用 ^ 表示 nullptr）
    ↓
    a---------->b----->f->^
    ↓           ↓      ↓
    c->^        c->^   ^
    ↓           ↓
    B--->C->^   ^
    ↓    ↓
    ^    ^

类型为 std::map<std::tuple<token, int>, std::vector<token>> 的 productions_map 的存储结果示意如下：
    map[("A",0)] = vector["a","c","$0"]
    map[("A",1)] = vector["b","c"]
    map[("A",2)] = vector["f"]
    map[("$0",0)] = vector["B"]
    map[("$0",1)] = vector["C"]
即 A->ac$0|bc|f, $0->B|C

构造思想：
    对于一个结点，如果其子结点有兄弟结点，说明其之前的父结点们为公共前缀，后面的部分开始不同了，
    于是可以在此结点位置处获得一个新结点作为新的产生式左部 production_left；
    不同的兄弟结点说明是有不同的 vector，故用 idx 进行区分
*/
void M6::LL1Parsing::TokenNode::CreateNewProductions(token production_left, int idx,
    std::map<std::tuple<token, int>, std::vector<token>> &productions_map)
{
    if (idx == -1)  // 单独处理根结点
    {
        m_left_ptr->CreateNewProductions(production_left, idx + 1, productions_map);
        return;
    }

    productions_map[std::make_tuple(production_left, idx)].push_back(m_token);

    if (m_right_ptr != nullptr)
    {
        m_right_ptr->CreateNewProductions(production_left, idx + 1, productions_map);
    }

    if (m_left_ptr != nullptr)
    {
        if (m_left_ptr->m_right_ptr != nullptr)
        {
            token new_token = GetNewToken();
            productions_map[std::make_tuple(production_left, idx)].push_back(new_token);
            m_left_ptr->CreateNewProductions(new_token, 0, productions_map);
        }
        else
        {
            m_left_ptr->CreateNewProductions(production_left, idx, productions_map);
        }
    }
}
