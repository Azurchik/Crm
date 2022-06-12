#ifndef CRM_TABLEMODEL_H
#define CRM_TABLEMODEL_H

#include <QAbstractTableModel>
#include <QDate>
#include <QMap>

#include "DatabaseController.h"

class CrmTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CrmTableModel(const QDate &date, const DatabaseController &db,
                           QObject *parent = nullptr);

    int recordIdByRow(int row) const;
    QString getStatusesInfo();

public slots:
    void dateChanged(const QDate &date);
    void resetTable();

private:
    void sortRecords(QList<Rcrd> &records);

private:
    QList<Rcrd> mRecords;
    const DatabaseController &mDb;

    int  mYear;
    int  mMonth;
    bool mHideWorker = true;


    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif // CRM_TABLEMODEL_H
