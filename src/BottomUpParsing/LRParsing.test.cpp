#include "LRParsing.h"

#include <iostream>

void PrintGrammarType(std::wstring key, std::wstring type)
{
    std::wcout << L"Correct Result: " << key << L"; Actual Output: " << type << L"." << std::endl;
}

void PrintPArsingTokensResult(std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>> &parsing_process)
{
    for (auto i : parsing_process)
    {
        std::wcout << std::get<0>(i) << L"; ";
        std::wcout << std::get<1>(i) << L"; ";
        std::wcout << std::get<2>(i) << L"; ";
        std::wcout << std::get<3>(i) << L"; ";
        
        auto temp = std::get<4>(i);
        if (temp.front() == L'r')
        {
            auto pos = temp.find(L' ');
            auto str = std::wstring(L"用第 ") + temp.substr(1, pos) + std::wstring(L" 条规则") + temp.substr(pos) + std::wstring(L" 规约");
            temp = str;
        }
        std::wcout << temp << std::endl;
    }
}

int main()
{
    std::wcout.imbue(std::locale("chs")); // 可以正常输出带有中文的字符串

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"(", L"S", L")"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LR(0)", parser.GetGrammarType());
    }

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
        PrintGrammarType(L"SLR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"S", L"b"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"S", L"d"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L""});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"SLR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"(", L"S", L")"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L""});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"SLR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"L", L"=", L"R"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"R"});
        parser.AddProduction(L"L", std::vector<std::wstring>{L"*", L"R"});
        parser.AddProduction(L"L", std::vector<std::wstring>{L"i"});
        parser.AddProduction(L"R", std::vector<std::wstring>{L"L"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LALR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"A", L"d"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b", L"B", L"d"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"B", L"e"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b", L"A", L"e"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"x"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"x"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"B", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"c", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"d"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LR(0)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"r", L"D"});
        parser.AddProduction(L"D", std::vector<std::wstring>{L"D", L",", L"i"});
        parser.AddProduction(L"D", std::vector<std::wstring>{L"i"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"SLR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"A", L"S"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"S", L"A"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"a"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"Not LR grammar", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"A"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b", L"B"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"0", L"A"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"1"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"0", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"1"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LR(0)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"A"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"B"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"a", L"A", L"b"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"c"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"a", L"B", L"b"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"b"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LR(0)", parser.GetGrammarType());

        auto parsing_process = std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>>();
        parser.ParsingTokens(std::vector<std::wstring>{L"a", L"a", L"c", L"b", L"b"}, parsing_process);
        PrintPArsingTokensResult(parsing_process);
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"A", L"c", L"B", L"e"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"b"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"A", L"b"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"d"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LR(0)", parser.GetGrammarType());

        auto parsing_process = std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>>();
        parser.ParsingTokens(std::vector<std::wstring>{L"a", L"b", L"b", L"c", L"d", L"e"}, parsing_process);
        PrintPArsingTokensResult(parsing_process);
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"A", L"a"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b", L"A", L"c"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"B", L"c"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b", L"B", L"a"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"d"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"d"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"LR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"i", L"S", L"e", L"S"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"i", L"S"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"Not LR grammar", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"S", L"a", L"b"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b", L"R"});
        parser.AddProduction(L"R", std::vector<std::wstring>{L"S"});
        parser.AddProduction(L"R", std::vector<std::wstring>{L"a"});
        parser.BuildLRParsingTable();
        PrintGrammarType(L"Not LR grammar", parser.GetGrammarType());
    }

    system("pause");
    return 0;
}