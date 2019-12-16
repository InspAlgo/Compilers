#include "SubsetConstruction.h"
#include <codecvt>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

int wmain(int argc, wchar_t* argv[])
{
    const std::locale utf8(std::locale(), new std::codecvt_utf8<wchar_t>);

    std::map<std::pair<std::wstring, wchar_t>, std::list<std::wstring>> NFA;
    NFA = std::map<std::pair<std::wstring, wchar_t>, std::list<std::wstring>>();

    std::set<wchar_t> character = std::set<wchar_t>();
    std::wstring start_node = L"";
    std::set<std::wstring> end_nodes = std::set<std::wstring>();

    std::wfstream read_file(argv[1], std::ios::in);
    read_file.imbue(utf8);
    if (!read_file.is_open())
    {
        std::wcout << argv[1] << std::endl;
        exit(-1);
    }
 
    std::wstring read_line = L"";
    std::wstring temp_word = L"";
    std::vector<std::wstring> lines;

    if (std::getline(read_file, read_line))
    {
        std::wstringstream stringin(read_line);
        lines.clear();
        while (stringin >> temp_word)
            lines.push_back(temp_word);

        start_node = lines[0];
        for (int i = 1; i < lines.size(); i++)
            end_nodes.insert(lines[i]);
    }

    while (std::getline(read_file, read_line))
    {
        std::wstringstream stringin(read_line);
        lines.clear();
        while (stringin >> temp_word)
            lines.push_back(temp_word);

        wchar_t c = *lines[1].data();
        if (c != L'\x3b5')
            character.insert(c);
        else
            c = L'\0';

        NFA[std::make_pair(lines[0], c)] = std::list<std::wstring>();
        for (int i = 2; i < lines.size(); i++)
            NFA[std::make_pair(lines[0], c)].push_back(lines[i]);
    }

    read_file.close();

    std::wstring new_start_node;
    std::set<std::wstring> new_end_nodes;
    std::map<std::pair<std::wstring, wchar_t>, std::wstring> new_DFA;

    SubsetConstruction test_subset = SubsetConstruction();
    test_subset.InputNFA(character, start_node, end_nodes, NFA);
    test_subset.GetDFA();

    test_subset.OutputDFA(new_start_node, new_end_nodes, new_DFA);

    std::wstring digraph = L"digraph G{\" \"[shape=none]";

    for (auto node : new_DFA)
    {
        std::wstring from_node = node.first.first;
        std::wstring c = L"";
        std::wstring to_node = node.second;
        switch (node.first.second)
        {
        case L'\0':
            c = L"ε";break;
        default:
            c.push_back(node.first.second);
            break;
        }

        digraph += L"\"" + from_node + L"\"->\"" + to_node + L"\"[label=\"" + c + L"\"]";
        digraph += L"\"" + from_node + L"\"[shape=circle]" + L"\"" + to_node + L"\"[shape=circle]";
    }
    digraph += L"\" \"->\"" + new_start_node + L"\"\"" + new_start_node + L"\"[shape=circle]";
    for (auto node : new_end_nodes)
    {
        digraph += L"\"" + node + L"\"[shape=doublecircle]";
    }
    digraph += L"}";

    std::wfstream file_dot(std::wstring(argv[1]) + L".toDFA.dot", std::ios::out);
    file_dot.imbue(utf8);
    file_dot << digraph;
    file_dot.close();

    std::wstring bat = L".\\\\Graphviz-bin\\\\dot.exe ";
    bat += std::wstring(argv[1]) + L".toDFA.dot | .\\\\Graphviz-bin\\\\gvpr.exe -c -f binarytree.gvpr | .\\\\Graphviz-bin\\\\neato.exe -n -T" + std::wstring(argv[2]) + L" -o " + std::wstring(argv[1]) + L".toDFA." + std::wstring(argv[2]);
    std::wfstream file_bat(std::wstring(argv[1]) + L".toDFA.bat", std::ios::out);
    file_bat.imbue(utf8);
    file_bat << bat;
    file_bat.close();

    return 0;
}