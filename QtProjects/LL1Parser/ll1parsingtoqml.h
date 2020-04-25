#ifndef LL1PARSINGTOQML_H
#define LL1PARSINGTOQML_H
#include <QObject>
#include <QVector>
#include <QVariantMap>
#include <vector>
#include <string>
#include <set>
#include <map>
#include "LL1Parsing.h"

class LL1ParsingToQML : public QObject
{
    Q_OBJECT
public:
    explicit LL1ParsingToQML(QObject *parent = nullptr);

    Q_INVOKABLE void test();
    Q_INVOKABLE void add_token(QString token);
    Q_INVOKABLE void add_token_left(QString token);
    Q_INVOKABLE bool add_production();
    Q_INVOKABLE void set_start_token(QString start_token);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString get_parsing_table_str();
    Q_INVOKABLE QString get_first_s_set_str();
    Q_INVOKABLE QString get_follow_set_str();
    Q_INVOKABLE QStringList get_nonterminal();
    Q_INVOKABLE QStringList get_terminal();
    Q_INVOKABLE QVariantMap get_table();

private:
    std::wstring m_start_token;
    std::vector<QString> m_production;
    std::wstring m_production_left;

    std::map<std::wstring, std::set<std::vector<std::wstring>>> m_productions_map;

    M6::LL1Parsing m_parser;

    std::vector<QString> m_re; // table str
    std::vector<QString> m_firsts;
    std::vector<QString> m_follow;
    QVector<QString> m_nonterminal_qv;
    QVector<QString> m_terminal_qv;
    std::map<QString, QString> m_table;
};

#endif // LL1PARSINGTOQML_H
