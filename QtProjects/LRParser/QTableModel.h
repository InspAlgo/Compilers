#ifndef QTableModel_H
#define QTableModel_H

#include <QAbstractTableModel>
#include <QMap>
#include <QVector>
#include <QVariantMap>

class QTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit QTableModel(QObject *parent = nullptr);
    Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void set_table_data(QVariantList rows_header, QVariantList columns_header, QVariantMap table);
    Q_INVOKABLE void clear();

private:
    QVector<QString> m_rows_header;
    QVector<QString> m_columns_header;
    QMap<QString, QString> m_table;
};

#endif // QTableModel_H
