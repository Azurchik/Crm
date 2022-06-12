#include "WorkersDialog.h"
#include "ui_WorkersDialog.h"

#include <QMessageBox>
#include <QListWidgetItem>

WorkersDialog::WorkersDialog(DatabaseController &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WorkersDialog)
    , mDb(db)
{
    ui->setupUi(this);

    mIsCreate = true;
    mDb.loadWorkers(mWorkers);
    ui->tableWorkers->setRowCount(mWorkers.size());

    for (int i = 0; i < mWorkers.size(); ++i) {
        ui->tableWorkers->setItem(i, 0,
                            new QTableWidgetItem(mWorkers[i].name) );
    }
    mIsCreate = false;

    connect(ui->pBtnAdd, &QPushButton::clicked,
            this, &WorkersDialog::add);

    connect(ui->pBtnRem, &QPushButton::clicked,
            this, &WorkersDialog::rem);

    connect(ui->tableWorkers, &QTableWidget::itemChanged,
            this, &WorkersDialog::edit);
}

WorkersDialog::~WorkersDialog()
{
    delete ui;
}

void WorkersDialog::edit(QTableWidgetItem *item)
{
    if (item == nullptr || mIsCreate) {
        return;
    }

    int row = item->row();
    if (row < 0 || row >= mWorkers.size()) {
        return;
    }

    auto &wrk = mWorkers[row];
    auto name = item->text();

    if (name.isEmpty() || mDb.workerIdByName(name) > 0)
    {
        QApplication::beep();
        item->setText(wrk.name);
        return;
    }

    QString pName = wrk.name;
    wrk.name = name;
    if (!mDb.updateWorker(wrk)) {
        QMessageBox::warning(this, "Db Error",
                             "Can't change Worker Name!");
        wrk.name = pName;
        item->setText(wrk.name);
    }
}

void WorkersDialog::add()
{
    Worker wrk;
    wrk.name = "Worker";

    if (!mWorkers.isEmpty()) {
        int index = 0;
        {
            QString name = wrk.name;
            while (mDb.workerIdByName(name) > 0) {
                name = wrk.name + QString::number(index++);
            }
            wrk.name = name;
        }
    }

    if (!mDb.addWorker(wrk)) {
        QMessageBox::warning(this, "Create Error",
                             "Can't create new Worker!");
        return;
    }

    wrk.id = mDb.workerIdByName(wrk.name);
    if (wrk.id == -1) {
        QMessageBox::warning(this, "Find Error",
                             "Can't find created Worker!");
    }
    else {
        mWorkers << wrk;
        mIsCreate = true;
        ui->tableWorkers->setItem(mWorkers.size() - 1, 0,
                                  new QTableWidgetItem(wrk.name));
        mIsCreate = false;
    }
}

void WorkersDialog::rem()
{
    int row = ui->tableWorkers->currentRow();
    if (row < 0 || row >= mWorkers.size()) {
        return;
    }

    if (mWorkers.size() == 1) {
        QMessageBox::warning(this, "Delete Error",
                             "Can't delete last Worker!");
    }
    else if (!mDb.deleteWorker(mWorkers[row].id)) {
        QMessageBox::warning(this, "Db Error",
                             "Can't delete Worker!");
    }
    else {
        mWorkers.removeAt(row);
        ui->tableWorkers->removeRow(row);
    }
}
