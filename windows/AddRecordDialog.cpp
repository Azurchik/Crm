#include "AddRecordDialog.h"
#include "ui_AddRecordDialog.h"

#include "ClientDialog.h"
#include "Consts.h"

#include <QMessageBox>
#include <QSettings>
#include <QDebug>

AddRecordDialog::AddRecordDialog(DatabaseController &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddRecordDialog)
    , mDb(db)
{
    ui->setupUi(this);

    ui->dateEdit->setDate(QDate::currentDate());
    ui->timeEdit->setTime(QTime::currentTime());

    mDb.loadCategories(mSrvCategs);
    for (auto &ctg: qAsConst(mSrvCategs)) {
        ui->cBoxCateg->addItem(ctg.name);
    }
    loadServicies();

    mDb.loadWorkers(mWorkers);
    for (auto &wrk: qAsConst(mWorkers)) {
        ui->cBoxWorker->addItem(wrk.name);
    }

    mDb.loadStatuses(mStatuses, "Rcrd");
    for (auto &stat: qAsConst(mStatuses)) {
        ui->cBoxStatus->addItem(stat.name);
    }    

    loadPreview();
    connections();

    readSettings();
}

AddRecordDialog::~AddRecordDialog()
{
    delete ui;
}

void AddRecordDialog::setDate(const QDate &date)
{
    QDate nDate = QDate::currentDate();
    nDate.setDate(date.year(), date.month(), nDate.day());

    ui->dateEdit->setDate(nDate);
}

Rcrd AddRecordDialog::getRecord() const
{
    Rcrd res;
    res.date = ui->dateEdit->date();
    res.time = ui->timeEdit->time();

    int index = ui->cBoxServ->currentIndex();
    if (ui->gBoxService->isChecked() && index != -1) {
        res.id[Rcrd::SrvId]  = mServices.at(index).id;
    }

    index = ui->tableClients->currentRow();
    if (index >= 0 && index < mClients.size())
    {
        res.id[Rcrd::ClnId]  = mClients.at(index).id;
    }

    index = ui->cBoxWorker->currentIndex();
    if (index != -1) {
        res.id[Rcrd::WorkId] = mWorkers.at(index).id;
    }

    index = ui->cBoxStatus->currentIndex();
    if (index != -1) {
        res.id[Rcrd::StatId] = mStatuses.at(index).id;
    }

    return res;
}

void AddRecordDialog::openClientWindow(QTableWidgetItem *item)
{
    if (item != nullptr) {
        int row = item->row();

        if (row != -1) {
            auto clnDialog = new ClientDialog(&mDb, mClients.at(row), this);
            clnDialog->setModal(false);
            clnDialog->open();
        }
    }
}

void AddRecordDialog::handleSearchType()
{
    switch(ui->cBoxSearch->currentIndex()) {
    case 1:
        ui->lblSearch->setText("Client Phone:");
        ui->lEditSearch->setInputMask("(\\0DD)-999-9999");
        break;
    case 0:
        ui->lblSearch->setText("Client Name:");
        ui->lEditSearch->setInputMask("");
        break;
    }
    ui->lEditSearch->setFocus(Qt::MouseFocusReason);
}

void AddRecordDialog::loadPreview()
{
    auto &list = ui->listPreview;

    list->item(0)->setText(ui->dateEdit->date().toString(DateFormat));
    list->item(1)->setText(ui->timeEdit->time().toString(TimeFormat));
    list->item(2)->setText(ui->cBoxWorker->currentText());

    if (ui->gBoxService->isChecked()) {
        list->item(3)->setText(ui->cBoxCateg->currentText()
                               + ": " +
                               ui->cBoxServ->currentText());
    }
    else {
        list->item(3)->setText("No Service");
    }

    list->item(5)->setText(ui->cBoxStatus->currentText());

    auto row = ui->tableClients->currentRow();
    if (row != -1) {
        list->item(4)->setText(mClients.at(row).name);
    }
    else {
        list->item(4)->setText("No Client");
    }
}

void AddRecordDialog::search()
{
    mClients.clear();
    ui->tableClients->clearContents();

    switch(ui->cBoxSearch->currentIndex()) {
    case 0:
        mDb.searchClientByName(ui->lEditSearch->text(), mClients);
        break;
    case 1:
        mDb.searchClientByPhone(ui->lEditSearch->text(), mClients, true);
        break;
    }

    ui->tableClients->setRowCount(mClients.size());

    for (int row = 0; row < mClients.size(); ++row) {
        addClientToTable(mClients.at(row), row);
    }    
}

