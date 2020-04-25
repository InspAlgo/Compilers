#ifndef LR0PARSER_H
#define LR0PARSER_H
#include <QObject>
#include <QVariantMap>
#include <list>
#include "LR0Parsing.h"

class LR0Parser : public QObject
{
    Q_OBJECT

public:
    explicit LR0Parser(QObject *parent = nullptr);

public:
    Q_INVOKABLE void set_start_token(QString start_token);
    Q_INVOKABLE bool add_production(QString production_left, QStringList production_right);
    Q_INVOKABLE void running();
    Q_INVOKABLE QStringList get_grammar_expanding();
    Q_INVOKABLE QStringList get_states();
    Q_INVOKABLE QStringList get_table_rowheader();
    Q_INVOKABLE QStringList get_table_colheader();
    Q_INVOKABLE QVariantMap get_parsing_table();
    Q_INVOKABLE void clear();

    Q_INVOKABLE void set_input_tokens(QStringList input_tokens);
    Q_INVOKABLE void run_tokens_parsing();
    Q_INVOKABLE QString get_step_data();

private:
    void GetData();  // 获取运行分析后的数据
    void AddStepData(std::tuple<int, std::wstring, std::wstring, std::wstring, std::wstring> &data);  // 添加每一步的信息

private:
    // LR(0) 分析器
    M6::LR0Parsing m_parser;

    // 输入到分析器里的数据
    std::wstring m_start_token;
    std::wstring m_dot;
    std::wstring m_end_of_file;
    std::set<std::tuple<std::wstring, std::vector<std::wstring>>> m_grammar;

    // 从分析器中获取的数据
    std::vector<std::tuple<std::wstring, std::vector<std::wstring>>> m_grammar_expanding;
    std::vector<std::set<std::tuple<std::wstring, std::vector<std::wstring>>>> m_states;
    std::vector<std::wstring> m_columns_header;
    std::map<std::tuple<int, std::wstring>, std::wstring> m_parsing_table;

    // 输入的符号串的每一步分析结果
    std::list<std::wstring> m_parsing_reslut;
};

#endif // LR0PARSER_H
