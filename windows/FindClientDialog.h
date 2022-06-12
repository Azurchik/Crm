#ifndef FINDCLIENTDIALOG_H
#define FINDCLIENTDIALOG_H

#include <QDialog>

#include "DatabaseController.h"

class QTableWidgetItem;

namespace Ui {
class FindClientDialog;
}

class FindClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindClientDialog(DatabaseController &db, QWidget *parent = nullptr);
    ~FindClientDialog();

private slots:
    void search();
    void handleSearchType();

    void createClient();
    void editClient(QTableWidgetItem *item);
    void removeClient();

    void readSettings();
    void writeSettings();

    void connections();

private:
    void addClientToTable(const Client &cln, int row);

    QList<Client> mClients;
    Ui::FindClientDialog *ui;

    DatabaseController &mDb;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // FINDCLIENTDIALOG_H
