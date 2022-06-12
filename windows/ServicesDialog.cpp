#include "ServicesDialog.h"
#include "ui_ServicesDialog.h"

#include "Consts.h"

#include <QSettings>
#include <QColorDialog>
#include <QCloseEvent>
#include <QMessageBox>

ServicesDialog::ServicesDialog(DatabaseController &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServicesDialog)
    , mDb(db)
{
    ui->setupUi(this);

    reset();

    connections();
    readSettigns();
}

ServicesDialog::~ServicesDialog()
{
    delete ui;
}

void ServicesDialog::onDialogButtons(QAbstractButton *bttn)
{
    if (bttn == nullptr) {
        return;
    }

    using Btn = QDialogButtonBox::StandardButton;

    switch (ui->buttonBox->standardButton(bttn))
    {
    case Btn::Save:
        save();
        break;

    case Btn::Discard:
        reset();
        break;

    default:
        return;
    }
}



void ServicesDialog::save()
{
    auto warnMsg = [&](const QString& procName)
    {
        return QMessageBox::warning(this, "Save Error",
                                QString("Error while %1!")
                                .arg(procName));
    };

    if (mCatgEdit[miCatg]) {
        if (!mDb.updateCategory(mCategories[miCatg])) {
            warnMsg("Update Category");
            return;
        }
        mCatgEdit[miCatg] = false;
    }

    for (int &id : mServToDelete) {
        if (!mDb.deleteService(id)) {
            warnMsg("Delete Service");
            return;
        }
    }

    for (int i = 0; i < mServices.size(); ++i) {
        auto &srv = mServices.at(i);

        if (srv.id == 0) {
            if (!mDb.addService(srv)) {
                warnMsg("Add Service");
                return;
            }
        }
        else if (mServEdit[i]) {
            if (!mDb.updateService(srv)) {
                warnMsg("Update Service");
                return;
            }
        }
        mServEdit[i] = false;
    }
}

void ServicesDialog::reset()
{
    miCatg = -1;
    loadCategories();
    onCatgChanged(0);
}



void ServicesDialog::onServRem()
{
    auto item = ui->tableServices->selectedItems().last();

    if (item == nullptr) {
        return;
    }

    mServToDelete << mServices.value(item->row()).id;

    mServEdit.removeAt(item->row());
    mServices.removeAt(item->row());
    ui->tableServices->removeRow(item->row());
}

void ServicesDialog::onCatgRem()
{
    using Btn = QMessageBox::StandardButton;

    if (mCategories.size() == 1) {
        QMessageBox::information(this, "Deletion Category",
                                 "You can't delete All Category");
        return;
    }

    Btn btn = QMessageBox::warning(this,
                QString("Deletion Category"),
                QString("\tAre you sure you want to Delete the '%1'?\n\n")
                .arg(ui->cBoxCatg->currentText())
                + "All Servicies in this Category will be deleted.\n"
                  "You will not be able to undo this action!",
                Btn::No | Btn::Yes,
                Btn::No );

    if (btn == Btn::No) {
        return;
    }



    if (!mDb.deleteCategory(mCategories[miCatg].id)) {
        QMessageBox::warning(this, "Save Error",
                            "Error while Delete Categroy!");
        return;
    }
    mCatgEdit.removeAt(miCatg);
    mCategories.removeAt(miCatg);

    miCatg = -1;
    ui->cBoxCatg->removeItem(ui->cBoxCatg->currentIndex());
    // onCatgChanged() will be emitted;
}



void ServicesDialog::onServAdd()
{
    Service serv;
    serv.name = "New";
    serv.categ_id = mCategories.at(miCatg).id;
    if (!mServices.isEmpty()) {
        int index = 0;
        {
            QString name = serv.name;
            while (mDb.servByNameCatg(name, serv.categ_id) > 0) {
                name = serv.name + QString::number(index++);
            }
            serv.name = name;
        }
    }
    else {
        serv.name = InitServName;
    }
    int row = mServices.size();

    mServices << serv;
    mServEdit << true;
    ui->tableServices->setRowCount(mServices.size());

    setServUi(serv, row);
    ui->tableServices->item(row, 1)->setSelected(true);
}

void ServicesDialog::onCatgAdd()
{
    Category catg;
    catg.name = "New";
    catg.rgb  = QColor(Qt::white);

    int index = 0;
    {
        QString name = catg.name;
        while (mDb.categoryIdByName(name) > 0) {
            name = catg.name + QString::number(index++);
        }
        catg.name = name;
    }
    if (!mDb.addCategory(catg)) {
        QApplication::beep();
        return;
    }

    catg.id = mDb.categoryIdByName(catg.name);
    if (catg.id == -1) {
        QApplication::beep();
        return;
    }
    index = mCategories.size();

    mCategories << catg;
    mCatgEdit << false;

    ui->cBoxCatg->addItem(catg.name);
    ui->cBoxCatg->setCurrentIndex(index);

    onServAdd();
    save();
}

