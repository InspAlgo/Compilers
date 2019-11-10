#include "LL1Parsing.h"

M6::LL1Parsing::LL1Parsing()
{
    m_start_token = L"";
}

void M6::LL1Parsing::SetStartToken(token start_token)
{
    if (start_token.length() > 0)
        m_start_token = start_token;
    else
        throw "ERROR: start_token is null.";
}

void M6::LL1Parsing::AddProduction(const token &production_left, const std::vector<token> &production_right)
{
    // 这里不用进 m_productions 因为后面 CheckGrammar 阶段会重新进
    m_productions_map[production_left].insert(production_right);  // 使用集合，防止重复输入
    m_nonterminal.insert(production_left);
    m_first_s_set[std::make_tuple(production_left, production_right)] = std::set<token>();
}

void M6::LL1Parsing::RunParsing()
{
    CheckGrammar();
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
    m_start_token = L"";
    m_nonterminal.clear();
    m_productions.clear();
    m_nullable.clear();
    m_first_set.clear();
    m_follow_set.clear();
    m_first_s_set.clear();
    m_predictive_parsing_table.clear();

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
    if (m_start_token.length() <= 0)
        return;

    ExtractLeftFactoring();
    RemovingIndirectLeftRecursion();
    RemovingDirectLeftRecursion();
    SimplifyGrammar();
}

void M6::LL1Parsing::ExtractLeftFactoring()
{
    m_productions.clear();

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

        for (auto production : productions_map)
        {
            token N = std::get<0>(production.first);
            m_productions.push_back(std::make_tuple(N, production.second));
            m_productions_map[N].insert(production.second);
        }

        head->DestroyChildNodes();
        delete head;
        head = nullptr;
    }
}

void M6::LL1Parsing::RemovingIndirectLeftRecursion()
{

}

void M6::LL1Parsing::RemovingDirectLeftRecursion()
{

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

    auto new_productions = std::vector<std::tuple<token, std::vector<token>>>();
    auto new_productions_map = std::map<token, std::set<std::vector<token>>>();

    for (auto i : m_productions)
    {
        auto node = std::get<0>(i);
        if (token_closure.find(node) != token_closure.end())
        {
            new_productions.push_back(i);
            new_productions_map[node] = m_productions_map[node];
        }
    }

    m_productions = new_productions;
    m_productions_map = new_productions_map;
}

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

M6::token M6::LL1Parsing::TokenNode::m_temp_token = L"$";
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

M6::token M6::LL1Parsing::TokenNode::GetNewToken()
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
    if (idx >= production.size() && m_left_ptr != nullptr)
    {
        auto end_ptr = this;
        while (end_ptr->m_right_ptr != nullptr && end_ptr->m_token != L"")
            end_ptr = end_ptr->m_right_ptr;

        // 兄弟结点中未出现 ε 结点，则可以添加 ε 结点
        if (end_ptr->m_right_ptr == nullptr && end_ptr->m_token != L"")
            end_ptr->m_right_ptr = new TokenNode(L"");
    }

    // 与本结点值相同，进入子结点找不同
    if (m_token == production[idx])
    {
        // 没有子结点
        if (m_left_ptr == nullptr)
        {
            // production 同时遍历完
            if (idx == production.size() - 1) 
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
            if (ptr->m_token == production[idx])  // 发现相同
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
            if ((idx - 1 >= 0) && ((*i) == production[idx - 1]))
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