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
        void AddProduction(const token &production_left, const std::vector<token> &production_right);  // ���һ������ʽ
        void RunParsing();
        void Clear();

    private:
        void Preprocess();  // һЩԤ������
        void Building();  // LR0 ��������
        item_set Goto(item_set set, token x);  // ������Ŀ�� set ���� x ת�������Ŀ��
        void Closure(item_set &set);  // ����Ŀ�� set �ıհ�����Ҫ�������Ŀ���л����� A->a.B ������Ŀ�����

        token m_dot;  // ��ţ�Ĭ��ʹ�����ĵġ����ű�ʾ
        token m_start_token;  // ��ʼ���ս��
        token m_end_of_file;  // �����������ڲ��ֽ̲�ʹ�� #������ʹ�� $��Ĭ��ʹ�� $
        std::tuple<token, std::vector<token>> m_start_item;  // ��ʼ��Ŀ S'->.S$
        std::set<token> m_nonterminal;  // ���ս������
        std::set<token> m_terminal;  // �ս������
        std::set<token> m_nt;  // ���ս�����ս������
        std::vector<std::tuple<token, std::vector<token>>> m_productions;
        std::map<std::tuple<token, std::vector<token>>, int> m_production_index;
        std::map<token, std::set<std::vector<token>>> m_productions_map;
        std::map<token, item_set> m_items_start;  // ���ս�� token ��Ӧ����ʽ��������Ŀ��ʼ����
        std::map<item_set, int> m_state_map;  // state ��
        std::vector<item_set> m_state_set;
        std::map<std::tuple<int, token>, int> m_action_table;  // action ��
        std::map<std::tuple<int, token>, int> m_goto_table;  // goto ��
        std::set<int> m_reduce_state;  // // �����ж��Ƿ��ǹ�Լ̬��������ǹ�Լ̬����ᷢ��ת��
    };
}