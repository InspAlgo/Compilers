#include "LR0Parsing.h"
#include <queue>

const int M6::LR0Parsing::StateType::None = 0;
const int M6::LR0Parsing::StateType::Shift = 1 << 1;
const int M6::LR0Parsing::StateType::Reduce = 1 << 2;
const int M6::LR0Parsing::StateType::ShiftReduce = 1 << 3;
const int M6::LR0Parsing::StateType::ReduceReduce = 1 << 4;

M6::LR0Parsing::LR0Parsing()
{
    m_start_token = L"";
    m_end_of_file = L"$";
    m_dot = L"\x2022";  // 英文 「•」
    m_parsing_stack_cur_data = std::make_tuple(0, L"", L"", L"", L"");
}

void M6::LR0Parsing::SetStartToken(std::wstring start_token)
{
    m_start_token = start_token;
}

void M6::LR0Parsing::SetDot(std::wstring dot)
{
    m_dot = dot;
}

void M6::LR0Parsing::SetEndOfFile(std::wstring end_of_file)
{
    m_end_of_file = end_of_file;
}

void M6::LR0Parsing::AddProduction(const std::wstring &production_left, const std::vector<std::wstring> &production_right)
{
    m_productions_map[production_left].insert(production_right);  // 使用集合，防止重复输入
    
    m_nonterminal.insert(production_left);
    
    for (auto i : production_right)
        m_nt.insert(i);
    m_nt.insert(production_left);
}

void M6::LR0Parsing::RunParsing()
{
    Preprocess();
    Building();
    ReData();
}

void M6::LR0Parsing::Clear()
{
    /*
        构建分析表模块
    */
    m_start_token = L"";
    // m_end_of_file 文件结束符赋值改变
    // m_dot 点号标记赋值改变
    m_nonterminal.clear();
    m_terminal.clear();
    m_nt.clear();
    m_items_start.clear();
    m_state_map.clear();
    m_state_set.clear();
    m_action_table.clear();
    m_goto_table.clear();
    m_state_type.clear();
    m_productions.clear();
    m_productions_map.clear();
    m_production_index.clear();
    m_parsing_table.clear();
    // m_start_item 元组使用赋值改变

    /*
        符号字符串分析模块
    */
    m_next_tokens.clear();
    m_state_index_stack.clear();
    m_token_stack.clear();
    m_parsing_stack_cur_data = std::make_tuple(0, L"", L"", L"", L"");
}

void M6::LR0Parsing::GetGrammar(std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> &productions)
{
    productions.clear();

    for (auto &i : m_productions)
    {
        productions.push_back(i);
    }
}

void M6::LR0Parsing::GetStates(std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &states)
{
    states.clear();
    for (auto &i : m_state_set)
    {
        states.push_back(i);
    }
}

void M6::LR0Parsing::GetColumnsHeader(std::vector<std::wstring> &columns_header)
{
    columns_header.clear();

    for (auto i : m_terminal)
        columns_header.push_back(i);

    columns_header.push_back(m_end_of_file);

    for (auto i : m_nonterminal)
        columns_header.push_back(i);
}

void M6::LR0Parsing::GetParsingTable(std::map<std::tuple<int, std::wstring>, std::wstring> &parsing_table)
{
    parsing_table.clear();
    for (auto &i : m_parsing_table)
    {
        parsing_table[i.first] = i.second;
    }
}

M6::LR0Parsing::item_set M6::LR0Parsing::Goto(item_set set, token x)
{
    auto temp = item_set();

    for (auto item_i : set)  // 对项目集中每一个项目
    {
        // 点号后面是非终结符，添加其带点号的拓展产生式
        auto item = std::get<1>(item_i);
        for (auto i = size_t(0), size = item.size(); i < size; i++)
        {
            if (i == size - size_t(1) || item[i + size_t(1)] == m_end_of_file)  // 点号不能是最后一个 token，且下一个不能是 $
                break;
            if (item[i] != m_dot)  // 寻找点号位置
                continue;
            if (item[i + size_t(1)] != x)  // 点号后面必须是 x，否则不能经过 x 进行转换
                break;
            
            auto new_item = std::vector<token>();
            for (auto k = size_t(0); k < size; k++)
                new_item.push_back(item[k]);
            
            new_item[i] = new_item[i + size_t(1)];
            new_item[i + size_t(1)] = m_dot;
            
            temp.insert(std::make_tuple(std::get<0>(item_i), new_item));
            break;
        }
    }

    Closure(temp);
    return temp;
}

