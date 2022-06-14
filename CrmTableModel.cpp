#include "CrmTableModel.h"

#include <QFont>
#include <QBrush>
#include <QLocale>
#include <QApplication>
#include <QDebug>

#include "Consts.h"

CrmTableModel::CrmTableModel(const QDate &date, const DatabaseController &db, QObject *parent)
    : QAbstractTableModel {parent}
    , mDb {db}
    , mYear {date.year()}
    , mMonth {date.month()}
{
    mDb.initRecordList(mRecords, mMonth, mYear);
    sortRecords(mRecords);

    mHideWorker = mDb.countWorkers() <= 1;
}

int CrmTableModel::recordIdByRow(int row) const
{
    if (row < 0 || row >= mRecords.size()) {
        return -1;
    }
    return mRecords[row].id[Record::Id];
}

void CrmTableModel::dateChanged(const QDate &date)
{
    if (date.month() == mMonth
        && date.year() == mYear)
    {
        return;
    }

    mYear = date.year();
    mMonth = date.month();

    resetTable();
}

void CrmTableModel::resetTable()
{
    beginResetModel();

    mDb.initRecordList(mRecords, mMonth, mYear);
    sortRecords(mRecords);

    mHideWorker = mDb.countWorkers() <= 1;

    endResetModel();
}

QString CrmTableModel::getStatusesInfo()
{
    QMap<QString, int> statInfo;
    for (auto &rcrd : qAsConst(mRecords)) {
        statInfo[rcrd.status] += 1;
    }

    QString res;
    for (auto it = statInfo.begin(); it != statInfo.end(); ++it) {
        res += QString("%1=%2\t")
                .arg(it.key()).arg(it.value());
    }

    return res;
}


void CrmTableModel::sortRecords(QList<Record> &records)
{
    std::sort(records.begin(), records.end(), [](const Record &r1, const Record &r2) {
        if (r1.date == r2.date) {
            return r1.time < r2.time;
        }
        return r1.date < r2.date;
    });
}


int CrmTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())  {
        return 0;
    }

    return mRecords.size();
}

int CrmTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())  {
        return 0;
    }

    return mHideWorker ? 5 : 6;
}

QVariant CrmTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    auto record = mRecords.at(index.row());

    if (role == Qt::DisplayRole)
    {
        if (mHideWorker) {
            switch (index.column())
            {
            case 0: return record.status;
            case 1: return record.date.toString(TableDateFormat);
            case 2: return record.time.toString(TimeFormat);
            case 3: return record.service;
            case 4: return record.client + ' '  + record.phone;
            }
        } else {
            switch (index.column())
            {
            case 0: return record.status;
            case 1: return record.date.toString(TableDateFormat);
            case 2: return record.time.toString(TimeFormat);
            case 3: return record.worker;
            case 4: return record.service;
            case 5: return record.client + ' '  + record.phone;
            }
        }
    }
    else if (role == Qt::BackgroundRole)
    {
        return mDb.colorByRcrdId(record.id[Record::Id]);
    }
    else if (role == Qt::DecorationRole) {
        if (mHideWorker) {
            switch (index.column())
            {
            case 3: return mDb.colorBySrvcId(record.id[Record::SrvId]);
            case 4: return mDb.colorByClntId(record.id[Record::ClnId]);
            }
        } else {
            switch (index.column())
            {
            case 4: return mDb.colorBySrvcId(record.id[Record::SrvId]);
            case 5: return mDb.colorByClntId(record.id[Record::ClnId]);
            }
        }
    }

    return QVariant();
}

QVariant CrmTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (mHideWorker) {
                switch (section) {
                case 0: return "Status";
                case 1: return "Day";
                case 2: return "Time";
                case 3: return "Service";
                case 4: return "Client";
                }
            }
            else {
                switch (section) {
                case 0: return "Status";
                case 1: return "Day";
                case 2: return "Time";
                case 3: return "Worker";
                case 4: return "Service";
                case 5: return "Client";
                }
            }
        }
        else if (role == Qt::FontRole) {
            auto font = QApplication::font();
            font.setBold(true);
            font.setPointSize(font.pointSize() + 2);

            return font;
        }
    }

    return QVariant();
}
