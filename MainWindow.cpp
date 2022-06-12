#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QSettings>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QSqlRelationalTableModel>

#include "Consts.h"
#include "windows/AddRecordDialog.h"
#include "windows/ServicesDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mTModel(new CrmTableModel(QDate::currentDate(), mDb, this))
{
    ui->setupUi(this);
    ui->dateEdit->setDate(QDate::currentDate());
    ui->tableView->setModel(mTModel);

    connections();
    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addRecord()
{
    auto window = new AddRecordDialog(mDb, this);
    window->setDate(ui->dateEdit->date());

    if (window->exec() == QDialog::Accepted)
    {
        if (mDb.addRecord( window->getRecord() )) {
            mTModel->resetTable();
            updateStatusBar();
        }
    }
}

void MainWindow::removeRecord()
{
    int row = ui->tableView->currentIndex().row();
    if (row == -1) {
        return;
    }

    auto res = QMessageBox::information(this, "Delition Record",
                             "Are you sure you want to delete this Record?",
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No  );

    if ( res == QMessageBox::Yes
        && mDb.deleteRecord(mTModel->recordIdByRow(row)) )
    {
        mTModel->resetTable();
        updateStatusBar();
    }
}

void MainWindow::updateStatusBar()
{
    ui->statusbar->showMessage(mTModel->getStatusesInfo());
}

void MainWindow::actionsDatabase()
{
    auto dialog = ServicesDialog(mDb, this);
    dialog.exec();
}

void MainWindow::delay(int msecs)
{
    QTime dieTime = QTime::currentTime().addMSecs(msecs);

    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void MainWindow::readSettings()
{
    QSettings stt {IniFName, QSettings::IniFormat};

    stt.beginGroup(objectName());
    if (stt.contains("size")) {
        resize(stt.value("size").toSize());
    }
    if (stt.contains("pos")) {
        move(stt.value("pos").toPoint());
    }
    if (stt.value("isMax", false).toBool()) {
        showMaximized();
    }
    if (stt.contains("splitHead")) {
        ui->splitterHeader->restoreState(
                    stt.value("splitHead").toByteArray() );
    }
    if (stt.contains("tableHead")) {
        ui->tableView->horizontalHeader()->restoreState(
                    stt.value("tableHead").toByteArray() );
    }
    ui->dateEdit->setDate(
                stt.value("date", QDate::currentDate()).toDate() );
    stt.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings stt {IniFName, QSettings::IniFormat};

    stt.beginGroup(objectName());
    stt.setValue("isMax", isMaximized());
    if (isMaximized()) {        
        showNormal();
        delay(123);
    }
    stt.setValue("pos", pos());
    stt.setValue("size", size());

    stt.setValue("date", ui->dateEdit->date());

    stt.setValue("splitHead", ui->splitterHeader->saveState());
    stt.setValue("tableHead", ui->tableView->horizontalHeader()
                                           ->saveState() );

    stt.endGroup();
}

void MainWindow::connections()
{
    connect(ui->dateEdit, &QDateEdit::dateChanged,
            mTModel, &CrmTableModel::dateChanged);

    connect(ui->dateEdit, &QDateEdit::dateChanged,
            this, &MainWindow::updateStatusBar);

    connect(ui->pBtn_AddRcrd, &QPushButton::clicked,
            this, &MainWindow::addRecord);

    connect(ui->pBtn_RemRecord, &QPushButton::clicked,
            this, &MainWindow::removeRecord);

// Tab Actions:
    connect(ui->actionServices, &QAction::triggered,
            this, &MainWindow::actionsDatabase);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

