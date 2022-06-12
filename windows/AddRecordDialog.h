#ifndef ADDRECORDDIALOG_H
#define ADDRECORDDIALOG_H

#include <QDialog>
#include "DatabaseController.h"

class QTableWidgetItem;

namespace Ui {
class AddRecordDialog;
}

class AddRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddRecordDialog(DatabaseController &db, QWidget *parent = nullptr);
    ~AddRecordDialog();

    void setDate(const QDate &date);
    Rcrd getRecord() const;

private slots:
    void openClientWindow(QTableWidgetItem *item);

    void handleSearchType();
    void loadPreview();
    void search();
    void createClient();
    void loadServicies();
    void onAccepted();

private:
    void readSettings();
    void writeSettings();

    void addClientToTable(const Client &cln, int row);
    void connections();

    QList<Worker> mWorkers;
    QList<Client> mClients;
    QList<Status> mStatuses;
    QList<Service> mServices;
    QList<Category> mSrvCategs;

    Ui::AddRecordDialog *ui;
    DatabaseController &mDb;    

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // ADDRECORDDIALOG_H
