#ifndef SERVICESDIALOG_H
#define SERVICESDIALOG_H

#include <QMap>
#include <QDialog>
#include <QAbstractButton>

#include "DatabaseController.h"

class QTableWidgetItem;

namespace Ui {
class ServicesDialog;
}

class ServicesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServicesDialog(DatabaseController &db, QWidget *parent = nullptr);
    ~ServicesDialog();

private slots:    
    void onDialogButtons(QAbstractButton *bttn);

    void onServRem();
    void onCatgRem();

    void onServAdd();
    void onCatgAdd();

    void onSetColor();
    void onCatgChanged(int index);

    void onServEdited(QTableWidgetItem *item);
    void onCatgNameEdited(const QString &name);

private:
    void save();
    void reset();

    void checkEdits();
    void setServUi(const Service &serv, int row);
    void setColorUi(const QColor &color);
    void loadCategories();

    void connections();
    void readSettigns();
    void writeSettigns();

private:
    bool mIsCreate;
    int miCatg;
    QList<int> mServToDelete;
    QList<bool> mServEdit;
    QList<bool> mCatgEdit; // by id

private:
    QList<Service> mServices;
    QList<Category> mCategories;

    Ui::ServicesDialog *ui;
    DatabaseController &mDb;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // SERVICESDIALOG_H
