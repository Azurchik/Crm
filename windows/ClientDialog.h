#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QDialogButtonBox>

#include "DatabaseController.h"


namespace Ui {
class ClientDialog;
}

class ClientDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClientDialog(QWidget *parent = nullptr); // Default

    explicit ClientDialog(const DatabaseController *db, QWidget *parent); // Creation
    explicit ClientDialog(const DatabaseController *db, const Client &cln, QWidget *parent = nullptr); // ReadOnly
    ~ClientDialog();

    Client clientFromUi() const;
    void restoreTableHeader(const QByteArray &state);
    QByteArray tableHeaderState() const;

private:
    void clientToUi(const Client &cln);
    void onAccepted(QAbstractButton *button);

    int  statIndexById(int id) const;
    void loadStatuses(const DatabaseController *db);
    void loadRecords(const DatabaseController *db, int clnId);

private:
    QList<Status> mStats;

    Ui::ClientDialog *ui;
};

#endif // CLIENTDIALOG_H
