#include "LR0Parsing.h"
#include <queue>

int M6::LR0Parsing::StateType::None = 0;
int M6::LR0Parsing::StateType::Shift = 1 << 1;
int M6::LR0Parsing::StateType::Reduce = 1 << 2;
int M6::LR0Parsing::StateType::ShiftReduce = 1 << 3;
int M6::LR0Parsing::StateType::ReduceReduce = 1 << 4;

M6::LR0Parsing::LR0Parsing()
{
    m_start_token = L"";
    m_end_of_file = L"$";
    m_dot = L"\xb7";  // 中文「·」
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
    m_start_token = L"";
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
    // m_start_item 赋值改变
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
        for (int i = 0, size = item.size(); i < size; i++)
        {
            if (i == size - 1 || item[i + 1] == m_end_of_file)  // 点号不能是最后一个 token，且下一个不能是 $
                break;
            if (item[i] != m_dot)  // 寻找点号位置
                continue;
            if (item[i + 1] != x)  // 点号后面必须是 x，否则不能经过 x 进行转换
                break;
            
            auto new_item = std::vector<token>();
            for (auto k = 0; k < size; k++)
                new_item.push_back(item[k]);
            
            new_item[i] = new_item[i + 1];
            new_item[i + 1] = m_dot;
            
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

            for (int i = 0, size = item.size(); i < size; i++)
            {
                if (i == size - 1 || item[i + 1] == m_end_of_file)  // 点号不能是最后一个 token，且下一个不能是 $
                    break;
                if (item[i] != m_dot)
                    continue;
                if (m_nonterminal.find(item[i + 1]) == m_nonterminal.end())  // 点号后不是非终结符
                    break;

                auto temp = m_items_start[item[i + 1]];
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
    auto acc = std::make_tuple(m_start_token + L"'", std::vector<token>{ m_start_token, m_dot, m_end_of_file });
    m_productions.push_back(std::make_tuple(m_start_token + L"'", std::vector<token>{ m_start_token, m_end_of_file }));  // 这个是 Accept 状态
    m_production_index[acc] = production_index++;
    
    // 其余在下面代码中合写

    /*
        起始项目 S'->.S$
    */
    m_start_item = std::make_tuple(m_start_token + L"'", 
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
            reduce_production.push_back(m_dot);
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
            for (auto i : m_production_index)
            {
                if (C.find(i.first) == C.end())
                    reduce_count++;
            }

            if (reduce_count)  // 含有规约动作
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
            if (C.size() > 1)  // 含有多个规约动作
                m_state_type[index_C] |= StateType::ReduceReduce;
        }
    }
}

void M6::LR0Parsing::ReData()
{
    for (auto state : m_state_set)
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
                        cell_data += L"r" + std::to_wstring(grammar_index);
                    }
                }
            }

            m_parsing_table[std::make_tuple(state_index, t)] = cell_data;
        }

        // $ 判断
        if (m_state_type[state_index] == (m_state_type[state_index] | StateType::Reduce))  // 只有规约状态
        {
            if (state.size() > 1)
            {
                std::wstring cell_data = L"";
                for (auto i : state)  // 获取所有项目
                {
                    if (m_production_index.find(i) != m_production_index.end())
                    {
                        int grammar_index = m_production_index[i];
                        cell_data += L"r" + std::to_wstring(grammar_index);
                    }
                }
                m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = cell_data;
            }
            else
            {
                int grammar_index = m_production_index[*state.begin()];

                if (grammar_index)  // 不是 0，说明不是 Accept State
                    m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = L"r" + std::to_wstring(grammar_index);
                else
                {
                    m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = L"acc";
                    for (auto i : m_terminal)
                        m_parsing_table[std::make_tuple(state_index, i)] = L"";
                }
            }
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