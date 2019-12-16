#include "LR0Parsing.h"
#include <iostream>

void PrintTuple(std::tuple<int, std::wstring, std::wstring, std::wstring, std::wstring> &data)
{
    std::wcout << L"步骤：" << std::get<0>(data) << std::endl;
    std::wcout << L"状态栈：" << std::get<1>(data) << std::endl;
    std::wcout << L"符号栈：" << std::get<2>(data) << std::endl;
    std::wcout << L"输入字符串：" << std::get<3>(data) << std::endl;
    std::wcout << L"分析动作：" << std::get<4>(data) << std::endl;
    std::wcout << std::endl;
    auto x = L'\n';
}

int wmain()
{
    std::wcout.imbue(std::locale("chs")); // 可以正常输出带有中文的字符串

    {
        M6::LR0Parsing parser;
        parser.SetStartToken(L"S");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"E", L"*", L"F"});
        parser.AddProduction(L"E", std::vector<std::wstring>{L"(", L"X", L"+", L"Y", L")"});
        parser.AddProduction(L"F", std::vector<std::wstring>{L"(", L"X", L"+", L"Y", L")"});
        parser.AddProduction(L"X", std::vector<std::wstring>{L"i"});
        parser.AddProduction(L"Y", std::vector<std::wstring>{L"i"});
        parser.RunParsing();
    }

    {
        M6::LR0Parsing parser;
        parser.SetStartToken(L"S");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"(", L"E", L")"});
        parser.AddProduction(L"E", std::vector<std::wstring>{L"E", L"*", L"F"});
        parser.AddProduction(L"E", std::vector<std::wstring>{L"E", L"+", L"F"});
        parser.AddProduction(L"E", std::vector<std::wstring>{L"F"});
        parser.AddProduction(L"F", std::vector<std::wstring>{L"n"});
        parser.SetNextTokenInput(std::vector<std::wstring>{L"(", L"n", L"+", L"n", L"+", L"n", L")"});
        parser.RunParsing();
    }

    {
        M6::LR0Parsing parser;
        parser.SetStartToken(L"S");
        parser.AddProduction(L"S", std::vector<std::wstring>{L"x", L"x", L"T"});
        parser.AddProduction(L"T", std::vector<std::wstring>{L"y"});
        parser.RunParsing();
        parser.SetNextTokenInput(std::vector<std::wstring>{L"x", L"x", L"y"});
        parser.LR0ParsingInit();
        auto re = std::make_tuple(0, std::wstring(L""), std::wstring(L""), std::wstring(L""), std::wstring(L""));
        parser.ReNextStep(re);
        PrintTuple(re);
        while (parser.RunCurStep())
        {
            parser.ReNextStep(re);
            PrintTuple(re);
        }
        parser.ReNextStep(re);
        PrintTuple(re);
    }

    {
        M6::LR0Parsing parser;
        for (int i = 0; i < 1000; i++)
        {
            parser.SetStartToken(L"S");
            parser.AddProduction(L"S", std::vector<std::wstring>{L"(", L"L", L")"});
            parser.AddProduction(L"S", std::vector<std::wstring>{L"x"});
            parser.AddProduction(L"L", std::vector<std::wstring>{L"S"});
            parser.AddProduction(L"L", std::vector<std::wstring>{L"L", L",", L"S"});
            parser.RunParsing();
            parser.Clear();
        }
    }

    system("pause");
    return 0;
}