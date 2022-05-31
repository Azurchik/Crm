#include "CrmTableModel.h"

#include <QFont>
#include <QBrush>
#include <QLocale>
#include <QApplication>

#include "CrmConsts.h"

CrmTableModel::CrmTableModel(const QDate &date, const DatabaseController &db, QObject *parent)
    : QAbstractTableModel {parent}
    , mDb {db}
    , mYear {date.year()}
    , mMonth {date.month()}
{
    mDb.initRecordList(currRecords(), mMonth, mYear);

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

    beginResetModel();
    if (currRecords().isEmpty()) {
        mDb.initRecordList(currRecords(), mMonth, mYear);
    }
    endResetModel();
}

QList<Rcrd> &CrmTableModel::currRecords()
{
    return mRecords[mYear][mMonth];
}


int CrmTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())  {
        return 0;
    }

    return mRecords[mYear][mMonth].size();
}

int CrmTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())  {
        return 0;
    }

    return mHideWorker ? 4 : 5;
}

QVariant CrmTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    auto currRow = [&]() -> const Rcrd& {
        return mRecords[mYear][mMonth].at(index.row());
    };

    if (role == Qt::DisplayRole) {
        auto &record = currRow();

        switch (index.column()) {
        case 0: return record.date.toString("dd ddd");
        case 1: return record.time.toString(TimeFormat);
        case 2: return record.client;
        case 3: return mHideWorker ? record.service : record.worker;
        case 4: return record.service;
        }
    }
    else if (role == Qt::BackgroundRole) {
        auto &record = currRow();

        switch (index.column()) {
        case 0:
        case 1: return QBrush{record.cRecord};
        }
    }
    else if (role == Qt::DecorationRole) {
        auto &record = currRow();

        switch (index.column()) {
        case 2: return QBrush{record.cClient};
        case 3: return mHideWorker ? QBrush{record.cService} : QBrush{};
        case 4: return QBrush{record.cService};
        }
    }

    return QVariant();
}

QVariant CrmTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
//        return QLocale(QLocale::English).standaloneDayName(section + 1).toUpper();
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0: return "Day";
            case 1: return "Time";
            case 2: return mHideWorker ? "Service" : "Worker";
            case 3: return mHideWorker ? "Client"  : "Service";
            case 4: return "Client";
            }
        }
        else if (role == Qt::FontRole) {
            auto font = QApplication::font();
            font.setBold(true);
            font.setPointSize(font.pointSize() + 2);

            return font;
        }
//        else if (role == Qt::BackgroundRole) {
//            return QBrush(Qt::lightGray, Qt::SolidPattern);
//        }
    }

    return QVariant();
}
