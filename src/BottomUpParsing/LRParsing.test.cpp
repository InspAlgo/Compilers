#include "LRParsing.h"

#include <iostream>

void PrintGrammarType(std::string key, std::string type)
{
    std::cout << "Correct Result: " << key << "; Actual Output: " << type << "." << std::endl;
}

int main()
{
    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"(", L"S", L")"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a"});
        parser.BuildLRParsingTable();
        PrintGrammarType("LR(0)", parser.GetGrammarType());
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
        PrintGrammarType("SLR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"S", L"b"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"a", L"S", L"d"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L""});
        parser.BuildLRParsingTable();
        PrintGrammarType("SLR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"(", L"S", L")"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L""});
        parser.BuildLRParsingTable();
        PrintGrammarType("SLR(1)", parser.GetGrammarType());
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
        PrintGrammarType("LALR(1)", parser.GetGrammarType());
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
        PrintGrammarType("LR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"B", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"c", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{L"d"});
        parser.BuildLRParsingTable();
        PrintGrammarType("LR(0)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"r", L"D"});
        parser.AddProduction(L"D", std::vector<std::wstring>{L"D", L",", L"i"});
        parser.AddProduction(L"D", std::vector<std::wstring>{L"i"});
        parser.BuildLRParsingTable();
        PrintGrammarType("SLR(1)", parser.GetGrammarType());
    }

    {
        M6::LRParsing parser;
        parser.SetStartToken(L"S", L"S'");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"A", L"S"});
        parser.AddProduction(L"S", std::vector<std::wstring>{L"b"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"S", L"A"});
        parser.AddProduction(L"A", std::vector<std::wstring>{L"a"});
        parser.BuildLRParsingTable();
        PrintGrammarType("Not LR grammar", parser.GetGrammarType());
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
        PrintGrammarType("LR(0)", parser.GetGrammarType());
    }

    system("pause");
    return 0;
}