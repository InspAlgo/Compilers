#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <map>

class LL1Parsing
{
public:
    LL1Parsing();
    void AddProduction(wchar_t production_left, std::wstring production_right);  // 添加一条产生式
    void GetPredictiveParsingTable();
private:
    void InitSet();  // 将所有的 nullable 初始化为 false，将所有的 FIRST 和 FOLLOW 初始化为空集合
    void GetNullable();  // 不动点算法求 nullable
    void GetFirstSet();  // 不动点算法求 FIRST
    void GetFollowSet();  // 不动点算法求 FOLLOW

    std::set<wchar_t> m_terminal;  // 终结符集合
    std::set<wchar_t> m_nonterminal;  // 非终结符集合
    std::vector<std::tuple<wchar_t, std::wstring>> m_productions;  // 所有产生式 wchar_t 表示左部非终结符，vector 里存放该非终结符对应的所有右部
    std::map<wchar_t, bool> m_nullable;  // 非终结符是否可空
    std::map<wchar_t, std::set<wchar_t>> m_first_set;  // FIRST 集合
    std::map<wchar_t, std::set<wchar_t>> m_follow_set;  // FOLLOW 集合
    std::map<std::pair<wchar_t, wchar_t>, std::set<std::wstring>> m_predictive_parsing_table;  // 预测分析表
};