void AddRecordDialog::createClient()
{
    auto clnDialog = new ClientDialog (&mDb, this);
    if (clnDialog->exec() == QDialog::Accepted)
    {
        auto cln = clnDialog->clientFromUi();
        if (mDb.addClient(cln))
        {
            ui->lEditSearch->clear();

            switch (ui->cBoxSearch->currentIndex()) {
            case 0:
                ui->lEditSearch->setText(cln.name);
                break;
            case 1:
                ui->lEditSearch->setText(cln.phone);
                break;
            }
            search();

            if (mClients.size() == 1) {
                ui->tableClients->selectRow(0);
            }
        }
    }
}

void AddRecordDialog::connections()
{
    connect(ui->cBoxCateg, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddRecordDialog::loadServicies);

    connect(ui->pBtnSearch, &QPushButton::clicked,
            this, &AddRecordDialog::search);

    connect(ui->pBtnAdd, &QPushButton::clicked,
            this, &AddRecordDialog::createClient);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &AddRecordDialog::onAccepted);

    connect(ui->cBoxSearch, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddRecordDialog::handleSearchType);

    connect(ui->tableClients, &QTableWidget::itemDoubleClicked,
            this, &AddRecordDialog::openClientWindow);

// Preview:
    connect(ui->dateEdit, &QDateEdit::dateChanged,
        this, &AddRecordDialog::loadPreview);
    connect(ui->timeEdit, &QTimeEdit::timeChanged,
        this, &AddRecordDialog::loadPreview);
    connect(ui->gBoxService, &QGroupBox::toggled,
        this, &AddRecordDialog::loadPreview);
    connect(ui->cBoxCateg, &QComboBox::currentTextChanged,
        this, &AddRecordDialog::loadPreview);
    connect(ui->cBoxServ, &QComboBox::currentTextChanged,
        this, &AddRecordDialog::loadPreview);
    connect(ui->cBoxStatus, &QComboBox::currentTextChanged,
        this, &AddRecordDialog::loadPreview);
    connect(ui->cBoxWorker, &QComboBox::currentTextChanged,
        this, &AddRecordDialog::loadPreview);
    connect(ui->tableClients, &QTableWidget::currentItemChanged,
            this, &AddRecordDialog::loadPreview);
}

void AddRecordDialog::closeEvent(QCloseEvent *event)
{
    writeSettings();
}


void AddRecordDialog::loadServicies()
{
    ui->cBoxServ->clear();
    mServices.clear();

    auto &ctg = mSrvCategs.at(ui->cBoxCateg->currentIndex());
    mDb.loadServicesByCateg(mServices, ctg.id);

    for (const auto &srv: qAsConst(mServices)) {
        ui->cBoxServ->addItem(srv.name);
    }
}

void AddRecordDialog::onAccepted()
{
    if ( ui->tableClients->currentRow() < 0
        || ui->tableClients->currentRow() >= mClients.size() )
    {
        QMessageBox::warning(this, "Adding Error",
                             "Please, select Client to create the Record");
        return;
    }
    accept();
}

void AddRecordDialog::readSettings()
{
    QSettings stt (IniFName, QSettings::IniFormat);
    stt.beginGroup(objectName());

    if (stt.contains("size")) {
        resize(stt.value("size").toSize());
    }
    if (stt.contains("splitter")) {
        ui->splitter->restoreState(stt.value("splitter").toByteArray());
    }
    if (stt.contains("tableHeader")) {
        ui->tableClients->horizontalHeader()->restoreState(stt.value("tableHeader").toByteArray());
    }

    ui->cBoxSearch->setCurrentIndex(stt.value("searchIndex", 0).toInt());

    stt.endGroup();
}

void AddRecordDialog::writeSettings()
{
    QSettings stt (IniFName, QSettings::IniFormat);
    stt.beginGroup(objectName());

    stt.setValue("size", size());
    stt.setValue("splitter", ui->splitter->saveState());
    stt.setValue("searchIndex", ui->cBoxSearch->currentIndex());
    stt.setValue("tableHeader", ui->tableClients->horizontalHeader()->saveState());

    stt.endGroup();
}

void AddRecordDialog::addClientToTable(const Client &cln, int row)
{
    ui->tableClients->setItem(row, 1, new QTableWidgetItem(cln.name));
    ui->tableClients->setItem(row, 2, new QTableWidgetItem(cln.phone));

    auto status = mDb.getStatus(cln.stat_id);

    QTableWidgetItem *item = new QTableWidgetItem(status.name);
    item->setBackground(status.rgb);
    ui->tableClients->setItem(row, 0, item);
}
