#include "LRParsing.h"

#include <iostream>

void PrintGrammarType(int type)
{
    switch (type)
    {
    case 0:
        std::cout << "Not LR Grammar." << std::endl;
        break;
    case 1:
        std::cout << "LR(0) Grammar." << std::endl;
        break;
    case 2:
        std::cout << "SLR(1) Grammar." << std::endl;
        break;
    case 3:
        std::cout << "LALR(1) Grammar." << std::endl;
        break;
    case 4:
        std::cout << "LR(1) Grammar." << std::endl;
        break;
    default:
        break;
    }
}

int main()
{
    // 测试 LR(0) 文法
    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"(", L"S", L")"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a"});
        parser.BuildLRParsingTable();
        PrintGrammarType(parser.GetGrammarType());
    }

    // 测试 SLR(1) 文法
    {
        M6::LRParsing parser;
        parser.SetStartToken(L"E", L"E'");
        parser.AddProduction(L"E", std::vector<std::wstring>{L"E", L"+", L"T"});
        parser.AddProduction(L"E", std::vector<std::wstring>{L"T"});
        parser.AddProduction(L"T", std::vector<std::wstring>{L"T", L"*", L"F"});
        parser.AddProduction(L"T", std::vector<std::wstring>{L"F"});
        parser.AddProduction(L"F", std::vector<std::wstring>{L"(", L"E", L")"});
        parser.AddProduction(L"F", std::vector<std::wstring>{L"id"});
        parser.BuildLRParsingTable();
        PrintGrammarType(parser.GetGrammarType());
    }

    // 测试 LR(1) 文法
    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"L", L"=", L"R"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"R"});
        parser.AddProduction(L"L", std::vector<std::wstring>{L"*", L"R"});
        parser.AddProduction(L"L", std::vector<std::wstring>{L"i"});
        parser.AddProduction(L"R", std::vector<std::wstring>{L"L"});
        parser.BuildLRParsingTable();
        PrintGrammarType(parser.GetGrammarType());
    }

    system("pause");
    return 0;
}