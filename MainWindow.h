#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "DatabaseController.h"
#include "CrmTableModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void delay(int msecs);

    void readSettings();
    void writeSettings();

    void connections();

private:
    Ui::MainWindow *ui;
    DatabaseController mDb;
    CrmTableModel *mTModel;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
