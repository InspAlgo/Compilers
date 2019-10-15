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
    void InputNFA(std::set<wchar_t> character, std::wstring start_node, std::set<std::wstring> end_nodes, std::map<std::pair<std::wstring, wchar_t>, std::list<std::wstring>> NFA);
    void GetDFA();  // 生成 DFA
    void OutputDFA(std::wstring& new_start_node, std::set<std::wstring>& new_end_nodes, std::map<std::pair<std::wstring, wchar_t>, std::wstring>& new_DFA);

private:
    void EpsilonClosure(std::wstring node); // ε-闭包计算接口
    std::set<std::wstring> EClosure(std::set<std::wstring> delta);  // 集合 delta 内各节点经过ε-闭包计算后得到的节点集合
    bool Visited(std::wstring node);  // 判断节点是否访问过，是返回 true
    void EpsilonClosureDFS(std::wstring node);  // ε-闭包计算，使用深度优先遍历算法实现
    std::set<std::wstring> Delta(std::set<std::wstring> q, wchar_t c);  // 集合 q 内各节点经过 c 转换得到的节点集合
    void WorkList(std::wstring start_node); // 子集构造计算，使用工作表算法实现
    std::wstring IDGenerator();  // DFA 状态编号生成器

    std::set<std::wstring> m_epsilon_closure;  // 每轮闭包计算的 closure 集合
    std::list<std::set<std::wstring>> m_worklist;  // 工作表，存放每次大闭包计算 EClosure 的结果
    std::map<std::pair<std::wstring, wchar_t>, std::list<std::wstring>> m_NFA;  // 存储 NFA 图结构
    std::map<std::pair<std::set<std::wstring>, wchar_t>, std::set<std::wstring>> m_DFA; // 存储 DFA 图结构
    std::map<std::wstring, bool> m_visited;  // 在进行闭包计算时判断是否访问过此节点
    std::set<wchar_t> m_character;  // 状态转换参数
    std::wstring m_start_node;  // 状态起始节点
    std::set<std::wstring> m_end_nodes;  // 状态终止节点集合
    int m_state_id;  // 生成的 DFA 状态 id 计数

    std::map<std::set<std::wstring>, std::wstring> m_state_name; // DFA 各节点与编号名称映射

    std::wstring m_new_start_node;  // 生成的 DFA 的状态起始节点名称
    std::set<std::wstring> m_new_end_nodes;  // 生成的 DFA 的状态终止节点
    std::map<std::pair<std::wstring, wchar_t>, std::wstring> m_new_DFA;  // 生成的 DFA 状态转移表
};