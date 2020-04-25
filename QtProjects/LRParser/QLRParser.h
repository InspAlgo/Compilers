#ifndef QLRPARSER_H
#define QLRPARSER_H

#include <QObject>

#include "LRParsing.h"
#include <QVariantMap>

class QLRParser : public QObject
{
    Q_OBJECT
public:
    explicit QLRParser(QObject *parent = nullptr);

    Q_INVOKABLE void build_parsing_table(QString start_token, QString new_start_token, QString input_grammar);

    Q_INVOKABLE QString get_grammar_type();  // 获取文法类型
    Q_INVOKABLE QStringList get_expanding_grammar();  // 获取拓广文法
    Q_INVOKABLE QStringList get_items_sets();  // 获取项目集
    Q_INVOKABLE QStringList get_dfa();  // 获取识别活前缀的 DFA
    Q_INVOKABLE QStringList get_col_header();  // 获取分析表列头
    Q_INVOKABLE QStringList get_row_header();  // 获取分析表行头 需要先调用 get_items_sets()
    Q_INVOKABLE QVariantMap get_parsing_table();  // 获取分析表

    Q_INVOKABLE QVariantMap parsing_tokens(QString tokens); // 分析输入符号串，返回分析过程
    Q_INVOKABLE QStringList get_parsing_process_step_count();  // 获取分析过程表行头 需要先调用 parsing_tokens


private:
    void GetItemsSets();

    QString m_end_of_file;

    M6::LRParsing m_parser;

    bool m_LR0;
    bool m_SLR1;
    bool m_LALR1;
    bool m_LR1;

    int m_items_sets_count;
    QStringList m_items_sets;
    QStringList m_parsing_process_step_count;

    std::set<std::tuple<std::wstring, std::vector<std::wstring>>> m_original_grammar;
};

#endif // QLRPARSER_H
