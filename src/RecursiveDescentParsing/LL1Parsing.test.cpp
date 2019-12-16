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
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c", L"B" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c", L"C" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"c"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"f"});
        parser.AddProduction(L"B", std::vector<M6::token>{ L"x", L"c", L"d" });
        parser.AddProduction(L"B", std::vector<M6::token>{ L"e", L"f"});
        parser.AddProduction(L"C", std::vector<M6::token>{ L"g"});
        parser.AddProduction(L"C", std::vector<M6::token>{ L""});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c", L"C" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"c"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"c"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"c", L"B"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"D", L"C"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"B"});
        parser.AddProduction(L"B", std::vector<M6::token>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: ExtractLeftFactoring
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"D", L"C"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"B"});
        parser.AddProduction(L"B", std::vector<M6::token>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: RemovingDirectLeftRecursion
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<M6::token>{ L"A", L"c" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"b"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"B"});
        parser.AddProduction(L"B", std::vector<M6::token>{ L"b", L"c", L"C", L"D"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: RemovingDirectLeftRecursion
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<M6::token>{ L"A", L"B" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"A", L"C"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"B"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: RemovingDirectLeftRecursion
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"A", std::vector<M6::token>{ L"A", L"B" });
        parser.AddProduction(L"A", std::vector<M6::token>{ L"A", L"C"});
        parser.SetStartToken(L"A");
        parser.RunParsing();
    }

    /*
        Test Method: SimplifyGrammar
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"S", std::vector<M6::token>{ L"A", L"B", L"a"});
        parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"d", L"C" });
        parser.AddProduction(L"B", std::vector<M6::token>{ L"b", L"d", L"E" });
        parser.AddProduction(L"E", std::vector<M6::token>{ L"b", L"d", L"x" });
        parser.AddProduction(L"F", std::vector<M6::token>{ L"b", L"d" });
        parser.AddProduction(L"G", std::vector<M6::token>{ L"h", L"d" });
        parser.SetStartToken(L"S");
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"Z", std::vector<M6::token>{ L"d" });
        parser.AddProduction(L"Z", std::vector<M6::token>{ L"X", L"Y", L"Z" });
        parser.AddProduction(L"Y", std::vector<M6::token>{ L"" });
        parser.AddProduction(L"Y", std::vector<M6::token>{ L"c" });
        parser.AddProduction(L"X", std::vector<M6::token>{ L"Y" });
        parser.AddProduction(L"X", std::vector<M6::token>{ L"a" });
        parser.SetStartToken(L"Z");
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"S", std::vector<M6::token>{ L"E", L"$" });
        parser.AddProduction(L"E", std::vector<M6::token>{ L"T", L"E\'"});
        parser.AddProduction(L"E\'", std::vector<M6::token>{ L"+", L"T", L"E\'" });
        parser.AddProduction(L"E\'", std::vector<M6::token>{ L"-", L"T", L"E\'" });
        parser.AddProduction(L"E\'", std::vector<M6::token>{ L"" });
        parser.AddProduction(L"T", std::vector<M6::token>{ L"F", L"T\'" });
        parser.AddProduction(L"T\'", std::vector<M6::token>{ L"*", L"F", L"T\'" });
        parser.AddProduction(L"T\'", std::vector<M6::token>{ L"/", L"F", L"T\'" });
        parser.AddProduction(L"T\'", std::vector<M6::token>{ L"" });
        parser.AddProduction(L"F", std::vector<M6::token>{ L"id" });
        parser.AddProduction(L"F", std::vector<M6::token>{ L"num" });
        parser.AddProduction(L"F", std::vector<M6::token>{ L"(", L"E", L")" });
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"С��", std::vector<M6::token>{ L"ϲ��", L"С��" });
        parser.AddProduction(L"С��", std::vector<M6::token>{ L"ϲ��", L"С��"});
        parser.AddProduction(L"С��", std::vector<M6::token>{ L"��", L"����ѧ��" });
        parser.AddProduction(L"С��", std::vector<M6::token>{ L"��", L"С��", L"����" });
        parser.AddProduction(L"С��", std::vector<M6::token>{ L"����", L"С��"});
        parser.SetStartToken(L"С��");
        parser.RunParsing();
    }

    /*
        Test Method: CalcNullable CalcFirstSet CalcFollowSet CalcFirstSSet
                     CalcPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"PROGRAM", std::vector<M6::token>{ L"begin", L"d" ,L";", L"S", L"end"});
        parser.AddProduction(L"S", std::vector<M6::token>{ L"d", L";", L"S"});
        parser.AddProduction(L"S", std::vector<M6::token>{ L"s", L"T"});
        parser.AddProduction(L"T", std::vector<M6::token>{ L"" });
        parser.AddProduction(L"T", std::vector<M6::token>{ L";", L"s", L"T" });
        parser.SetStartToken(L"PROGRAM");
        parser.RunParsing();
    }

    /*
        Test Method: GetPredictiveParsingTable
    */
    {
        M6::LL1Parsing parser;
        parser.AddProduction(L"Z", std::vector<M6::token>{ L"d" });
        parser.AddProduction(L"Z", std::vector<M6::token>{ L"X", L"Y", L"Z"});
        parser.AddProduction(L"Y", std::vector<M6::token>{ L"" });
        parser.AddProduction(L"Y", std::vector<M6::token>{ L"c" });
        parser.AddProduction(L"X", std::vector<M6::token>{ L"Y" });
        parser.AddProduction(L"X", std::vector<M6::token>{ L"a" });
        parser.SetStartToken(L"Z");
        parser.RunParsing();

        auto parsing_table = std::map<std::tuple<M6::token, M6::token>,
            std::set<std::tuple<M6::token, std::vector<M6::token>>>>();

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
            parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c", L"B" });
            parser.AddProduction(L"A", std::vector<M6::token>{ L"a", L"c", L"C" });
            parser.AddProduction(L"A", std::vector<M6::token>{ L"b", L"c"});
            parser.AddProduction(L"A", std::vector<M6::token>{ L"f"});
            parser.AddProduction(L"B", std::vector<M6::token>{ L"x", L"c", L"d" });
            parser.AddProduction(L"B", std::vector<M6::token>{ L"e", L"f"});
            parser.AddProduction(L"C", std::vector<M6::token>{ L"g"});
            parser.AddProduction(L"C", std::vector<M6::token>{ L""});
            parser.SetStartToken(L"A");
            parser.RunParsing();
            parser.Clear();

        }
    }

    return 0;
}