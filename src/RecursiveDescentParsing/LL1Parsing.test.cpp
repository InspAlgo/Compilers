#include "LL1Parsing.h"
#include <iostream>

/*
    ���Բ��裺
       ����Ҫ���Եķ����Ķ�Ӧ�Ĵ���������ϵ㣬
       ͬʱ�ڱ��ⷽ�������ϵ㣬
       ������ص��Թ��߼���
       ������Ӧ֧�� C++11
*/

int main()
{
    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c", L"B" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c", L"C" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"c"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"f"});
        parser.AddProduction(L"B", std::vector<std::wstring>{ L"x", L"c", L"d" });
        parser.AddProduction(L"B", std::vector<std::wstring>{ L"e", L"f"});
        parser.AddProduction(L"C", std::vector<std::wstring>{ L"g"});
        parser.AddProduction(L"C", std::vector<std::wstring>{ L""});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c", L"C" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"c"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"c"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"c", L"B"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"D", L"C"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"D", L"C"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: RemovingDirectLeftRecursion
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"A", L"c" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"b"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"B"});
        parser.AddProduction(L"B", std::vector<std::wstring>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: RemovingDirectLeftRecursion
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"A", L"B" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"A", L"C"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"B"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: RemovingDirectLeftRecursion
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"A", L"B" });
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"A", L"C"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: SimplifyGrammar
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"S", std::vector<std::wstring>{ L"A", L"B", L"a"});
        parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"d", L"C" });
        parser.AddProduction(L"B", std::vector<std::wstring>{ L"b", L"d", L"E" });
        parser.AddProduction(L"E", std::vector<std::wstring>{ L"b", L"d", L"x" });
        parser.AddProduction(L"F", std::vector<std::wstring>{ L"b", L"d" });
        parser.AddProduction(L"G", std::vector<std::wstring>{ L"h", L"d" });
        parser.SetStartToken(L"S");
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"Z", std::vector<std::wstring>{ L"d" });
        parser.AddProduction(L"Z", std::vector<std::wstring>{ L"X", L"Y", L"Z" });
        parser.AddProduction(L"Y", std::vector<std::wstring>{ L"" });
        parser.AddProduction(L"Y", std::vector<std::wstring>{ L"c" });
        parser.AddProduction(L"X", std::vector<std::wstring>{ L"Y" });
        parser.AddProduction(L"X", std::vector<std::wstring>{ L"a" });
        parser.SetStartToken(L"Z");
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"S", std::vector<std::wstring>{ L"E", L"$" });
        parser.AddProduction(L"E", std::vector<std::wstring>{ L"T", L"E\'"});
        parser.AddProduction(L"E\'", std::vector<std::wstring>{ L"+", L"T", L"E\'" });
        parser.AddProduction(L"E\'", std::vector<std::wstring>{ L"-", L"T", L"E\'" });
        parser.AddProduction(L"E\'", std::vector<std::wstring>{ L"" });
        parser.AddProduction(L"T", std::vector<std::wstring>{ L"F", L"T\'" });
        parser.AddProduction(L"T\'", std::vector<std::wstring>{ L"*", L"F", L"T\'" });
        parser.AddProduction(L"T\'", std::vector<std::wstring>{ L"/", L"F", L"T\'" });
        parser.AddProduction(L"T\'", std::vector<std::wstring>{ L"" });
        parser.AddProduction(L"F", std::vector<std::wstring>{ L"id" });
        parser.AddProduction(L"F", std::vector<std::wstring>{ L"num" });
        parser.AddProduction(L"F", std::vector<std::wstring>{ L"(", L"E", L")" });
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"С��", std::vector<std::wstring>{ L"ϲ��", L"С��" });
        parser.AddProduction(L"С��", std::vector<std::wstring>{ L"ϲ��", L"С��"});
        parser.AddProduction(L"С��", std::vector<std::wstring>{ L"��", L"����ѧ��" });
        parser.AddProduction(L"С��", std::vector<std::wstring>{ L"��", L"С��", L"����" });
        parser.AddProduction(L"С��", std::vector<std::wstring>{ L"����", L"С��"});
        parser.SetStartToken(L"С��");
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"PROGRAM", std::vector<std::wstring>{ L"begin", L"d" ,L";", L"S", L"end"});
        parser.AddProduction(L"S", std::vector<std::wstring>{ L"d", L";", L"S"});
        parser.AddProduction(L"S", std::vector<std::wstring>{ L"s", L"T"});
        parser.AddProduction(L"T", std::vector<std::wstring>{ L"" });
        parser.AddProduction(L"T", std::vector<std::wstring>{ L";", L"s", L"T" });
        parser.SetStartToken(L"PROGRAM");
        parser.RunParsing();
    }

    /*
        Test Method: GetPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"Z", std::vector<std::wstring>{ L"d" });
        parser.AddProduction(L"Z", std::vector<std::wstring>{ L"X", L"Y", L"Z"});
        parser.AddProduction(L"Y", std::vector<std::wstring>{ L"" });
        parser.AddProduction(L"Y", std::vector<std::wstring>{ L"c" });
        parser.AddProduction(L"X", std::vector<std::wstring>{ L"Y" });
        parser.AddProduction(L"X", std::vector<std::wstring>{ L"a" });
        parser.SetStartToken(L"Z");
        parser.RunParsing();

        auto parsing_table = std::map<std::tuple<std::wstring, std::wstring>,
            std::set<std::tuple<std::wstring, std::vector<std::wstring>>>>();

        parser.GetPredictiveParsingTable(parsing_table);

        for (auto NT : parsing_table)
        {
            std::wcout << std::get<0>(NT.first) << L" - " << std::get<1>(NT.first) << L": ";
            for (auto production : NT.second)
            {
                auto left = std::get<0>(production);
                auto right = std::get<1>(production);

                std::wstring p_str = left + L"->";
                for (auto t : right)
                {
                    p_str += t + L" ";
                }

                std::wcout << p_str << L" ";
            }
            std::wcout << std::endl;
        }
    }

    /*
        �����ڴ��Ƿ�й©
        ���Խ������й©
    */
    {
        M6::LL1Parsing parser;
        for (int i = 0; i < 1000; i++)
        {
            parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c", L"B" });
            parser.AddProduction(L"A", std::vector<std::wstring>{ L"a", L"c", L"C" });
            parser.AddProduction(L"A", std::vector<std::wstring>{ L"b", L"c"});
            parser.AddProduction(L"A", std::vector<std::wstring>{ L"f"});
            parser.AddProduction(L"B", std::vector<std::wstring>{ L"x", L"c", L"d" });
            parser.AddProduction(L"B", std::vector<std::wstring>{ L"e", L"f"});
            parser.AddProduction(L"C", std::vector<std::wstring>{ L"g"});
            parser.AddProduction(L"C", std::vector<std::wstring>{ L""});
            parser.SetStartToken(L"A");
            parser.RunParsing();
            parser.Clear();

        }
    }

    return 0;
}