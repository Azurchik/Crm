#include "ClientDialog.h"
#include "ui_ClientDialog.h"

#include <QMessageBox>

#include "Consts.h"


ClientDialog::ClientDialog(const DatabaseController &db, const Client &cln,
                           QWidget *parent, bool isRead)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
    , mClient(cln)
{
    ui->setupUi(this);

    ui->pEditDescr->setReadOnly(isRead);
    ui->lEditPhone->setReadOnly(isRead);
    ui->lEditName->setReadOnly(isRead);
    ui->dateEdit->setReadOnly(isRead);
    ui->cBoxStatus->setDisabled(isRead);

    using Btn = QDialogButtonBox::StandardButton;

    if (isRead) {
        ui->lblStatus->setText("Status: " + db.getStatus(cln.stat_id).name);
        ui->buttonBox->setStandardButtons(Btn::Cancel);
    }
    else {
        loadStatuses(db);
    }
    clientToUi(cln);

    loadRecords(db, cln.id);

    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &ClientDialog::onButtonsClicked);
}


// Create Client Dialog
ClientDialog::ClientDialog(const DatabaseController &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
{
    ui->setupUi(this);

    ui->lblRecords->hide();
    ui->tableRecords->hide();

    using Btn = QDialogButtonBox::StandardButton;

    setWindowTitle("Create Client");
    ui->dateEdit->setDate(QDate::currentDate());
    ui->buttonBox->setStandardButtons(Btn::Ok | Btn::Cancel);

    loadStatuses(db);
    ui->cBoxStatus->setCurrentIndex(mStats.size() - 1);

    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &ClientDialog::onButtonsClicked);
}

ClientDialog::~ClientDialog()
{
    delete ui;
}

Client ClientDialog::clientFromUi() const
{
    Client cl;
    cl.id = mClient.id;

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

void ClientDialog::loadStatuses(const DatabaseController &db)
{
    db.loadStatuses(mStats, "Client");

    for (auto &stat : qAsConst(mStats)) {
        ui->cBoxStatus->addItem(stat.name);
    }
}

void ClientDialog::loadRecords(const DatabaseController &db, int clnId)
{
    QList<Record> list;
    db.loadRecordsByClient(list, clnId);

    std::sort(list.begin(), list.end(), [](const Record &r1, const Record &r2) {
        if (r1.date == r2.date) {
            return r1.time > r2.time;
        }
        return r1.date > r2.date;
    });

    ui->tableRecords->setRowCount(list.size());

    int row = 0;
    while (list.size() > 0) {
        Record unit = list.takeFirst();

        for (int col = 0; col < ui->tableRecords->columnCount(); ++col) {
            QString title;
            QColor color;

            switch (col) {
            case 0:
                title = unit.status;
                color = db.colorByRcrdId(unit.id[Record::Id]);
                break;
            case 1:
                title = unit.date.toString(TableDateFormat);
                color = db.colorByRcrdId(unit.id[Record::Id]);
                break;
            case 2:
                title = unit.time.toString(TimeFormat);
                color = db.colorByRcrdId(unit.id[Record::Id]);
                break;
            case 3:
                title = unit.worker;
                color = db.colorByRcrdId(unit.id[Record::Id]);
                break;
            case 4:
                title = unit.service;
                color = db.colorBySrvcId(unit.id[Record::SrvId]);
                break;
            case 5:
                title = unit.client + ' '  + unit.phone;
                color = db.colorByClntId(unit.id[Record::ClnId]);
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

void ClientDialog::onButtonsClicked(QAbstractButton *button)
{
    using Btn = QDialogButtonBox::StandardButton;

    switch (ui->buttonBox->standardButton(button)) {
    case Btn::Ok:
    case Btn::SaveAll:
        if (ui->lEditName->text().isEmpty()) {
            QMessageBox::warning(this, "Error Client",
                                 "Please, enter the name for new Client!");
        }
        else {
            accept();
        }
        break;

    case Btn::Reset:
        clientToUi(mClient);
        break;

    default:
        return;
    }

}
