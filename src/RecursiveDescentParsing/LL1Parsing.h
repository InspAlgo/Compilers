#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <map>

namespace M6 {
    using token = std::wstring;

    class LL1Parsing
    {
    public:
        LL1Parsing();
        void AddProduction(const token &production_left, const std::vector<token> &production_right);  // 添加一条产生式
        void RunParsing();
        void GetPredictiveParsingTable(std::map<std::tuple<token, token>,
            std::set<std::tuple<token, std::vector<token>>>> &predictive_parsing_table);  // 获取预测分析表
        void Clear();  // 清空类中所有成员容器的内部元素

    private:
        void InitSet();  // 将所有的 nullable 初始化为 false，将所有的 FIRST 和 FOLLOW 初始化为空集合
        void CalcNullable();  // 不动点算法求 nullable
        void CalcFirstSet();  // 不动点算法求 FIRST
        void CalcFollowSet();  // 不动点算法求 FOLLOW
        void CalcFirstSSet();  // 求 FIRST_S
        void CalcPredictiveParsingTable();  // 求预测分析表

        std::set<token> m_nonterminal;  // 非终结符集合
        std::vector<std::tuple<token, std::vector<token>>> m_productions;  // 所有产生式左部非终结符，vector 里存放该非终结符对应的所有右部
        std::map<token, bool> m_nullable;  // 非终结符是否可空
        std::map<token, std::set<token>> m_first_set;  // FIRST 集合
        std::map<token, std::set<token>> m_follow_set;  // FOLLOW 集合
        std::map<std::tuple<token, std::vector<token>>,
            std::set<token>> m_first_s_set;  // FIRST_S 集合，即将一个 FIRST 的计算推广到一个串或一个句子上面
        std::map<std::tuple<token, token>,
            std::set<std::tuple<token, std::vector<token>>>> m_predictive_parsing_table;  // 预测分析表
    };
}

