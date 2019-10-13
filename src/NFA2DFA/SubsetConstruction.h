#pragma once
#include <string>
#include <set>
#include <list>
#include <map>

/*
    子集构造算法
*/
class SubsetConstruction
{
public:
    SubsetConstruction();
    void InputNFA(std::set<char> character, std::string start_node, std::set<std::string> end_nodes, std::map<std::pair<std::string, char>, std::list<std::string>> NFA);
    void GetDFA();
    void OutputDFA(std::string new_start_node, std::set<std::string> new_end_nodes, std::map<std::pair<std::string, char>, std::string> new_DFA);

private:
    void EpsilonClosure(std::string node);  // ε-闭包计算接口
    std::set<std::string> EClosure(std::set<std::string> delta);
    bool Visited(std::string node);  // 判断节点是否访问过，是返回 true
    void EpsilonClosureDFS(std::string node);  // ε-闭包计算，使用深度优先遍历算法实现
    std::set<std::string> Delta(std::set<std::string> q, char c);
    void WorkList(std::string start_node);  // 子集构造计算，使用工作表算法实现
    std::string IDGenerator();  // DFA 状态编号生成器

    std::set<std::string> m_epsilon_closure;  // 每轮闭包计算的 closure 集合
    std::list<std::set<std::string>> m_worklist;
    std::map<std::pair<std::string, char>, std::list<std::string>> m_NFA;  // 存储 NFA 图结构
    std::map<std::pair<std::set<std::string>, char>, std::set<std::string>> m_DFA;  // 存储 DFA 图结构
    std::map<std::string, bool> m_visited;  // 在进行闭包计算时判断是否访问过此节点
    std::set<char> m_character;  // 状态转换参数
    std::string m_start_node;  // 状态起始节点
    std::set<std::string> m_end_nodes;  // 状态终止节点集合
    int m_state_id;

    std::string m_new_start_node;
    std::set<std::string> m_new_end_nodes;
    std::map<std::pair<std::string, char>, std::string> m_new_DFA;
};