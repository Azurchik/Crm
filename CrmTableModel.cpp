#include "CrmTableModel.h"

#include <QBrush>
#include <QLocale>

CrmTableModel::CrmTableModel(const QDate &date, QObject *parent)
    : QAbstractTableModel{parent}
{
    mDate.setDate(date.year(), date.month(), 1);   
}
int CrmTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())  {
        return 0;
    }
    return weeksInMonth() * 2;
}

int CrmTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())  {
        return 0;
    }

    return 7;
}

QVariant CrmTableModel::data(const QModelIndex &index, int role) const
{
//    if (role == Qt::DisplayRole) {
//        int row = index.row() + 1;
//        int col = index.column() + 1;

//        if ((row % 2) && (mDate.dayOfWeek() == col)) {
//            int day = mDate.day();
//            mDate.addDays(1);

////            mDaysToCell.


//            return day;
//        }
//    }
//    else if (role == Qt::BackgroundRole) {
//        int row = index.row() + 1;

//        if (row % 2) {
//            return QBrush(Qt::lightGray);
//        }
//    }

    return QVariant();
}

QVariant CrmTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return QLocale(QLocale::English).standaloneDayName(section + 1).toUpper();
    }

    return QVariant();
}


int CrmTableModel::weeksInMonth() const
{
    QDate date(mDate.year(), mDate.year(), 1);
    int mDays = date.daysInMonth();

    mDays -= 8 - date.dayOfWeek();

    date.setDate(mDate.year(), mDate.year(), date.daysInMonth());
    mDays -= date.dayOfWeek();

    return 2 + mDays / 7;
}

void CrmTableModel::initTable()
{

}