void M6::LR0Parsing::Closure(item_set &set)
{
    auto is_changing = true;
    auto temp_set = set;  // temp_set 是用来指定遍历的集合，下一轮时 temp_set 重新指向新生成的集合，可以避免重复遍历
    while (is_changing)
    {
        auto set_size = set.size();
        auto new_set = item_set();

        for (auto item_i : temp_set)  // 对项目集中每一个项目
        {
            // 点号后面是非终结符，添加其带点号的拓展产生式
            auto item = std::get<1>(item_i);

            for (auto i = size_t(0), size = item.size(); i < size; i++)
            {
                if (i == size - size_t(1) || item[i + size_t(1)] == m_end_of_file)  // 点号不能是最后一个 token，且下一个不能是 $
                    break;
                if (item[i] != m_dot)
                    continue;
                if (m_nonterminal.find(item[i + size_t(1)]) == m_nonterminal.end())  // 点号后不是非终结符
                    break;

                auto temp = m_items_start[item[i + size_t(1)]];
                new_set.insert(temp.begin(), temp.end());
                break;
            }
        }

        set.insert(new_set.begin(), new_set.end());
        is_changing = (set.size() == set_size ? false : true);
        temp_set = new_set;
    }
}

void M6::LR0Parsing::Preprocess()
{
    /*
        求 terminal
    */
    for (auto i : m_nt)
    {
        if (m_nonterminal.find(i) == m_nonterminal.end())
            m_terminal.insert(i);
    }

    /*
        将所有产生式以 vector 列出，并给出在规约状态下的编号
    */
    int production_index = 0;
    auto acc = std::make_tuple(m_start_token + L"\xb4", std::vector<token>{ m_start_token, m_dot, m_end_of_file });
    m_productions.push_back(std::make_tuple(m_start_token + L"\xb4", std::vector<token>{ m_start_token, m_end_of_file }));  // 这个是 Accept 状态  L"\xb4" 是  L'´'
    m_production_index[acc] = production_index++;
    
    // 其余在下面代码中合写

    /*
        起始项目 S'->.S$
    */
    m_start_item = std::make_tuple(m_start_token + L"\xb4",
        std::vector<token>{ m_dot, m_start_token, m_end_of_file });

    /*
        非终结符对应产生式的所有项目初始集合
        如 S->AB|aC|dB，则 m_items_start[S] = { [.AB], [.aC], [.dB] }
           A->ab|dF，则 m_items_start[A] = { [.ab], [.dF] }
    */
    for (auto productions : m_productions_map)
    {
        auto temp_set = item_set();

        for (auto producton : productions.second)
        {
            auto temp_production = std::vector<token>{ m_dot };
            auto reduce_production = std::vector<token>();

            for (auto i : producton)
            {
                temp_production.push_back(i);
                reduce_production.push_back(i);
            }
            
            temp_set.insert(std::make_tuple(productions.first, temp_production));

            m_productions.push_back(std::make_tuple(productions.first, producton));
            reduce_production.push_back(m_dot);  // TODO 需要修改对 epsilon 的处理
            m_production_index[std::make_tuple(productions.first, reduce_production)] = production_index++;
        }

        m_items_start[productions.first] = temp_set;
    }
}

