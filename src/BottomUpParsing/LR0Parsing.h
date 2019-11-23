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
        void AddProduction(const std::wstring &production_left, const std::vector<std::wstring> &production_right);  // ���һ������ʽ
        void RunParsing();
        void Clear();
        void GetGrammar(std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> &productions); // ��ȡ��չ�ķ�����Ҫ����������һ�� S'->S$
        void GetStates(std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> &states);  // ��ȡ����״̬
        void GetParsingTable(std::map<std::tuple<int, std::wstring>, std::wstring> &parsing_table);  // ��ȡ������

    private:
        void Preprocess();  // һЩԤ������
        void Building();  // LR0 ��������
        item_set Goto(item_set set, token x);  // ������Ŀ�� set ���� x ת�������Ŀ��
        void Closure(item_set &set);  // ����Ŀ�� set �ıհ�����Ҫ�������Ŀ���л����� A->a.B ������Ŀ�����
        void ReData();  // ����׼��һЩ�������ݣ��Ա� Get �������û�ȡ

        token m_dot;  // ��ţ�Ĭ��ʹ�����ĵġ����ű�ʾ
        token m_start_token;  // ��ʼ���ս��
        token m_end_of_file;  // �����������ڲ��ֽ̲�ʹ�� #������ʹ�� $��Ĭ��ʹ�� $
        std::tuple<token, std::vector<token>> m_start_item;  // ��ʼ��Ŀ S'->.S$
        std::set<token> m_nonterminal;  // ���ս������
        std::set<token> m_terminal;  // �ս������
        std::set<token> m_nt;  // ���ս�����ս������
        std::vector<std::tuple<token, std::vector<token>>> m_productions;  // ��չ�ķ���
        std::map<std::tuple<token, std::vector<token>>, int> m_production_index;
        std::map<token, std::set<std::vector<token>>> m_productions_map;
        std::map<token, item_set> m_items_start;  // ���ս�� token ��Ӧ����ʽ��������Ŀ��ʼ����
        std::map<item_set, int> m_state_map;  // state ��
        std::vector<item_set> m_state_set;  // state ������ vector ˳��洢����������
        std::map<std::tuple<int, token>, int> m_action_table;  // action ��
        std::map<std::tuple<int, token>, int> m_goto_table;  // goto ��
        std::set<int> m_reduce_state;  // // �����ж��Ƿ��ǹ�Լ̬��������ǹ�Լ̬����ᷢ��ת��
        std::map<std::tuple<int, token>, std::wstring> m_parsing_table;  // �������ڽ�����ʾ�ķ�����
    };
}