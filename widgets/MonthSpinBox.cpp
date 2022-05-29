#include "MonthSpinBox.h"
#include <QLineEdit>



MonthSpinBox::MonthSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    auto lEdit = new QLineEdit(this);
    lEdit->setReadOnly(true);
    lEdit->setBackgroundRole(QPalette::NoRole);
    setLineEdit(lEdit);

    setRange(0, 11);
    setSingleStep(1);
}

int MonthSpinBox::valueFromText(const QString &text) const
{
    return Months.key(text, minimum());
}

QString MonthSpinBox::textFromValue(int val) const
{
    return Months.value(val, Months.begin().value());
}

QMap <int, QString> MonthSpinBox::Months = {
    { 0, "January"},
    { 1, "February"},
    { 2, "March"},
    { 3, "April"},
    { 4, "May"},
    { 5, "June"},
    { 6, "July"},
    { 7, "August"},
    { 8, "September"},
    { 9, "October"},
    {10, "November"},
    {11, "December"}
};