void M6::LR0Parsing::Building()
{
    auto C0 = item_set();
    C0.insert(m_start_item);
    Closure(C0);

    int count = 0;
    m_state_set.push_back(C0);
    m_state_type.push_back(StateType::None);
    m_state_map[C0] = count++;

    auto Q = std::queue<item_set>();
    Q.push(C0);

    while (!Q.empty())
    {
        auto C = Q.front();
        Q.pop();
        
        auto flag_shift = false;  // 假设不可移进
        int index_C = m_state_map[C];

        for (auto x : m_nt)
        {
            auto D = Goto(C, x);

            if (!D.size())  // D 为空则说明 C 不能通过 x 转换，故跳过
            {
                if (m_nonterminal.find(x) == m_nonterminal.end())
                    m_action_table[std::make_tuple(index_C, x)] = -1;  // 标记为负数表示不可转移
                else
                    m_goto_table[std::make_tuple(index_C, x)] = -1;

                continue;
            }

            if (m_state_map.find(D) == m_state_map.end())
            {
                m_state_set.push_back(D);
                m_state_type.push_back(StateType::None);
                m_state_map[D] = count++;
                Q.push(D);
            }

            flag_shift = true;  // 能够发生移进

            if (m_nonterminal.find(x) == m_nonterminal.end())
                m_action_table[std::make_tuple(index_C, x)] = m_state_map[D];
            else
                m_goto_table[std::make_tuple(index_C, x)] = m_state_map[D];
        }

        if (flag_shift)  // 可发生移进动作
        {
            m_state_type[index_C] |= StateType::Shift;
            int reduce_count = 0;  // 可规约的项目数量

            // 判断是否含有规约动作
            for (auto &i : m_production_index)
            {
                if (C.find(i.first) != C.end())
                    reduce_count++;
            }

            if (reduce_count > 0)  // 含有规约动作
            {
                m_state_type[index_C] |= StateType::Reduce;
                m_state_type[index_C] |= StateType::ShiftReduce;
            }

            if (reduce_count > 1)  // 含有多个规约动作
            {
                m_state_type[index_C] |= StateType::ReduceReduce;
            }

        }
        else  // 只有规约动作
        {
            m_state_type[index_C] |= StateType::Reduce;
            if (C.size() > size_t(1))  // 含有多个规约动作
                m_state_type[index_C] |= StateType::ReduceReduce;
        }
    }
}

void M6::LR0Parsing::ReData()
{
    for (auto &state : m_state_set)
    {
        int state_index = m_state_map[state];  // 获取状态索引值，这里的循环是按从小到大的顺序的

        // terminal -- Action 表
        for (auto t : m_terminal)
        {
            std::wstring cell_data = L"";  // 单元格内容

            // 如果是移进动作
            if (m_state_type[state_index] == (m_state_type[state_index] | StateType::Shift))
            {
                int next_index = m_action_table[std::make_tuple(state_index, t)];  // 获取可移进到的下一个状态编号
                cell_data += (next_index < 0 ? L"" : L"s" + std::to_wstring(next_index));
            }

            // 如果是规约动作
            if (m_state_type[state_index] == (m_state_type[state_index] | StateType::Reduce))
            {
                for (auto i : state)  // 获取所有项目
                {
                    if (m_production_index.find(i) != m_production_index.end())
                    {
                        int grammar_index = m_production_index[i];
                        cell_data += (grammar_index > 0 ? L"r" + std::to_wstring(grammar_index) : L"");
                    }
                }
            }

            m_parsing_table[std::make_tuple(state_index, t)] = cell_data;
        }

        // $ 判断
        if (m_state_type[state_index] == (m_state_type[state_index] | StateType::Reduce))  // 有规约状态
        {
            std::wstring cell_data = L"";

            for (auto &i : state)  // 获取所有项目
            {
                if (m_production_index.find(i) != m_production_index.end())
                {
                    int grammar_index = m_production_index[i];
                    cell_data += (grammar_index > 0 ? L"r" + std::to_wstring(grammar_index) : L"acc");
                }
            }

            m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = cell_data;
        }
        else
            m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = L"";

        // nonterminal -- Goto 表
        for (auto t : m_nonterminal)
        {
            std::wstring cell_data = L"";  // 单元格内容

            // 如果是移进动作
            if (m_state_type[state_index] == (m_state_type[state_index] | StateType::Shift))
            {
                int next_index = m_goto_table[std::make_tuple(state_index, t)];  // 获取可移进到的下一个状态编号
                cell_data += (next_index < 0 ? L"" : L"g" + std::to_wstring(next_index));
            }

            m_parsing_table[std::make_tuple(state_index, t)] = cell_data;
        }
    }
}

