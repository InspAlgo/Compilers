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
        ~LRParsing();

        // 清空所有容器类型成员，起始符号全部置空，文法标记全部置 false
        void Clear();

        // 设置文法中的起始符号和拓广文法中的新起始符号
        void SetStartToken(const std::wstring& start_token, const std::wstring& new_start_token);

        // 设置一个项目里的点号
        // Clear 后不会恢复成默认值
        void SetDot(const std::wstring& dot);

        // 设置文件结束符，即左界符或右界符
        // Clear 后不会恢复成默认值
        void SetEndOfFile(const std::wstring& end_of_file);

        // 添加产生式
        void AddProduction(const std::wstring& production_left, const std::vector<std::wstring>& production_right);

        // 构建 LR 分析表
        void BuildLRParsingTable();

        // 获取文法类型
        // 返回 0-非LR文法 1-LR(0)文法 2-SLR(1)文法 3-LALR(1)文法 4-LR(1)文法
        std::wstring GetGrammarType();

        // 获取文法分析表
        void GetParsingTable(std::map<std::tuple<std::wstring, std::wstring>, std::wstring>& parsing_table);

    public:
        // 分析输入符号串，末尾不带右界符，由本函数自动添加，返回分析过程
        void ParsingTokens(const std::vector<std::wstring>& input_tokens, std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>>& parsing_process);

    private:
        using Token = std::wstring;  // 符号类型
        using ItemLR0 = std::tuple<Token, std::vector<Token>>;  // LR(0)/SLR(1) 文法的项目类型
        using ItemLR1 = std::tuple<Token, std::vector<Token>, std::set<Token>>;  // LR(1)/LALR(1) 文法的项目类型

    private:
        // 预处理部分
        // 包含生成 terminal 集合、拓广文法、生成规约项目表、创建 NULLABLE/FIRST/FOLLOW 集
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
        void CreateFirstSet();

        // 生成文法的 FOLLOW 集
        void CreateFollowSet();

        // LR(0)/SLR(1) 文法的闭包函数
        void Closure(std::set<ItemLR0>& items_set);

        // LR(1)/LALR(1) 文法的闭包函数
        void Closure(std::set<ItemLR1>& items_set);

        // LR(0)/SLR(1) 文法的状态转移函数
        std::set<ItemLR0> Go(const std::set<ItemLR0>& items_set, const Token& x);

        // LR(1)/LALR(1) 文法的状态转移函数
        std::set<ItemLR1> Go(const std::set<ItemLR1>& items_set, const Token& x);

        // 获取 LR(1) 项目集类型 set{[A->α·β,a],...,[B->r·,b]} 中规约项目 B->r· 的向前查看符 b
        std::set<Token> GetLookAheadTokens(const std::set<ItemLR1>& items_set, const ItemLR0& item);

        // 构造识别 LR(0)/SLR(1) 文法规范句型活前缀 DFA
        // 即构造 LR(0)/SLR(1) 文法的项目集规范族
        void BuildItemsSets();

        // 构建 LR(0) ACTION/GOTO 表
        void BuildLR0ParsingTable();

        // 构建 SLR(1) ACTION/GOTO 表
        void BuildSLR1ParsingTable();

        // 构造 LR(1) 文法的项目集族
        void BuildItem2sSets();

        // 创建 LR(1) 文法项目集中对应的 LR(0) 项目集
        void CreateItemsSet1Map2();

        // 构建 LR(1) ACTION/GOTO 表
        void BuildLR1ParsingTable();

        // 合并同心集的向前查看符
        void MergeLookAheadTokens(const std::set<size_t>& same_cores_set, const size_t& index);

        // 构建 LALR(1) 项目集
        // 调用 MergeLookAheadTokens() 方法
        void BuildItem3sSets();

        // 更新 LALR(1) 的 DFA
        // 因为之前存的 DFA 是 LR(1) 的
        void UpdateDFAofLALR1();

        // 构建 LALR(1) ACTION/GOTO 表
        void BuildLALR1ParsingTable();

        // 创建 LR(0)/SLR(1) 分析表
        void CreateParsingTable(bool& grammar_flag);

        // 创建 LR(1)/LALR(1) 分析表
        void CreateParsingTable(bool& grammar_flag, const std::vector<std::set<ItemLR1>>& items_sets, const std::map<std::set<ItemLR1>, size_t>& items_sets_3_map);

        // 拷贝 DFA
        void CopyDFA(std::map<std::tuple<size_t, Token>, size_t>& DFA);

        // 拷贝分析表到具体类型的分析表中
        void CopyParsingTable(std::map<std::tuple<std::wstring, Token>, std::wstring>& from_parsing_table, std::map<std::tuple<std::wstring, Token>, std::wstring>& to_parsing_table);

    private:
        // 总控程序，也称驱动程序，对于所有的 LR 分析器其总控程序是相同的
        void Control();

        // 执行当前步骤
        bool RunCurStep(int step_count);

        // 将分析栈中的数据转成字符串
        void ParsingStackToString(int index, std::wstring parsing_action);

    private:
        bool m_LR0;  // 标记是否为 LR(0) 文法
        bool m_SLR1;  // 标记是否为 SLR(1) 文法
        bool m_LALR1;  // 标记是否为 LALR(1) 文法
        bool m_LR1;  // 标记是否为 LR(1) 文法

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

        std::vector<std::tuple<Token, std::vector<Token>>> m_expanding_grammar;  // 拓广文法
        std::map<Token, std::set<ItemLR0>> m_original_items;  // 初始项目集 即 A->·aB B->·Da 这种 dot 符在首位的项目
        std::map<ItemLR0, size_t> m_reduction_items;  // 所有的规约项目  <规约项目, 对应的拓广文法中的序号>
        
        std::map<Token, bool> m_nullable;  // 拓广文法的非终结符是否可推导到 epsilon
        std::map<Token, std::set<Token>> m_first_set;  // 拓广文法的 FIRST 集(非推广形式)
        std::map<Token, std::set<Token>> m_follow_set;  // 拓广文法的 FOLLOW 集

        std::vector<std::set<ItemLR0>> m_items_sets_LR0orSLR1;  // LR(0)/SLR(1) 项目集规范族，因为 LR(0) 和 SLR(1) 的项目集族相同
        std::map<std::set<ItemLR0>, size_t> m_items_sets_LR0orSLR1_map;  // 方便获取 m_items_sets_LR0orSLR1 中项目集位置

        std::vector<std::set<ItemLR1>> m_items_sets_LR1;  // LR(1) 项目集族
        std::map<std::set<ItemLR1>, size_t> m_items_sets_LR1_map;  // 方便获取 m_items_sets_2 中项目集位置
        std::map<size_t, std::set<ItemLR0>> m_items_sets_LR1_map_LR0;  // LR(1) 项目集中对应的 LR(0) 项目集

        std::vector<std::set<ItemLR1>> m_items_sets_LALR1;  // LALR(1) 合并同心集后的项目集族
        std::map<std::set<ItemLR1>, size_t> m_items_sets_LALR1_map;  // 方便获取 m_items_sets_LALR1 中项目集位置
        std::map<size_t, std::set<ItemLR0>> m_items_sets_LALR1_map_LR0;  // LALR(1) 项目集中对应的 LR(0) 项目集
        std::map<size_t, std::set<size_t>> m_items_sets_LALR1_map_LR1;  // 组成 LALR(1) 项目集对应的 LR(1) 项目集编号
        std::map<size_t, size_t> m_items_sets_LR1_map_LALR1;  // LR(1) 项目集对应的 LALR(1) 项目集

        std::map<std::tuple<size_t, Token>, size_t> m_DFA;  // 当前文法项目集的 DFA
        std::map<std::tuple<size_t, Token>, size_t> m_DFA_LR0orSLR1;  // LR(0) 项目集的 DFA
        std::map<std::tuple<size_t, Token>, size_t> m_DFA_LALR1;  // LR(1) 项目集的 DFA
        std::map<std::tuple<size_t, Token>, size_t> m_DFA_LR1;  // LALR(1) 项目集的 DFA

        std::map<std::tuple<size_t, Token>, std::set<std::wstring>> m_action_table;  //  当前文法 ACTION 表
        std::map<std::tuple<size_t, Token>, std::wstring> m_goto_table;  //  当前文法 GOTO 表

        std::map<std::tuple<std::wstring, Token>, std::wstring> m_parsing_table;  //  当前文法 LR 分析表
        std::map<std::tuple<std::wstring, Token>, std::wstring> m_parsing_table_LR0;  // LR(0) 分析表
        std::map<std::tuple<std::wstring, Token>, std::wstring> m_parsing_table_SLR1;  // SLR(1) 分析表
        std::map<std::tuple<std::wstring, Token>, std::wstring> m_parsing_table_LALR1;  // LALR(1) 分析表
        std::map<std::tuple<std::wstring, Token>, std::wstring> m_parsing_table_LR1;  // LR(1) 分析表

    private:
        std::vector<Token> m_input_tokens;  // 输入符号串
        std::vector<std::wstring> m_state_stack;  // 状态栈
        std::vector<Token> m_tokens_stack;  // 符号栈
        std::vector<std::wstring> m_parsing_action_stack;  // 分析动作栈
        std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring> m_cur_parsing_data;  // 当前分析数据
        std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>> m_parsing_process;  // 分析过程
    };
}