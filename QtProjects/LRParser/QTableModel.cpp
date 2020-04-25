#include "QTableModel.h"
#include <QtDebug>

QTableModel::QTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant QTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole){
        if(orientation == Qt::Horizontal)
            return  QString("hor-%1").arg(section);
        else
            return QString("ver-%1").arg(section);
    }
    return QVariant();
}

int QTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rows_header.size();
}

int QTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_columns_header.size();
}

QVariant QTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if(role == Qt::DisplayRole
            && index.row() >= 0 && index.row() < rowCount()
            && index.column() >= 0 && index.column() < columnCount())
    {
        return QString(m_table[m_rows_header[index.row()] + m_columns_header[index.column()]]);
    }

    return QVariant();
}

void QTableModel::set_table_data(QVariantList rows_header, QVariantList columns_header, QVariantMap table)
{
    m_rows_header.clear();
    for(auto i : rows_header)
        m_rows_header.push_back(i.toString());

    m_columns_header.clear();
    for(auto i : columns_header)
        m_columns_header.push_back(i.toString());

    m_table.clear();
    for(auto i : m_rows_header)
    {
        for(auto j : m_columns_header)
            m_table[i+j] = "";
    }

    for(auto i = table.begin(); i != table.end(); ++i)
    {
        m_table[i.key()] = i.value().toString();
    }
}

void QTableModel::clear()
{
    m_rows_header.clear();
    m_columns_header.clear();
    m_table.clear();
}
