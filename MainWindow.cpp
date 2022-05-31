#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QSettings>

#include "CrmConsts.h"

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
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

