#include "RecordDialog.h"
#include "ui_RecordDialog.h"

#include "Consts.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>

RecordDialog::RecordDialog(DatabaseController &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RecordDialog)
    , mDb(db)
{
    ui->setupUi(this);

    mDb.loadCategories(mCategories);
    for (auto &ctg: qAsConst(mCategories)) {
        ui->cBoxCateg->addItem(ctg.name);
    }
    loadServices();

    mDb.loadWorkers(mWorkers);
    for (auto &wrk: qAsConst(mWorkers)) {
        ui->cBoxWorker->addItem(wrk.name);
    }

    mDb.loadStatuses(mStatuses, "Rcrd");
    for (auto &stat: qAsConst(mStatuses)) {
        ui->cBoxStatus->addItem(stat.name);
    }

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &RecordDialog::onUpdate);

    connect(ui->cBoxCateg, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RecordDialog::loadServices);


    readSettings();
}

RecordDialog::~RecordDialog()
{
    delete ui;
}

void RecordDialog::initRecord(int id)
{
    mRec = mDb.getRecord(id);

    if (mRec.id[0] == 0) {
        ui->lEditCln->setText( " - - ");
        setWindowTitle("WRONG RECORD!");
        return;
    }

    ui->lEditCln->setText(mRec.client + " "
                        + mRec.phone );

    ui->timeEdit->setTime(mRec.time);
    ui->dateEdit->setDate(mRec.date);

    if (mRec.id[Record::StatId] != 0) {
        ui->gBoxService->setChecked(true);
        Service _srv;

        int sIndex = 0;
        for (auto &serv : mServices) {
            if (serv.id == mRec.id[Record::StatId]) {
                _srv = serv;
                break;
            }
            ++sIndex;
        }

        int index = 0;
        for (auto &catg : mCategories) {
            if (catg.id == _srv.categ_id) {
                ui->cBoxCateg->setCurrentIndex(index);
                break;
            }
            ++index;
        }
        ui->cBoxServ->setCurrentIndex(sIndex);
    }

    int index = 0;
    for (auto &stat : mStatuses) {
        if (stat.id == mRec.id[Record::StatId]) {
            ui->cBoxStatus->setCurrentIndex(index);
        }
        ++index;
    }

    index = 0;
    for (auto &stat : mWorkers) {
        if (stat.id == mRec.id[Record::WorkId]) {
            ui->cBoxStatus->setCurrentIndex(index);
        }
        ++index;
    }

}

void RecordDialog::onUpdate()
{
    mRec.id[Record::SrvId]  = mServices[ui->cBoxServ->currentIndex()].id;
    mRec.id[Record::StatId] = mStatuses[ui->cBoxStatus->currentIndex()].id;
    mRec.id[Record::WorkId] = mWorkers[ui->cBoxWorker->currentIndex()].id;

    if (!mDb.updateRecord(mRec))  {
        QMessageBox::warning(this, "Db Error",
                             "Can't update Record!");
    }
    else {
        accept();
    }
}

void RecordDialog::loadServices()
{
    ui->cBoxServ->clear();
    mServices.clear();

    auto &ctg = mCategories.at(ui->cBoxCateg->currentIndex());
    mDb.loadServicesByCateg(mServices, ctg.id);

    QPalette palette = ui->lEditColor->palette();
    palette.setColor(QPalette::Base, ctg.rgb);
    ui->lEditColor->setPalette(palette);

    for (const auto &srv: qAsConst(mServices)) {
        ui->cBoxServ->addItem(srv.name);
    }
}

void RecordDialog::readSettings()
{
    QSettings stt (IniFName, QSettings::IniFormat);
    stt.beginGroup(objectName());

    if (stt.contains("size")) {
        resize(stt.value("size").toSize());
    }
    stt.endGroup();
}

void RecordDialog::writeSettings()
{
    QSettings stt (IniFName, QSettings::IniFormat);
    stt.beginGroup(objectName());

    if (stt.contains("size")) {
        resize(stt.value("size").toSize());
    }
    stt.endGroup();
}


void RecordDialog::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}
