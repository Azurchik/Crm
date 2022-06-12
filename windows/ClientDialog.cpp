#include "ClientDialog.h"
#include "ui_ClientDialog.h"

#include <QMessageBox>

#include "Consts.h"

ClientDialog::ClientDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &ClientDialog::onAccepted);
}

// ReadOnly Dialog
ClientDialog::ClientDialog(const DatabaseController *db, const Client &cln, QWidget *parent)
    : ClientDialog(parent)
{
    ui->pEditDescr->setReadOnly(true);
    ui->lEditPhone->setReadOnly(true);
    ui->lEditName->setReadOnly(true);
    ui->dateEdit->setReadOnly(true);

    using Btn = QDialogButtonBox::StandardButton;

    ui->buttonBox->setStandardButtons(Btn::Cancel);

    loadStatuses(db);
    clientToUi(cln);

    ui->cBoxStatus->setDisabled(true);
    loadRecords(db, cln.id);
}


// Create Client Dialog
ClientDialog::ClientDialog(const DatabaseController *db, QWidget *parent)
    : ClientDialog(parent)
{
    ui->lblRecords->hide();
    ui->tableRecords->hide();

    using Btn = QDialogButtonBox::StandardButton;

    setWindowTitle("Create Client");
    ui->dateEdit->setDate(QDate::currentDate());
    ui->buttonBox->setStandardButtons(Btn::Ok | Btn::Cancel);


    loadStatuses(db);
    ui->cBoxStatus->setCurrentIndex(mStats.size() - 1);
}

ClientDialog::~ClientDialog()
{
    delete ui;
}

Client ClientDialog::clientFromUi() const
{
    Client cl;
    cl.dateReg = ui->dateEdit->date();
    cl.name    = ui->lEditName->text();
    cl.phone   = ui->lEditPhone->text();
    cl.descr   = ui->pEditDescr->toPlainText();
    cl.stat_id = ui->cBoxStatus->currentIndex();

    if (cl.stat_id >= 0 && cl.stat_id < mStats.size()) {
        cl.stat_id = mStats.value(cl.stat_id).id;
    }

    return cl;
}

void ClientDialog::restoreTableHeader(const QByteArray &state)
{
    if (!state.isEmpty()) {
        ui->tableRecords->horizontalHeader()->restoreState(state);
    }
}

QByteArray ClientDialog::tableHeaderState() const
{
    return ui->tableRecords->horizontalHeader()->saveState();
}

int ClientDialog::statIndexById(int id) const
{
    for (int i = 0; i < mStats.size(); ++i) {
        if (mStats.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

void ClientDialog::loadStatuses(const DatabaseController *db)
{
    if (db != nullptr) {
        db->loadStatuses(mStats, "Client");

        for (auto &stat : qAsConst(mStats)) {
            ui->cBoxStatus->addItem(stat.name);
        }
    }
}

void ClientDialog::loadRecords(const DatabaseController *db, int clnId)
{
    QList<Rcrd> list;
    db->loadRecordsByClient(list, clnId);

    std::sort(list.begin(), list.end(), [](const Rcrd &r1, const Rcrd &r2) {
        if (r1.date == r2.date) {
            return r1.time > r2.time;
        }
        return r1.date > r2.date;
    });

    ui->tableRecords->setRowCount(list.size());

    int row = 0;
    while (list.size() > 0) {
        Rcrd unit = list.takeFirst();

        for (int col = 0; col < ui->tableRecords->columnCount(); ++col) {
            QString title;
            QColor color;

            switch (col) {
            case 0:
                title = unit.status;
                color = db->colorByRcrdId(unit.id[Rcrd::Id]);
                break;
            case 1:
                title = unit.date.toString(TableDateFormat);
                color = db->colorByRcrdId(unit.id[Rcrd::Id]);
                break;
            case 2:
                title = unit.time.toString(TimeFormat);
                color = db->colorByRcrdId(unit.id[Rcrd::Id]);
                break;
            case 3:
                title = unit.worker;
                color = db->colorByRcrdId(unit.id[Rcrd::Id]);
                break;
            case 4:
                title = unit.service;
                color = db->colorBySrvcId(unit.id[Rcrd::SrvId]);
                break;
            case 5:
                title = unit.client + ' '  + unit.phone;
                color = db->colorByClntId(unit.id[Rcrd::ClnId]);
                break;
            }

            auto *item = new QTableWidgetItem(title);
            item->setBackground(color);
            ui->tableRecords->setItem(row, col, item);
        }
        ++row;
    }

}

void ClientDialog::clientToUi(const Client &cln)
{
    ui->dateEdit->setDate(cln.dateReg);
    ui->lEditName->setText(cln.name);
    ui->lEditPhone->setText(cln.phone);
    ui->pEditDescr->setPlainText(cln.descr);
    ui->cBoxStatus->setCurrentIndex(statIndexById(cln.id));
}

void ClientDialog::onAccepted(QAbstractButton *button)
{
    using Btn = QDialogButtonBox::StandardButton;

    Btn btn = ui->buttonBox->standardButton(button);

    if (btn == Btn::Ok)
    {
        if (ui->lEditName->text().isEmpty()) {
            QMessageBox::warning(this, "Error Creating Client",
                                 "Please, enter the name for Creating Client!");
        }
        else {
            accept();
        }
    }

}
