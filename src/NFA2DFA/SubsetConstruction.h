#pragma once
#include <string>
#include <set>
#include <list>
#include <map>

/*
    �Ӽ������㷨
*/
class SubsetConstruction
{
public:
    SubsetConstruction();
    void InputNFA(std::set<char> character, std::string start_node, std::set<std::string> end_nodes, std::map<std::pair<std::string, char>, std::list<std::string>> NFA);
    void GetDFA();
    void OutputDFA(std::string new_start_node, std::set<std::string> new_end_nodes, std::map<std::pair<std::string, char>, std::string> new_DFA);

private:
    void EpsilonClosure(std::string node);  // ��-�հ�����ӿ�
    std::set<std::string> EClosure(std::set<std::string> delta);
    bool Visited(std::string node);  // �жϽڵ��Ƿ���ʹ����Ƿ��� true
    void EpsilonClosureDFS(std::string node);  // ��-�հ����㣬ʹ��������ȱ����㷨ʵ��
    std::set<std::string> Delta(std::set<std::string> q, char c);
    void WorkList(std::string start_node);  // �Ӽ�������㣬ʹ�ù������㷨ʵ��
    std::string IDGenerator();  // DFA ״̬���������

    std::set<std::string> m_epsilon_closure;  // ÿ�ֱհ������ closure ����
    std::list<std::set<std::string>> m_worklist;
    std::map<std::pair<std::string, char>, std::list<std::string>> m_NFA;  // �洢 NFA ͼ�ṹ
    std::map<std::pair<std::set<std::string>, char>, std::set<std::string>> m_DFA;  // �洢 DFA ͼ�ṹ
    std::map<std::string, bool> m_visited;  // �ڽ��бհ�����ʱ�ж��Ƿ���ʹ��˽ڵ�
    std::set<char> m_character;  // ״̬ת������
    std::string m_start_node;  // ״̬��ʼ�ڵ�
    std::set<std::string> m_end_nodes;  // ״̬��ֹ�ڵ㼯��
    int m_state_id;

    std::string m_new_start_node;
    std::set<std::string> m_new_end_nodes;
    std::map<std::pair<std::string, char>, std::string> m_new_DFA;
};