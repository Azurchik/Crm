#ifndef WORKERSDIALOG_H
#define WORKERSDIALOG_H

#include <QDialog>

#include "DatabaseController.h"

class QTableWidgetItem;

namespace Ui {
class WorkersDialog;
}

class WorkersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WorkersDialog(DatabaseController &db, QWidget *parent = nullptr);
    ~WorkersDialog();

private slots:
    void edit(QTableWidgetItem *item);
    void add();
    void rem();

private:
    bool mIsCreate = false;
    QList<Worker> mWorkers;

    Ui::WorkersDialog *ui;
    DatabaseController &mDb;
};

#endif // WORKERSDIALOG_H
