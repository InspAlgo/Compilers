#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>

/*
    DFA 最小化: Hopcroft 算法
*/
class Hopcroft
{
public:
    Hopcroft();
    void InputDFA();
    void GetMiniDFA();
    void OutputMiniDFA();

private:
    std::string m_start_node;  // 输入的 DFA 的状态起始节点名称
    std::unordered_set<std::string> m_end_nodes;  // 输入的 DFA 的状态终止节点
    std::unordered_map<std::pair<std::string, char>, std::string> m_DFA;  // 输入的 DFA 状态转移表

    std::string m_new_start_node;  // 生成的最小化 DFA 的状态起始节点名称
    std::unordered_set<std::string> m_new_end_nodes;  // 生成的最小化 DFA 的状态终止节点
    std::unordered_map<std::pair<std::string, char>, std::string> m_new_DFA;  // 生成的最小化 DFA 状态转移表
};