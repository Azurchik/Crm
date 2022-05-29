#ifndef CRM_TABLEMODEL_H
#define CRM_TABLEMODEL_H

#include <QAbstractTableModel>
#include <QDate>
#include <QHash>

#include "DatabaseController.h"

class CrmTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CrmTableModel(const QDate &date, QObject *parent = nullptr);

private:
    int weeksInMonth() const;
    void initTable();

    QDate mDate;
    DatabaseController db;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif // CRM_TABLEMODEL_H
