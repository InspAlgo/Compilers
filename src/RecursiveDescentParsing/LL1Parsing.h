#pragma once

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <map>

namespace M6
{
    class LL1Parsing
    {
    public:
        LL1Parsing();
        void SetStartToken(std::wstring start_token);
        void AddProduction(const std::wstring &production_left, const std::vector<std::wstring> &production_right);  // 添加一条产生式
        void RunParsing();
        void GetFirstSet(std::map<std::wstring, std::set<std::wstring>> &first_set);  // 获取 FIRST 集
        void GetFollowSet(std::map<std::wstring, std::set<std::wstring>> &follow_set);  // 获取 FOLLOW 集
        void GetPredictiveParsingTable(std::map<std::tuple<std::wstring, std::wstring>,
            std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &predictive_parsing_table);  // 获取预测分析表
        void GetTerminalSet(std::set<std::wstring> &terminal);  // 获取终结符集合
        void GetNonterminalSet(std::set<std::wstring> &nonterminal);  // 获取非终结符集合
        void Clear();  // 清空类中所有成员容器的内部元素

    private:
        using token = std::wstring;

    private:
        void CheckGrammar();  // 检查并修正文法，使之满足 LL(1) 文法规范
        void ExtractLeftFactoring();  // 提取左因子
        void RemovingDirectLeftRecursion();  // 消除直接左递归
        void RemovingIndirectLeftRecursion();  // 消除间接左递归
        void SimplifyGrammar();  // 简化文法，消除不需要的产生式
        void TokenClosure(token node, std::set<token> &token_closure);
        bool IndirectLeftRecursionTokenClosure(token node, std::vector<token> &token_closure);

        void InitSet();  // 将所有的 nullable 初始化为 false，将所有的 FIRST 和 FOLLOW 初始化为空集合
        void CalcNullable();  // 不动点算法求 nullable
        void CalcFirstSet();  // 不动点算法求 FIRST
        void CalcFollowSet();  // 不动点算法求 FOLLOW
        void CalcSelectSet();  // 求 FIRST_S
        void CalcPredictiveParsingTable();  // 求预测分析表

        // 用于构建产生式的语法树，通过公共结点提取左因子，构造字典树的思路
        class TokenNode
        {
        public:
            TokenNode(token data);
            static void SetStaticData(token temp_token, int idx);
            void AddDifferentNodes(std::vector<token> &production, int idx);
            void CreateNewProductions(token production_left, int idx,
                std::map<std::tuple<token, int>, std::vector<token>> &productions_map);
            token GetNewToken();
            void DestroyChildNodes();  // 删除所有子结点

        private:
            static token m_temp_token;
            static int m_production_idx;
            token m_token;
            TokenNode* m_left_ptr;  // 孩子结点
            TokenNode* m_right_ptr;  // 兄弟结点，也是其父节点的孩子结点
        };

        token m_start_token;
        token m_end_of_file;
        std::set<token> m_nonterminal;  // 非终结符集合
        std::set<token> m_terminal;  // 终结符集合
        std::vector<std::tuple<token, std::vector<token>>> m_productions;  // 所有产生式左部非终结符，vector 里存放该非终结符对应的所有右部
        std::map<token, std::set<std::vector<token>>> m_productions_map;
        std::map<token, bool> m_nullable;  // 非终结符是否可空
        std::map<token, std::set<token>> m_first_set;  // FIRST 集 (非推广形式)
        std::map<token, std::set<token>> m_follow_set;  // FOLLOW 集
        std::map<std::tuple<token, std::vector<token>>,
            std::set<token>> m_select_set;  // FIRST_S 集合
        std::map<std::tuple<token, token>,
            std::set<std::tuple<token, std::vector<token>>>> m_predictive_parsing_table;  // 预测分析表
        std::map<std::tuple<token, token>,
            std::set<std::tuple<token, std::vector<token>>>> m_predictive_parsing_table_sharp;  // 带句子的右界符 # 的预测分析表
    };
}

