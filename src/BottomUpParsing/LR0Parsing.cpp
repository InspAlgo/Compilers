#include "LR0Parsing.h"
#include <queue>

M6::LR0Parsing::LR0Parsing()
{
    m_start_token = L"";
    m_end_of_file = L"$";
    m_dot = L"\xb7";  // 中文「·」
}

void M6::LR0Parsing::SetStartToken(token start_token)
{
    m_start_token = start_token;
}

void M6::LR0Parsing::SetDot(token dot)
{
    m_dot = dot;
}

void M6::LR0Parsing::SetEndOfFile(token end_of_file)
{
    m_end_of_file = end_of_file;
}

void M6::LR0Parsing::AddProduction(const token &production_left, const std::vector<token> &production_right)
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
}

void M6::LR0Parsing::Clear()
{
    m_start_token = L"";
    m_end_of_file = L"$";
    m_dot = L"\xb7";  // 中文「·」
    m_nonterminal.clear();
    m_terminal.clear();
    m_nt.clear();
    m_items_start.clear();
    m_state_map.clear();
    m_state_set.clear();
    m_action_table.clear();
    m_goto_table.clear();
    m_reduce_state.clear();
}

M6::item_set M6::LR0Parsing::Goto(M6::item_set set, M6::token x)
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

void M6::LR0Parsing::Closure(M6::item_set &set)
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
    m_state_map[C0] = count++;

    auto Q = std::queue<item_set>();
    Q.push(C0);

    while (!Q.empty())
    {
        auto C = Q.front();
        Q.pop();
        
        auto flag_reduce = true;

        for (auto x : m_nt)
        {
            auto D = Goto(C, x);

            if (!D.size())  // D 为空则说明 C 不能通过 x 转换，故跳过
                continue;

            if (m_state_map.find(D) == m_state_map.end())
            {
                m_state_set.push_back(D);
                m_state_map[D] = count++;
                Q.push(D);
            }

            flag_reduce = false;  // 能够发生转移，说明是非规约态

            if (m_nonterminal.find(x) != m_nonterminal.end())
                m_action_table[std::make_tuple(m_state_map[C], x)] = m_state_map[D];
            else
                m_goto_table[std::make_tuple(m_state_map[C], x)] = m_state_map[D];
        }

        if (flag_reduce)
            m_reduce_state.insert(m_state_map[C]);
    }
}