void M6::LR0Parsing::SetNextTokenInput(const std::vector<std::wstring> &next_tokens)
{
    m_next_tokens.clear();

    m_next_tokens.push_back(m_end_of_file);

    for (auto i = next_tokens.rbegin(); i != next_tokens.rend(); ++i)
        m_next_tokens.push_back(*i);
}

void M6::LR0Parsing::ReNextStep(std::tuple<int, std::wstring, std::wstring, std::wstring, std::wstring> &data)
{
    data = m_parsing_stack_cur_data;
}

M6::LR0Parsing::token M6::LR0Parsing::NextToken()
{
    token re = L"";

    if (m_next_tokens.size())
    {
        re = *m_next_tokens.rbegin();  // 由于使用的是倒序储存，故每次从末尾取符号
        m_next_tokens.pop_back();
    }

    if(!m_next_tokens.size())  // 始终保持有输入串的右界符存在
        m_next_tokens.push_back(m_end_of_file);
    
    return re;
}

int M6::LR0Parsing::NextState()
{
    int re = -1;

    if (m_state_index_stack.size())
        re = *m_state_index_stack.rbegin();

    return re;
}

void M6::LR0Parsing::ParsingStackToStrData(int index, std::wstring parsing_action)
{
    std::wstring state_stack_str = L"";  // 状态栈
    for (auto i : m_state_index_stack)
        state_stack_str += L" " + std::to_wstring(i);

    std::wstring token_stack_str = L"";  // 符号栈
    for (auto i : m_token_stack)
        token_stack_str += L" " + i;

    std::wstring input_tokens_str = L"";  // 输入字符串
    for (auto i = m_next_tokens.rbegin(); i != m_next_tokens.rend(); ++i)
        input_tokens_str += L" " + *i;

    m_parsing_stack_cur_data = std::make_tuple(index, state_stack_str, token_stack_str, input_tokens_str, parsing_action);
}

void M6::LR0Parsing::LR0ParsingInit()
{
    m_state_index_stack.clear();  // 状态栈
    m_token_stack.clear();  // 符号栈

    m_token_stack.push_back(m_end_of_file);  // 与右界符相对应的左界符
    m_state_index_stack.push_back(0);  // 初始状态

    ParsingStackToStrData(0, L"");
}

bool M6::LR0Parsing::RunCurStep()
{
    ParsingStackToStrData(std::get<0>(m_parsing_stack_cur_data) + 1, L"");

    auto next_token = NextToken();  // 没有下一个时，返回 L""
    auto state = NextState();  // 取状态栈栈顶元素，当栈为空时，返回 -1

    if (state < 0)  // 当前状态栈栈顶为空，错误中断
    {
        std::get<4>(m_parsing_stack_cur_data) = L"error";
        return false;
    }
    if (*m_token_stack.rbegin() == m_start_token)  // 当前符号栈栈顶为起始符号，说明规约结束
    {
        std::get<4>(m_parsing_stack_cur_data) = L"acc";
        return false;
    }

    auto temp = m_parsing_table[std::make_tuple(state, next_token)];
    
    if (temp.length() <= 0)  // error
    {
        std::get<4>(m_parsing_stack_cur_data) = L"error";
        return false;  // 错误中断
    }
    else if (temp[0] == L's')  // 移进动作
    {
        std::get<4>(m_parsing_stack_cur_data) = temp;

        m_token_stack.push_back(next_token);
        m_state_index_stack.push_back(std::stoi(temp.substr(1)));
    }
    else if (temp[0] == L'r')  // 规约动作
    {
        std::get<4>(m_parsing_stack_cur_data) = temp;

        auto pr = m_productions[std::stoi(temp.substr(1))];  // production-reducing 规约产生式

        // 移出符号栈中被规约的符号以及对应的状态栈编号
        for (auto len = std::get<1>(pr).size(); len; len--)
        {
            m_token_stack.pop_back();
            m_state_index_stack.pop_back();
        }

        auto s = NextState();
        m_token_stack.push_back(std::get<0>(pr));
        m_state_index_stack.push_back(m_goto_table[std::make_tuple(s, std::get<0>(pr))]);
    }
    else
    {
        std::get<4>(m_parsing_stack_cur_data) = L"error";
        return false;
    }

    return true;  // 结束当前步骤，还有剩余步骤
}