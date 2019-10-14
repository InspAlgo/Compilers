#include "SubsetConstruction.h"

SubsetConstruction::SubsetConstruction()
{
    m_NFA = std::map<std::pair<std::string, char>, std::list<std::string>>();
    m_visited = std::map<std::string, bool>();
    m_worklist = std::list<std::set<std::string>>();
    m_start_node = "";
    m_character = std::set<char>();
    m_end_nodes = std::set<std::string>();
    m_state_id = 0;
    m_state_name = std::map<std::set<std::string>, std::string>();
    m_new_start_node = "";
    m_new_end_nodes = std::set<std::string>();
    m_new_DFA = std::map<std::pair<std::string, char>, std::string>();
}

void SubsetConstruction::InputNFA(std::set<char> character, std::string start_node,
    std::set<std::string> end_nodes, std::map<std::pair<std::string, char>, std::list<std::string>> NFA)
{
    m_character = character;
    m_start_node = start_node;
    m_end_nodes = end_nodes;
    m_NFA = NFA;
    m_state_id = 0;
    m_state_name.clear();
    m_new_start_node = "";
    m_new_end_nodes.clear();
    m_new_DFA.clear();
}

std::string SubsetConstruction::IDGenerator()
{
    std::string id = "q" + std::to_string(m_state_id);
    m_state_id++;
    return id;
}

void SubsetConstruction::GetDFA()
{
    WorkList(m_start_node);

    for (auto node : m_DFA)
    {
        std::set<std::string> from_nodes = node.first.first;
        std::set<std::string> to_nodes = node.second;

        if (m_state_name.find(from_nodes) == m_state_name.end())
            m_state_name[from_nodes] = IDGenerator();
        if (m_state_name.find(to_nodes) == m_state_name.end())
            m_state_name[to_nodes] = IDGenerator();

        std::string from_node = m_state_name[from_nodes];
        std::string to_node = m_state_name[to_nodes];

        m_new_DFA[std::make_pair(from_node, node.first.second)] = to_node;

        for (auto end_node : m_end_nodes)
        {
            if (from_nodes.find(end_node) != from_nodes.end())
                m_new_end_nodes.insert(from_node);
            if (to_nodes.find(end_node) != to_nodes.end())
                m_new_end_nodes.insert(to_node);
        }
    }
}

void SubsetConstruction::OutputDFA(std::string& new_start_node, std::set<std::string>& new_end_nodes, 
    std::map<std::pair<std::string, char>, std::string>& new_DFA)
{
    new_start_node = m_new_start_node;
    new_end_nodes = m_new_end_nodes;
    new_DFA = m_new_DFA;
}

void SubsetConstruction::EpsilonClosure(std::string node)
{
    m_visited.clear();
    m_epsilon_closure = std::set<std::string>();
    EpsilonClosureDFS(node);
}

bool SubsetConstruction::Visited(std::string node)
{
    if (m_visited.find(node) == m_visited.end())
        return false;
    return true;
}

void SubsetConstruction::EpsilonClosureDFS(std::string node)
{
    // 把此节点并到集合中，任何一个元素的闭包都包含自身
    m_epsilon_closure.insert(node);
    m_visited[node] = true;

    // 取出转移参数为 ε 的节点列表，用 '\0' 表示 ε
    std::list<std::string> to_node_list = m_NFA[std::make_pair(node, '\0')];

    // next_node 是 node 的后继节点，且通过 ε 转换
    for (auto next_node : to_node_list)
    {
        if (!Visited(next_node))
            EpsilonClosureDFS(next_node);
    }
}

std::set<std::string> SubsetConstruction::Delta(std::set<std::string> q, char c)
{
    std::set<std::string> delta = std::set<std::string>();

    for (auto q_node : q)
    {
        if (m_NFA.find(std::make_pair(q_node, c)) == m_NFA.end())
            continue;

        for (auto node : m_NFA[std::make_pair(q_node, c)])
            delta.insert(node);
    }

    return delta;
}

std::set<std::string> SubsetConstruction::EClosure(std::set<std::string> delta)
{
    std::set<std::string> e_closure = std::set<std::string>();

    for (auto node : delta)
    {
        EpsilonClosure(node);
        e_closure.insert(m_epsilon_closure.begin(), m_epsilon_closure.end());
    }

    return e_closure;
}

void SubsetConstruction::WorkList(std::string start_node)
{
    EpsilonClosure(start_node);
    std::set<std::string> q0 = m_epsilon_closure;  // DFA 的起始节点

    m_state_name[q0] = IDGenerator();
    m_new_start_node = m_state_name[q0];

    std::set<std::set<std::string>> Q = std::set<std::set<std::string>>();
    Q.insert(q0);

    m_worklist.push_back(q0);

    while (!m_worklist.empty())
    {
        std::set<std::string> q = m_worklist.front();
        m_worklist.pop_front();

        for (auto c : m_character)
        {
            std::set<std::string> t = EClosure(Delta(q, c));
            if (t.size() == 0)
                continue;

            // 并上 t 集合
            m_DFA[std::make_pair(q, c)].insert(t.begin(), t.end());

            if (Q.find(t) == Q.end())
            {
                Q.insert(t);
                m_worklist.push_back(t);
            }
        }
    }
}