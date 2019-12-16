#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <tuple>

/*
    LR(0) SLR(1) LR(1) LALR(1) 四种 LR 分析算法
*/

namespace M6
{
    class LRParsing
    {
    public:
        LRParsing();

        // 清空文法及分析表
        void Clear();

        // 设置文法中的起始符号和拓广文法中的新起始符号
        void SetStartToken(const std::wstring &start_token, const std::wstring &new_start_token);

        // 设置一个项目里的点号
        // Clear 后不会恢复成默认值
        void SetDot(const std::wstring &dot);

        // 设置文件结束符，即左界符或右界符
        // Clear 后不会恢复成默认值
        void SetEndOfFile(const std::wstring &end_of_file);

        // 添加产生式
        void AddProduction(const std::wstring &production_left, const std::vector<std::wstring> &production_right);

        // 构建 LR 分析表
        void BuildLRParsingTable();

        // 获取文法类型
        // 返回 0-非LR文法 1-LR(0)文法 2-SLR(1)文法 3-LALR(1)文法 4-LR(1)文法
        int GetGrammarType();

    private:
        using Token = std::wstring;  // 符号类型
        using Production = std::tuple<Token, std::vector<Token>>;  // 产生式类型
        using Item = std::tuple<Token, std::vector<Token>>;  // LR(0)/SLR(1) 项目类型
        using Item2 = std::tuple<Token, std::vector<Token>, std::set<Token>>;  // LR(1)/LALR(1) 项目类型

    private:
        // 预处理部分
        // 包含生成 terminal 集合、拓广文法、生成规约项目表
        void Preprocess();

        // 生成 terminal 集合
        void CreateTerminalSet();

        // 拓广文法
        void ExpandGrammar();

        // 生成拓广文法对应的初始项目集
        void CreateOriginalItems();

        // 生成拓广文法的对应的规约项目表
        // 依赖拓广文法，故需之前有调用 ExpandGrammar() 方法
        void CreateReductionItemTable();

        // 生成拓广文法的非终结符判空表
        void CreateNullable();

        // 生成文法的 FIRST 集(非推广形式)
        // 推广形式使用的是 SELECT 集表示
        void CreateFirstSet();

        // 生成文法的 FOLLOW 集
        void CreateFollowSet();

        // LR(0)/SLR(1) 文法的闭包函数
        void Closure(std::set<Item> &items_set);

        // LR(1)/LALR(1) 文法的闭包函数
        void Closure(std::set<Item2> &items_set);

        // LR(0)/SLR(1) 文法的状态转移函数
        std::set<Item> Go(const std::set<Item> &items_set, const Token &x);

        // LR(1)/LALR(1) 文法的状态转移函数
        std::set<Item2> Go(const std::set<Item2> &items_set, const Token &x);

        // 构造识别 LR(0)/SLR(1) 文法规范句型活前缀 DFA
        // 即构造 LR(0)/SLR(1) 文法的项目集规范族
        void BuildItemsSets();

        // 构建 LR(0) ACTION/GOTO 表
        void BuildLR0ParsingTable();

        // 构建 SLR(1) ACTION/GOTO 表
        void BuildSLR1ParsingTable();

        // 构建 LR(1) ACTION/GOTO 表
        void BuildLR1ParsingTable();

        // 构建 LALR(1) ACTION/GOTO 表
        void BuildLALR1ParsingTable();

        // 创建 LR(0)/SLR(1) 分析表
        void CreateParsingTable1(bool &grammar_flag);

    private:
        Token m_start_token;  // 文法中的起始符号
        Token m_new_start_token;  // 拓广文法中的新起始符号
        Token m_dot;  // 一个项目里的点号
        Token m_end_of_file;  // 文件结束符，即左界符或右界符
        std::map<Token, std::set<std::vector<Token>>> m_original_grammar;  // 输入的初始文法

        std::set<Token> m_nonterminals;  // 初始文法的非终结符集合
        std::set<Token> m_terminals;  // 初始文法的终结符集合
        std::set<Token> m_alltokens;  // 初始文法的所有非终结符和终结符集合
        std::set<Token> m_expanding_nonterminals;  // 拓广文法的非终结符集合
        std::set<Token> m_expanding_terminals;  // 拓广文法的终结符集合
        std::set<Token> m_expanding_alltokens;  // 拓广文法的所有非终结符和终结符集合

        std::vector<Production> m_expanding_grammar;  // 拓广文法
        std::map<Token, std::set<Item>> m_original_items;  // 初始项目集 即 A->·aB B->·Da 这种 dot 符在首位的项目
        std::map<Item, size_t> m_reduction_items;  // 所有的规约项目  <规约项目, 对应的拓广文法中的序号>
        
        std::map<Token, bool> m_nullable;  // 拓广文法的非终结符是否可推导到 epsilon
        std::map<Token, std::set<Token>> m_first_set;  // 拓广文法的 FIRST 集(非推广形式,推广形式使用的是 SELECT 集)
        std::map<Token, std::set<Token>> m_follow_set;  // 拓广文法的 FOLLOW 集

        bool m_LR0;  // 标记是否为 LR(0) 文法
        bool m_SLR1;  // 标记是否为 SLR(1) 文法
        bool m_LALR1;  // 标记是否为 LALR(1) 文法
        bool m_LR1;  // 标记是否为 LR(1) 文法

        std::vector<std::set<Item>> m_items_sets_1;  // LR(0) 和 SLR(1) 项目集规范族
        std::map< std::set<Item>, size_t> m_items_sets_1_map;  // 方便获取 m_items_sets_1 中项目集位置
        std::map<std::tuple<size_t, Token>, size_t> m_LR0_DFA;  // 识别 LR(0) 和 SLR(1) 文法活前缀的 DFA

        std::vector<std::set<Item2>> m_items_set_2;  // LR(1) 项目集族


        std::map<std::tuple<size_t, Token>, std::set<std::string>> m_LR0_action_table;  // LR(0)/SLR(1) ACTION 表
        std::map<std::tuple<size_t, Token>, std::string> m_LR0_goto_table;  // LR(0)/SLR(1) GOTO 表
        std::map<std::tuple<std::set<size_t>, Token>, std::set<size_t>> m_action_table;  // ACTION 表
        std::map<std::tuple<std::set<size_t>, Token>, std::set<size_t>> m_goto_table;  // GOTO 表

        std::map<std::tuple<std::string, Token>, std::string> m_parsing_table;  // LR 分析表
    };
}