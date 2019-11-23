#include "LR0Parsing.h"
#include <queue>

M6::LR0Parsing::LR0Parsing()
{
    m_start_token = L"";
    m_end_of_file = L"$";
    m_dot = L"\xb7";  // ���ġ�����
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
    m_productions_map[production_left].insert(production_right);  // ʹ�ü��ϣ���ֹ�ظ�����
    
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
    m_end_of_file = L"$";
    m_dot = L"\xb7";  // ���ġ�����
    m_nonterminal.clear();
    m_terminal.clear();
    m_nt.clear();
    m_items_start.clear();
    m_state_map.clear();
    m_state_set.clear();
    m_action_table.clear();
    m_goto_table.clear();
    m_reduce_state.clear();
    m_productions.clear();
    m_productions_map.clear();
    m_production_index.clear();
    m_parsing_table.clear();
    // m_start_item ��ֵ�ı�
}

void M6::LR0Parsing::GetGrammar(std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> &productions)
{
    productions.clear();

}

void M6::LR0Parsing::GetStates(std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &states)
{
    states.clear();
    for (auto i : m_state_set)
    {
        states.push_back(i);
    }
}

void M6::LR0Parsing::GetParsingTable(std::map<std::tuple<int, std::wstring>, std::wstring> &parsing_table)
{
    parsing_table.clear();
    for (auto i : m_parsing_table)
    {
        parsing_table[i.first] = i.second;
    }
}

M6::LR0Parsing::item_set M6::LR0Parsing::Goto(item_set set, token x)
{
    auto temp = item_set();

    for (auto item_i : set)  // ����Ŀ����ÿһ����Ŀ
    {
        // ��ź����Ƿ��ս������������ŵ���չ����ʽ
        auto item = std::get<1>(item_i);
        for (int i = 0, size = item.size(); i < size; i++)
        {
            if (i == size - 1 || item[i + 1] == m_end_of_file)  // ��Ų��������һ�� token������һ�������� $
                break;
            if (item[i] != m_dot)  // Ѱ�ҵ��λ��
                continue;
            if (item[i + 1] != x)  // ��ź�������� x�������ܾ��� x ����ת��
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
    auto temp_set = set;  // temp_set ������ָ�������ļ��ϣ���һ��ʱ temp_set ����ָ�������ɵļ��ϣ����Ա����ظ�����
    while (is_changing)
    {
        auto set_size = set.size();
        auto new_set = item_set();

        for (auto item_i : temp_set)  // ����Ŀ����ÿһ����Ŀ
        {
            // ��ź����Ƿ��ս������������ŵ���չ����ʽ
            auto item = std::get<1>(item_i);

            for (int i = 0, size = item.size(); i < size; i++)
            {
                if (i == size - 1 || item[i + 1] == m_end_of_file)  // ��Ų��������һ�� token������һ�������� $
                    break;
                if (item[i] != m_dot)
                    continue;
                if (m_nonterminal.find(item[i + 1]) == m_nonterminal.end())  // ��ź��Ƿ��ս��
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
        �� terminal
    */
    for (auto i : m_nt)
    {
        if (m_nonterminal.find(i) == m_nonterminal.end())
            m_terminal.insert(i);
    }

    /*
        �����в���ʽ�� vector �г����������ڹ�Լ״̬�µı��
    */
    int production_index = 0;
    auto acc = std::make_tuple(m_start_token + L"'", std::vector<token>{ m_start_token, m_dot, m_end_of_file });
    m_productions.push_back(std::make_tuple(m_start_token + L"'", std::vector<token>{ m_start_token, m_end_of_file }));  // ����� Accept ״̬
    m_production_index[acc] = production_index++;
    
    // ��������������к�д

    /*
        ��ʼ��Ŀ S'->.S$
    */
    m_start_item = std::make_tuple(m_start_token + L"'", 
        std::vector<token>{ m_dot, m_start_token, m_end_of_file });

    /*
        ���ս����Ӧ����ʽ��������Ŀ��ʼ����
        �� S->AB|aC|dB���� m_items_start[S] = { [.AB], [.aC], [.dB] }
           A->ab|dF���� m_items_start[A] = { [.ab], [.dF] }
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

            if (!D.size())  // D Ϊ����˵�� C ����ͨ�� x ת����������
            {
                if (m_nonterminal.find(x) == m_nonterminal.end())
                    m_action_table[std::make_tuple(m_state_map[C], x)] = -1;  // ���Ϊ������ʾ����ת��
                else
                    m_goto_table[std::make_tuple(m_state_map[C], x)] = -1;

                continue;
            }

            if (m_state_map.find(D) == m_state_map.end())
            {
                m_state_set.push_back(D);
                m_state_map[D] = count++;
                Q.push(D);
            }

            flag_reduce = false;  // �ܹ�����ת�ƣ�˵���Ƿǹ�Լ̬

            if (m_nonterminal.find(x) == m_nonterminal.end())
                m_action_table[std::make_tuple(m_state_map[C], x)] = m_state_map[D];
            else
                m_goto_table[std::make_tuple(m_state_map[C], x)] = m_state_map[D];
        }

        if (flag_reduce)  // �� Accept state Ҳ����������
            m_reduce_state.insert(m_state_map[C]);
    }
}

void M6::LR0Parsing::ReData()
{
    for (auto state : m_state_set)
    {
        int state_index = m_state_map[state];  // ��ȡ״̬����ֵ�������ѭ���ǰ���С�����˳���

        // terminal -- Action ��
        for (auto t : m_terminal)
        {
            std::wstring cell_data = L"";  // ��Ԫ������

            // ������ǿɹ�Լ״̬
            if (m_reduce_state.find(state_index) == m_reduce_state.end())
            {
                int next_index = m_action_table[std::make_tuple(state_index, t)];  // ��ȡ���ƽ�������һ��״̬���
                cell_data += (next_index < 0 ? L"" : L"s" + std::to_wstring(next_index));
            }
            else  // �ǿɹ�Լ״̬
            {
                int grammar_index = m_production_index[*state.begin()];
                cell_data += L"r" + std::to_wstring(grammar_index);
            }

            m_parsing_table[std::make_tuple(state_index, t)] = cell_data;
        }

        // $ �ж�
        if (m_reduce_state.find(state_index) != m_reduce_state.end())  // �ǹ�Լ״̬
        {
            int grammar_index = m_production_index[*state.begin()];

            if (grammar_index)  // ���� 0��˵������ Accept State
                m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = L"r" + std::to_wstring(grammar_index);
            else
                m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = L"acc";
        }
        else
            m_parsing_table[std::make_tuple(state_index, m_end_of_file)] = L"";

        // nonterminal -- Goto ��
        for (auto t : m_nonterminal)
        {
            std::wstring cell_data = L"";  // ��Ԫ������

            // ������ǿɹ�Լ״̬
            if (m_reduce_state.find(state_index) == m_reduce_state.end())
            {
                int next_index = m_goto_table[std::make_tuple(state_index, t)];  // ��ȡ���ƽ�������һ��״̬���
                cell_data += (next_index < 0 ? L"" : L"g" + std::to_wstring(next_index));
            }

            m_parsing_table[std::make_tuple(state_index, t)] = cell_data;
        }
    }
}