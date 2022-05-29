#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mTModel(new CrmTableModel(QDate::currentDate(), this))
{
    ui->setupUi(this);
    ui->tableView->setModel(mTModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

