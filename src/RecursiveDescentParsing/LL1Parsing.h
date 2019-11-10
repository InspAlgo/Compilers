#pragma once

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <map>

namespace M6 
{
    using token = std::wstring;

    class LL1Parsing
    {
    public:
        LL1Parsing();
        void SetStartToken(token start_token);
        void AddProduction(const token &production_left, const std::vector<token> &production_right);  // 添加一条产生式
        void RunParsing();
        void GetPredictiveParsingTable(std::map<std::tuple<token, token>,
            std::set<std::tuple<token, std::vector<token>>>> &predictive_parsing_table);  // 获取预测分析表
        void Clear();  // 清空类中所有成员容器的内部元素

    private:
        void CheckGrammar();  // 检查并修正文法，使之满足 LL(1) 文法规范
        void ExtractLeftFactoring();  // 提取左因子
        void RemovingDirectLeftRecursion();  // 消除直接左递归
        void RemovingIndirectLeftRecursion();  // 消除间接左递归
        void SimplifyGrammar();  // 简化文法，消除不需要的产生式
        void TokenClosure(token node, std::set<token> &token_closure);

        void InitSet();  // 将所有的 nullable 初始化为 false，将所有的 FIRST 和 FOLLOW 初始化为空集合
        void CalcNullable();  // 不动点算法求 nullable
        void CalcFirstSet();  // 不动点算法求 FIRST
        void CalcFollowSet();  // 不动点算法求 FOLLOW
        void CalcFirstSSet();  // 求 FIRST_S
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
        std::set<token> m_nonterminal;  // 非终结符集合
        std::vector<std::tuple<token, std::vector<token>>> m_productions;  // 所有产生式左部非终结符，vector 里存放该非终结符对应的所有右部
        std::map<token, std::set<std::vector<token>>> m_productions_map;
        std::map<token, bool> m_nullable;  // 非终结符是否可空
        std::map<token, std::set<token>> m_first_set;  // FIRST 集合
        std::map<token, std::set<token>> m_follow_set;  // FOLLOW 集合
        std::map<std::tuple<token, std::vector<token>>,
            std::set<token>> m_first_s_set;  // FIRST_S 集合，即将一个 FIRST 的计算推广到一个串或一个句子上面
        std::map<std::tuple<token, token>,
            std::set<std::tuple<token, std::vector<token>>>> m_predictive_parsing_table;  // 预测分析表
    };
}

