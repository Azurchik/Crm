#ifndef RECORDDIALOG_H
#define RECORDDIALOG_H

#include <QDialog>

#include "DatabaseController.h"

namespace Ui {
class RecordDialog;
}

class RecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecordDialog(DatabaseController &db, QWidget *parent = nullptr);
    ~RecordDialog();

    void initRecord(int id);

private slots:
    void loadServices();
    void onUpdate();

private:
    void readSettings();
    void writeSettings();

    Record mRec;

    QList<Status> mStatuses;
    QList<Worker> mWorkers;
    QList<Service> mServices;
    QList<Category> mCategories;

    Ui::RecordDialog *ui;
    DatabaseController &mDb;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // RECORDDIALOG_H
