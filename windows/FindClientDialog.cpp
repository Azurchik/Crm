#include "FindClientDialog.h"
#include "ui_FindClientDialog.h"

#include "Consts.h"
#include "windows/ClientDialog.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>

FindClientDialog::FindClientDialog(DatabaseController &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindClientDialog)
    , mDb(db)
{
    ui->setupUi(this);

    mDb.loadClients(mClients);
    ui->tableClients->setRowCount(mClients.size());

    for (int row = 0; row < mClients.size(); ++row) {
        addClientToTable(mClients.at(row), row);
    }

    connections();
    readSettings();
}

FindClientDialog::~FindClientDialog()
{
    delete ui;
}

void FindClientDialog::search()
{
    mClients.clear();
    ui->tableClients->clearContents();

    auto text = ui->lEditSearch->text();

    if (!text.isEmpty()) {
        switch(ui->cBoxSearch->currentIndex()) {
        case 0:
            mDb.searchClientByName(text, mClients);
            break;
        case 1:
            mDb.searchClientByPhone(text, mClients, true);
            break;
        }
    }
    else {
        mDb.loadClients(mClients);
    }

    ui->tableClients->setRowCount(mClients.size());

    for (int row = 0; row < mClients.size(); ++row) {
        addClientToTable(mClients.at(row), row);
    }
}

void FindClientDialog::removeClient()
{
    auto item = ui->tableClients->selectedItems().last();

    if (item == nullptr) {
        return;
    }

    auto cln = mClients.at(item->row());

    using Btn = QMessageBox::StandardButton;
    Btn btn = QMessageBox::warning(this,
                QString("Delete Client"),
                QString("Are you sure you want to Delete the '%1'?\n\n")
                .arg(cln.name)
                + "All History with this Client will be deleted.\n"
                  "You will not be able to undo this action!",
                Btn::No | Btn::Yes,
                Btn::No );

    if (btn == Btn::No) {
        return;
    }

    mDb.deleteClient(cln.id);
    mClients.removeAt(item->row());
    ui->tableClients->removeRow(item->row());
}

void FindClientDialog::readSettings()
{
    QSettings stt (IniFName, QSettings::IniFormat);
    stt.beginGroup(objectName());

    if (stt.contains("size")) {
        resize(stt.value("size").toSize());
    }
    if (stt.contains("tableHeader")) {
        ui->tableClients->horizontalHeader()->restoreState(stt.value("tableHeader").toByteArray());
    }

    ui->cBoxSearch->setCurrentIndex(stt.value("searchIndex", 0).toInt());

}

void FindClientDialog::writeSettings()
{
    QSettings stt (IniFName, QSettings::IniFormat);
    stt.beginGroup(objectName());

    stt.setValue("size", size());
    stt.setValue("searchIndex", ui->cBoxSearch->currentIndex());
    stt.setValue("tableHeader", ui->tableClients->horizontalHeader()->saveState());

    stt.endGroup();
}

void FindClientDialog::connections()
{
    connect(ui->pBtnSearch, &QPushButton::clicked,
            this, &FindClientDialog::search);

    connect(ui->pBtnAdd, &QPushButton::clicked,
            this, &FindClientDialog::createClient);

    connect(ui->pBtnRem, &QPushButton::clicked,
            this, &FindClientDialog::removeClient);


    connect(ui->cBoxSearch, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FindClientDialog::handleSearchType);

    connect(ui->tableClients, &QTableWidget::itemDoubleClicked,
            this, &FindClientDialog::editClient);
}

void FindClientDialog::createClient()
{
    auto clnDialog = new ClientDialog (mDb, this);
    if (clnDialog->exec() == QDialog::Accepted)
    {
        auto cln = clnDialog->clientFromUi();
        if (mDb.addClient(cln))
        {
            ui->lEditSearch->clear();
            search();

            if (mClients.size() == 1) {
                ui->tableClients->selectRow(0);
            }
        }
        else {
            QMessageBox::warning(this, "Db Error",
                                 "Can't create new Client!");
        }
    }
}

void FindClientDialog::editClient(QTableWidgetItem *item)
{
    if ( item == nullptr ||
        item->row() == -1) {
        return;
    }

    auto clnDialog = new ClientDialog(mDb, mClients.at(item->row()), this, false);
    if (clnDialog->exec() == QDialog::Accepted)
    {
        auto cln = clnDialog->clientFromUi();
        if (mDb.updateClient(cln))
        {
            addClientToTable(cln, item->row());
        }
        else {
            QMessageBox::warning(this, "Db Error",
                                 "Can't update Client:\n"
                                 + mClients.at(item->row()).name);
        }
    }
}

void FindClientDialog::handleSearchType()
{
    switch(ui->cBoxSearch->currentIndex()) {
    case 1:
        ui->lblSearch->setText("Client Phone:");
        ui->lEditSearch->setInputMask("(\\0D9)-999-9999");
        break;
    case 0:
        ui->lblSearch->setText("Client Name:");
        ui->lEditSearch->setInputMask("");
        break;
    }
    ui->lEditSearch->setFocus(Qt::MouseFocusReason);
}

void FindClientDialog::addClientToTable(const Client &cln, int row)
{
    ui->tableClients->setItem(row, 1, new QTableWidgetItem(cln.name));
    ui->tableClients->setItem(row, 2, new QTableWidgetItem(cln.phone));

    auto status = mDb.getStatus(cln.stat_id);

    QTableWidgetItem *item = new QTableWidgetItem(status.name);
    item->setBackground(status.rgb);
    ui->tableClients->setItem(row, 0, item);
}

void FindClientDialog::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}
