#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <tuple>

namespace M6
{
    using token = std::wstring;
    using item_set = std::set<std::tuple<token, std::vector<token>>>;

    class LR0Parsing
    {
    public:
        LR0Parsing();
        void SetStartToken(token start_token);
        void SetDot(token dot);
        void SetEndOfFile(token end_of_file);
        void AddProduction(const token &production_left, const std::vector<token> &production_right);  // 添加一条产生式
        void RunParsing();
        void Clear();

    private:
        void Preprocess();  // 一些预处理部分
        void Building();  // LR0 分析表构建
        item_set Goto(item_set set, token x);  // 返回项目集 set 经过 x 转换后的项目集
        void Closure(item_set &set);  // 求项目集 set 的闭包，主要是针对项目集中还包含 A->a.B 这种项目的情况

        token m_dot;  // 点号，默认使用中文的·符号表示
        token m_start_token;  // 起始非终结符
        token m_end_of_file;  // 结束符，国内部分教材使用 #，国外使用 $，默认使用 $
        std::tuple<token, std::vector<token>> m_start_item;  // 起始项目 S'->.S$
        std::set<token> m_nonterminal;  // 非终结符集合
        std::set<token> m_terminal;  // 终结符集合
        std::set<token> m_nt;  // 非终结符和终结符集合
        std::vector<std::tuple<token, std::vector<token>>> m_productions;
        std::map<std::tuple<token, std::vector<token>>, int> m_production_index;
        std::map<token, std::set<std::vector<token>>> m_productions_map;
        std::map<token, item_set> m_items_start;  // 非终结符 token 对应产生式的所有项目初始集合
        std::map<item_set, int> m_state_map;  // state 表
        std::vector<item_set> m_state_set;
        std::map<std::tuple<int, token>, int> m_action_table;  // action 表
        std::map<std::tuple<int, token>, int> m_goto_table;  // goto 表
        std::set<int> m_reduce_state;  // // 用来判断是否是规约态，如果不是规约态，则会发生转移
    };
}