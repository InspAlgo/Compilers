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
        void Clear();

    public:  // 构建分析表模块
        void SetStartToken(std::wstring start_token);
        void SetDot(std::wstring dot);
        void SetEndOfFile(std::wstring end_of_file);
        void AddProduction(const std::wstring &production_left, const std::vector<std::wstring> &production_right);  // 添加一条产生式
        void RunParsing();
        void GetGrammar(std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> &productions); // 获取拓展文法，主要就是增加了一个 S'->S$
        void GetStates(std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &states);  // 获取所有状态
        void GetColumnsHeader(std::vector<std::wstring> &columns_header);  // 分析表列表头
        void GetParsingTable(std::map<std::tuple<int, std::wstring>, std::wstring> &parsing_table);  // 获取分析表

    public:  // 使用生成的分析表进行字符串分析模块，这里的字符串是以终结符构成的文法字符串
        void SetNextTokenInput(const std::vector<std::wstring> &next_tokens);  // 设置需要进行分析的字符串，不需要带 $，会自动添加
        void LR0ParsingInit();  // LR(0) 分析初始
        bool RunCurStep();  // 执行当前步操作
        void ReNextStep(std::tuple<int, std::wstring, std::wstring, std::wstring, std::wstring> &data);  // 返回每一步分析的结果，分别是 <状态栈,符号栈,输入字符串> -1 表示退栈， 0表示不动，1表示进栈

    public:
        class StateType
        {
        public:
            static const int None;  // 无类型 0
            static const int Shift;  // 移进 1<<1
            static const int Reduce;  // 规约 1<<2
            static const int ShiftReduce;  // 移进-规约冲突 1<<3
            static const int ReduceReduce;  // 规约-规约冲突 1<<4
        };

    private:  // 构建分析表模块
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
        std::map<std::tuple<token, std::vector<token>>, int> m_production_index;  // 可规约项目表
        std::map<token, std::set<std::vector<token>>> m_productions_map;  // 输入的所有产生式，即文法
        std::map<token, item_set> m_items_start;  // 非终结符 token 对应产生式的所有项目初始集合
        std::map<item_set, int> m_state_map;  // state 表
        std::vector<item_set> m_state_set;  // state 集，以 vector 顺序存储，方便索引
        std::vector<int> m_state_type;  // state 集类型，0-无类型; 1<<1 - 移进, 1<<2 - 规约
        std::map<std::tuple<int, token>, int> m_action_table;  // action 表
        std::map<std::tuple<int, token>, int> m_goto_table;  // goto 表
        std::map<std::tuple<int, token>, std::wstring> m_parsing_table;  // 返回用于界面显示的分析表

    private:  // 字符串分析模块
        token NextToken();
        int NextState();
        void ParsingStackToStrData(int index, std::wstring parsing_action);

        std::vector<token> m_next_tokens;  // 需要进行分析的字符串，这里使用逆序存储，方便 pop
        std::tuple<int, std::wstring, std::wstring, std::wstring, std::wstring> m_parsing_stack_cur_data;  // 分析栈当前操作数据 <步骤, 状态栈，符号栈，输入符号串，分析动作>
        std::vector<int> m_state_index_stack;  // 状态栈
        std::vector<token> m_token_stack;  // 符号栈，规约在这里发生
    };
}