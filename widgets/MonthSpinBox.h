#ifndef MONTHSPINBOX_H
#define MONTHSPINBOX_H

#include <QSpinBox>

class MonthSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit MonthSpinBox(QWidget *parent = nullptr);



protected:
    static QMap <int, QString> Months;

    // QSpinBox interface
protected:
    int valueFromText(const QString &text) const override;
    QString textFromValue(int val) const override;
};

#endif // MONTHSPINBOX_H
