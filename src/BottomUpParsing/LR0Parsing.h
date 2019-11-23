#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <tuple>

namespace M6
{
    class LR0Parsing
    {
        using token = std::wstring;
        using item_set = std::set<std::tuple<token, std::vector<token>>>;

    public:
        LR0Parsing();
        void SetStartToken(std::wstring start_token);
        void SetDot(std::wstring dot);
        void SetEndOfFile(std::wstring end_of_file);
        void AddProduction(const std::wstring &production_left, const std::vector<std::wstring> &production_right);  // 添加一条产生式
        void RunParsing();
        void Clear();
        void GetGrammar(std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> &productions); // 获取拓展文法，主要就是增加了一个 S'->S$
        void GetStates(std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &states);  // 获取所有状态
        void GetParsingTable(std::map<std::tuple<int, std::wstring>, std::wstring> &parsing_table);  // 获取分析表

    private:
        void Preprocess();  // 一些预处理部分
        void Building();  // LR0 分析表构建
        item_set Goto(item_set set, token x);  // 返回项目集 set 经过 x 转换后的项目集
        void Closure(item_set &set);  // 求项目集 set 的闭包，主要是针对项目集中还包含 A->a.B 这种项目的情况
        void ReData();  // 用来准备一些返回数据，以便 Get 函数调用获取

        token m_dot;  // 点号，默认使用中文的·符号表示
        token m_start_token;  // 起始非终结符
        token m_end_of_file;  // 结束符，国内部分教材使用 #，国外使用 $，默认使用 $
        std::tuple<token, std::vector<token>> m_start_item;  // 起始项目 S'->.S$
        std::set<token> m_nonterminal;  // 非终结符集合
        std::set<token> m_terminal;  // 终结符集合
        std::set<token> m_nt;  // 非终结符和终结符集合
        std::vector<std::tuple<token, std::vector<token>>> m_productions;  // 拓展文法表
        std::map<std::tuple<token, std::vector<token>>, int> m_production_index;
        std::map<token, std::set<std::vector<token>>> m_productions_map;
        std::map<token, item_set> m_items_start;  // 非终结符 token 对应产生式的所有项目初始集合
        std::map<item_set, int> m_state_map;  // state 表
        std::vector<item_set> m_state_set;  // state 集，以 vector 顺序存储，方便索引
        std::map<std::tuple<int, token>, int> m_action_table;  // action 表
        std::map<std::tuple<int, token>, int> m_goto_table;  // goto 表
        std::set<int> m_reduce_state;  // // 用来判断是否是规约态，如果不是规约态，则会发生转移
        std::map<std::tuple<int, token>, std::wstring> m_parsing_table;  // 返回用于界面显示的分析表
    };
}