void ServicesDialog::onCatgChanged(int index)
{
    if (index < 0 || index >= mCategories.size()) {
        return;
    }
    if (index == miCatg) {
        return;
    }
    if (miCatg != -1) {
        checkEdits();
    }

    mServEdit.clear();
    mServices.clear();
    mServToDelete.clear();
    ui->tableServices->clearContents();

    auto ctg = mCategories.at(index);
    setColorUi(ctg.rgb);

    mDb.loadServicesByCateg(mServices, ctg.id);
    ui->tableServices->setRowCount(mServices.size());

    int row = 0;
    for (auto &srv : mServices) {
        setServUi(srv, row);
        mServEdit << false;
        row++;
    }

    miCatg = index;
}

void ServicesDialog::onServEdited(QTableWidgetItem *item)
{
    if (item != nullptr && !mIsCreate) {
        auto &serv = mServices[item->row()];

        switch (item->column()) {
        case 1:
            if (item->text() == serv.name) {
                return;
            }
            else if (item->text() == InitServName) {
                // Just catch error;
            }
            else if ( item->text().toLower()
                != InitServName.toLower() )
            {
                bool check = true;
                for (auto &serv : mServices) {
                    if (serv.name == item->text()) {
                        check = false;
                        break;
                    }
                }

                if (check) {
                    serv.name = item->text();
                    mServEdit[item->row()] = true;
                    return;
                }
            }
            QApplication::beep();
            item->setText(serv.name);
            break;

        case 0: {
            bool ok = false;
            float value = item->text().toFloat(&ok);

            if (ok) {
                serv.price = value;
                mServEdit[item->row()] = true;
            }
            else {
                QApplication::beep();
                item->setText(QString::number(serv.price));
            }
        } break;
        }
    }
}

void ServicesDialog::onCatgNameEdited(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }

    if (ui->cBoxCatg->currentIndex() == miCatg) {
        auto &ctg = mCategories[miCatg];

        if (ctg.name != name) {
            mCatgEdit[miCatg] = true;
            ctg.name = name;
            ui->cBoxCatg->setItemText(miCatg, name);
        }
    }
}

void ServicesDialog::checkEdits()
{
    auto saveMsg = [&](const QString &name) -> bool
    {
        using Btn = QMessageBox::StandardButton;
        Btn btn = QMessageBox::warning(this,
                    QString("Unsaved %1").arg(name),
                    "Do you want save?",
                    Btn::Yes | Btn::No);

        if (btn == Btn::Yes) {
            save();
            return true;
        }
        return false;
    };

    if (mCatgEdit[miCatg]) {
        auto &catg = mCategories[miCatg];

        if (!saveMsg(catg.name)) {
            catg = mDb.category(catg.id);
            ui->cBoxCatg->setItemText(miCatg, catg.name);
            mCatgEdit[miCatg] = false;
        }
    }
    else if (mServToDelete.size() > 0) {
        saveMsg("Services");
    }
    else {
        for (bool &edit : mServEdit)  {
            if (edit) {
                saveMsg("Services");
                break;
            }
        }
    }
}

void ServicesDialog::setServUi(const Service &serv, int row)
{
    mIsCreate = true;
    ui->tableServices->setItem(row, 0, new QTableWidgetItem(
                                   QString::number(serv.price) ));
    ui->tableServices->setItem(row, 1, new QTableWidgetItem(serv.name));    
    mIsCreate = false;
}

void ServicesDialog::setColorUi(const QColor &color)
{
    QPalette palette = ui->lEditColor->palette();
    palette.setColor(QPalette::Base, color);
    ui->lEditColor->setPalette(palette);

    ui->lblRgb->setText(color.name(QColor::HexRgb).toLower());
}

void ServicesDialog::onSetColor()
{
    auto &catg = mCategories[miCatg];
    auto color = QColorDialog::getColor(catg.rgb, this,
                                QString("Choose color for %1")
                                .arg(catg.name));

    if (color.isValid()) {
        setColorUi(color);
        catg.rgb = color;
        mCatgEdit[miCatg] = true;
    }
}

void ServicesDialog::loadCategories()
{    
    mCatgEdit.clear();
    mDb.loadCategories(mCategories);
    ui->cBoxCatg->clear();

    int row = 0;
    for (auto &catg: mCategories)
    {
        ui->cBoxCatg->addItem(catg.name);
        mCatgEdit << false;
        ++row;
    }
}

void ServicesDialog::connections()
{
    connect(ui->cBoxCatg, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ServicesDialog::onCatgChanged);

    connect(ui->cBoxCatg, &QComboBox::editTextChanged,
            this, &ServicesDialog::onCatgNameEdited);

    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &ServicesDialog::onDialogButtons);



    connect(ui->pBtnAddServ, &QPushButton::clicked,
            this, &ServicesDialog::onServAdd);

    connect(ui->pBtnRemServ, &QPushButton::clicked,
            this, &ServicesDialog::onServRem);

    connect(ui->pBtnAddCatg, &QPushButton::clicked,
            this, &ServicesDialog::onCatgAdd);

    connect(ui->pBtnRemCatg, &QPushButton::clicked,
            this, &ServicesDialog::onCatgRem);

    connect(ui->pBtnColor, &QPushButton::clicked,
            this, &ServicesDialog::onSetColor);

    connect(ui->tableServices, &QTableWidget::itemChanged,
            this, &ServicesDialog::onServEdited);
}

void ServicesDialog::readSettigns()
{

}

void ServicesDialog::writeSettigns()
{

}


void ServicesDialog::closeEvent(QCloseEvent *event)
{
    writeSettigns();
    event->accept();
}


