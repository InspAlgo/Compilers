#ifndef QTODFA_H
#define QTODFA_H

#include <QObject>
#include <QDir>

class QToDFA : public QObject
{
    Q_OBJECT

public:
    explicit QToDFA(QObject *parent = nullptr);

    Q_INVOKABLE QString to_dfa(QString nfa);
    Q_INVOKABLE QString get_dir();
    Q_INVOKABLE void remove_temp_files();
};

#endif // QTODFA_H
