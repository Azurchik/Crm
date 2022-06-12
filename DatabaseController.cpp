#include "DatabaseController.h"

#include <QDir>
#include <QDate>
#include <QDebug>
#include <QFileInfo>
#include <QApplication>

#include <QSqlError>
#include <QSqlQuery>
#include <QColor>

#include "Consts.h"

DatabaseController::DatabaseController()
    : mDatabase(QSqlDatabase::addDatabase("QSQLITE"))
{
    mDatabase.setDatabaseName(dbFilePath());

    if (!QFileInfo::exists(mDatabase.databaseName())) {
        createDatabase();
        initTables();
    }
    else {
        mDatabase.open();
    }
}

Status DatabaseController::getStatus(int id) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT * FROM status "
                  "WHERE id = :id");
    query.bindValue(":id", id);

    if (!execQuery(query)) {
        return {};
    }

    return {query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            QColor(query.value(3).toString())
    };
}

int DatabaseController::servByNameCatg(const QString &name,
                                             int catg_id) const
{
    QSqlQuery query(mDatabase);
    QString prep("SELECT id FROM Service "
                 "WHERE name LIKE "
                 "\'" + name + "\' "
                 "AND catg_id = :id");
    query.prepare(prep);
    query.bindValue(":id", catg_id);

    if (!execQuery(query)) {
        qWarning() << "Error while getting id "
                      "by name. Service!\n"
                   << query.lastError().text()
                   << query.lastQuery();
        return -1;
    }

    if (!query.isValid()) {
        return 0;
    }
    else {
        return query.value(0).toInt();
    }
}

int DatabaseController::categoryIdByName(const QString &name) const
{
    QSqlQuery query(mDatabase);
    QString prep("SELECT id FROM Srv_Category "
                 "WHERE name "
                 "LIKE \'" + name + "\'");
    query.prepare(prep);

    if (!execQuery(query)) {
        qWarning() << "Error while getting id "
                      "by name. Category!\n"
                   << query.lastError().text()
                   << query.lastQuery();
        return -1;
    }

    if (!query.isValid()) {
        return 0;
    }
    else {
        return query.value(0).toInt();
    }
}

bool DatabaseController::updateService(const Service &srv)
{
    QSqlQuery query(mDatabase);
    query.prepare("UPDATE Service SET"
                  "     name  = :name,"
                  "     price = :price "
                  "WHERE"
                  "     id = :id");

    query.bindValue(":id"   , srv.id);
    query.bindValue(":name" , srv.name);
    query.bindValue(":price", srv.price);

    if (!execQuery(query)) {
        qWarning("Error while update Service by id: %i!", srv.id);
        return false;
    }

    return true;
}

bool DatabaseController::updateCategory(const Category &ctg)
{
    QSqlQuery query(mDatabase);
    query.prepare("UPDATE Srv_Category SET"
                  "     name = :name,"
                  "     rgb  = :rgb "
                  "WHERE "
                  "     id = :id");

    query.bindValue(":id"   , ctg.id);
    query.bindValue(":name" , ctg.name);
    query.bindValue(":rgb" , ctg.rgb);

    if (!execQuery(query)) {
        qWarning("Error while update Category by id: %i!", ctg.id);
        return false;
    }

    return true;
}

bool DatabaseController::deleteRecord(int id)
{
    QSqlQuery query(mDatabase);
    query.prepare("DELETE FROM Rcrd "
                  "WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning("Error while removing Record!"
                 "\n\tId: %i\n\tError: %s",
                 id, qPrintable(query.lastError().text())
                 );
        return false;
    }

    return true;
}

bool DatabaseController::deleteService(int id)
{
    QSqlQuery query(mDatabase);
    query.prepare("DELETE FROM Service "
                  "WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning("Error while removing Service!"
                 "\n\tId: %i\n\tError: %s",
                 id, qPrintable(query.lastError().text())
                 );
        return false;
    }

    return true;
}

bool DatabaseController::deleteCategory(int id)
{
    QSqlQuery query(mDatabase);
    query.prepare("DELETE FROM Srv_Category "
                  "WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning("Error while removing Category!"
                 "\n\tId: %i\n\tError: %s",
                 id, qPrintable(query.lastError().text())
                 );
        return false;
    }

    return true;
}

bool DatabaseController::addRecord(const Rcrd &rcd)
{
    QSqlQuery query(mDatabase);
    QString preparation = "INSERT INTO Rcrd (cln_id, work_id, srv_id, "
                          "stat_id, time, date) VALUES ";
    preparation += QString("(%1, %2, %3, %4, '%5', '%6')")
            .arg(rcd.id[Rcrd::ClnId]).arg(rcd.id[Rcrd::WorkId])
            .arg(rcd.id[Rcrd::SrvId]).arg(rcd.id[Rcrd::StatId])
            .arg(rcd.time.toString(TimeFormat), rcd.date.toString(DateFormat));


    query.prepare(preparation);

    if (!execQuery(query)) {
        qWarning() << "Error while adding new Record!";
        return false;
    }
    return true;
}

bool DatabaseController::addClient(const Client &cln)
{
    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO Client (name, phone, descr, "
                  "stat_id, dateReg) VALUES "
                  "(:name, :phone, :descr, :stat, :date);");

    query.bindValue(":name" , cln.name);
    query.bindValue(":phone", cln.phone);
    query.bindValue(":descr", cln.descr);
    query.bindValue(":stat" , cln.stat_id);
    query.bindValue(":date" , cln.dateReg.toString(DateFormat));

    if (!execQuery(query)) {
        qWarning() << "Error while adding new Client!";
        return false;
    }

    return true;
}

bool DatabaseController::addService(const Service &srv)
{
    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO Service (name, catg_id, price, dateReg) "
                  "VALUES (:name, :catg, :price, :date)");

    query.bindValue(":name" , srv.name);
    query.bindValue(":catg" , srv.categ_id);
    query.bindValue(":date" , srv.dateReg);
    query.bindValue(":price", srv.price);

    if (!execQuery(query)) {
        qWarning() << "Error while adding new Service!";
        return false;
    }

    return true;
}

bool DatabaseController::addCategory(const Category &ctg)
{
    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO Srv_Category (name, rgb) "
                  "VALUES (:name, :rgb)");

    query.bindValue(":name" , ctg.name);
    query.bindValue(":phone", ctg.rgb.name(QColor::HexRgb));

    if (!execQuery(query)) {
        qWarning() << "Error while adding new Category!";
        return false;
    }

    return true;
}

int DatabaseController::countWorkers() const
{
    int res = 0;
    QSqlQuery query(mDatabase);
    query.prepare("SELECT COUNT(id) FROM Worker");

    if (execQuery(query)) {
        res = query.value(0).toInt();
    }

     return res;
}

void DatabaseController::initRecordList(QList<Rcrd> &list, int month, int year) const
{
    QSqlQuery query(mDatabase);
    QString preparation = QString(
                "SELECT Rcrd.*, Client.name, "
                "    Client.phone, Worker.name, "
                "    Service.name, Status.name "
                "FROM Rcrd "
                "LEFT JOIN Client ON"
                "     Client.id = Rcrd.cln_id "
                "LEFT JOIN Service ON"
                "     Service.id = Rcrd.srv_id "
                "LEFT JOIN Worker ON"
                "     Worker.id = Rcrd.work_id "
                "LEFT JOIN Status ON"
                "     Status.id = Rcrd.stat_id ");
    QString values("WHERE date LIKE '__.%1.%2'");

    if (month < 10) {
        auto tMonth = '0' + QString::number(month);
        preparation += values.arg(tMonth).arg(year % 100);
    }
    else {
        preparation += values.arg(month).arg(year % 100);
    }

    query.prepare(preparation);
    if (!execQuery(query)) {
        qWarning("Error while getting records. Date: %i.%i", month, year);
        return;
    }

    list.clear();

    if (!query.isValid()) {
        qDebug("There are no records by the date: %i.%i", month, year);
        return;
    }


    do {
        list.push_back({});
        auto &item = list.last();

        for (int i = 0; i < 5; ++i) {
            item.id[i] = query.value(i).toInt();
        }

        item.time = QTime::fromString(query.value(5).toString(), TimeFormat);
        item.date = QDate::fromString(query.value(6).toString(), DateFormat);
        item.client  = query.value(7).toString();
        item.phone   = query.value(8).toString();
        item.worker  = query.value(9).toString();
        item.service = query.value(10).toString();
        item.status  = query.value(11).toString();
    }
    while (query.next());
}

void DatabaseController::searchClientByName(const QString &name, QList<Client> &result) const
{
    if (name.isEmpty()) {
        return;
    }
    QSqlQuery query(mDatabase);  
    QString prep = QString(
                "SELECT * FROM Client "
                "WHERE name LIKE '%%1%' "
                "ORDER BY stat_id")
            .arg(name);
    query.prepare(prep);

    if (!execQuery(query)) {
        qWarning() << "Error while searching Client by Name:" << name;
        return;
    }

    if (query.isValid()) {
        do {
            Client cln;
            cln.id      = query.value(0).toInt();
            cln.name    = query.value(1).toString();
            cln.phone   = query.value(2).toString();
            cln.descr   = query.value(3).toString();
            cln.stat_id = query.value(4).toInt();
            cln.dateReg = QDate::fromString(query.value(5).toString(), DateFormat);

            result << cln;
        }
        while (query.next());
    }
}

void DatabaseController::searchClientByPhone(const QString &phone, QList<Client> &result,
                                             bool wasStrongPhoneValidation) const
{
    if (phone.isEmpty()) {
        return;
    }
    QSqlQuery query(mDatabase);
    // '\%%1\%'
    QString prep = QString(
                "SELECT * FROM Client "
                "WHERE phone LIKE %1 "
                "ORDER BY stat_id");

    QString tmp;

    if (wasStrongPhoneValidation)
    {
        auto parts = phone.splitRef("-");
        tmp += parts.first();

        if (tmp.size() != 5) {
            int i = tmp.size();

            tmp.insert(i - 1, '%'); // == (07%) || (0%)

            if (tmp.size() == 5) {  // == (07%)
                i = tmp.indexOf('0') + 1;
                tmp.insert(i, '%');
            }
        }
        tmp += '-';

        if (parts[1].size() == 3) {
            tmp += parts[1];
        }
        else {
            tmp += '%';

            for (int i = 0; i < parts[1].size(); ++i) {
                tmp += parts[1].at(i) + '%';
            }
        }
        tmp += '-';


        if (parts[2].size() == 4) {
            tmp += parts[2];
        }
        else {
            tmp += '%';

            for (int i = 0; i < parts[2].size(); ++i) {
                tmp += parts[2].at(i) + '%';
            }
        }

        tmp.append('\'');
        tmp.prepend('\'');
    }
    else {
        tmp += "'%" + phone + "%'";
    }

    query.prepare(prep.arg(tmp));

    if (!execQuery(query)) {
        qWarning() << "Error while searching Client by phone:" << phone;
        return;
    }

    if (query.isValid()) {
        do {
            Client cln;
            cln.id      = query.value(0).toInt();
            cln.name    = query.value(1).toString();
            cln.phone   = query.value(2).toString();
            cln.descr   = query.value(3).toString();
            cln.stat_id = query.value(4).toInt();
            cln.dateReg = QDate::fromString(query.value(5).toString(), DateFormat);

            result << cln;
        }
        while (query.next());
    }
}

void DatabaseController::loadWorkers(QList<Worker> &list) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT * FROM Worker");

    if (!execQuery(query)) {
        qWarning("Error while loading Workers!");
        return;
    }

    if (query.isValid()) {
        do {
            Worker unit;
            unit.id       = query.value(0).toInt();
            unit.name     = query.value(1).toString();
            unit.dateReg  = QDate::fromString(query.value(2).toString(),
                                              DateFormat);

            list << unit;
        }
        while (query.next());
    }
}

void DatabaseController::loadStatuses(QList<Status> &list, const QString &nTable) const
{
    QSqlQuery query(mDatabase); {
        QString prep = "SELECT * FROM Status";
        if (!nTable.isEmpty()) {
            prep.append(" WHERE _table LIKE '");
            prep.append(nTable);
            prep.append("'");
        }
        query.prepare(prep);
    }


    if (!execQuery(query)) {
        qWarning("Error while loading Statuses!");
        return;
    }

    if (query.isValid()) {
        do {
            Status unit;
            unit.id    = query.value(0).toInt();
            unit.table = query.value(1).toString();
            unit.name  = query.value(2).toString();
            unit.rgb   = query.value(3).toString();

            list << unit;
        }
        while (query.next());
    }
}

void DatabaseController::loadCategories(QList<Category> &list) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT * FROM Srv_Category");

    if (!execQuery(query)) {
        qWarning("Error while loading Categories!");
        return;
    }       

    if (query.isValid()) {
        do {
            Category unit;
            unit.id   = query.value(0).toInt();
            unit.name = query.value(1).toString();
            unit.rgb  = query.value(2).toString();

            list << unit;
        }
        while (query.next());
    }
}

void DatabaseController::loadRecordsByClient(QList<Rcrd> &list, int clnId) const
{
    QSqlQuery query(mDatabase);
    query.prepare(
                "SELECT Rcrd.*, Client.name, "
                "    Client.phone, Worker.name, "
                "    Service.name, Status.name "
                "FROM Rcrd "
                "LEFT JOIN Client ON"
                "     Client.id = Rcrd.cln_id "
                "LEFT JOIN Service ON"
                "     Service.id = Rcrd.srv_id "
                "LEFT JOIN Worker ON"
                "     Worker.id = Rcrd.work_id "
                "LEFT JOIN Status ON"
                "     Status.id = Rcrd.stat_id "
                "WHERE cln_id = :id"
                );
    query.bindValue(":id", clnId);

    if (!execQuery(query)) {
        qWarning("Error while getting records. Cln_id: %i", clnId);
        return;
    }

    list.clear();

    if (!query.isValid()) {
        qDebug("There are no records by Cln_id: %i", clnId);
        return;
    }


    do {
        list.push_back({});
        auto &item = list.last();

        for (int i = 0; i < 5; ++i) {
            item.id[i] = query.value(i).toInt();
        }

        item.time = QTime::fromString(query.value(5).toString(), TimeFormat);
        item.date = QDate::fromString(query.value(6).toString(), DateFormat);
        item.client  = query.value(7).toString();
        item.phone   = query.value(8).toString();
        item.worker  = query.value(9).toString();
        item.service = query.value(10).toString();
        item.status  = query.value(11).toString();
    }
    while (query.next());
}

void DatabaseController::loadServicies(QList<Service> &list) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT * FROM Service");

    if (!execQuery(query)) {
        qWarning("Error while loading Servicies!");
        return;
    }

    if (query.isValid()) {
        do {
            Service unit;
            unit.id       = query.value(0).toInt();
            unit.name     = query.value(1).toString();
            unit.categ_id = query.value(2).toInt();
            unit.price    = query.value(3).toFloat();
            unit.dateReg  = QDate::fromString(query.value(4).toString(),
                                              DateFormat);
            list << unit;
        }
        while (query.next());
    }
}

void DatabaseController::loadServicesByCateg(QList<Service> &list, int ctgId) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT * FROM Service "
                  "WHERE catg_id = :id");
    query.bindValue(":id", ctgId);
    if (!execQuery(query)) {
        qWarning("Error while loading Servicies by ctgId: %i", ctgId);
        return;
    }

    if (query.isValid()) {
        do {
            Service unit;
            unit.id       = query.value(0).toInt();
            unit.name     = query.value(1).toString();
            unit.categ_id = query.value(2).toInt();
            unit.price    = query.value(3).toFloat();            
            unit.dateReg  = QDate::fromString(query.value(4).toString(),
                                              DateFormat);

            list << unit;
        }
        while (query.next());
    }
}

Category DatabaseController::category(int id) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT * FROM Srv_Category "
                  "WHERE id = :id");

    query.bindValue(":id", id);

    Category unit;
    if (!execQuery(query)) {
        qWarning("Error while loading Category!");
        return unit;
    }

    if (query.isValid()) {
        unit.id   = query.value(0).toInt();
        unit.name = query.value(1).toString();
        unit.rgb  = query.value(2).toString();
    }
    return unit;
}

QColor DatabaseController::colorByRcrdId(int id) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT rgb FROM status "
                  "WHERE id = ("
                  "     SELECT stat_id"
                  "     FROM Rcrd "
                  "     WHERE id = :id)");
    query.bindValue(":id", id);

    if (execQuery(query) && query.isValid()) {
        return query.value(0).toString();
    }
    return {};
}

QColor DatabaseController::colorByClntId(int id) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT rgb FROM status "
                  "WHERE id = ("
                  "     SELECT stat_id"
                  "     FROM Client"
                  "     WHERE id = :id)");
    query.bindValue(":id", id);

    if (execQuery(query) && query.isValid()) {
        return query.value(0).toString();
    }
    return {};
}

QColor DatabaseController::colorBySrvcId(int id) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT rgb FROM Srv_Category "
                  "WHERE id = ("
                  "     SELECT catg_id"
                  "     FROM Service"
                  "     WHERE id = :id)");
    query.bindValue(":id", id);

    if (execQuery(query) && query.isValid()) {
        return query.value(0).toString();
    }
    return {};
}




QString DatabaseController::dbDirName() const
{
    return {"databases"};
}

QString DatabaseController::dbFileName() const
{
    return {"main.db"};
}

QString DatabaseController::dbFilePath() const
{
    QDir dir {QApplication::applicationDirPath()};

    if ( !dir.cd(dbDirName()) )
    {
        qWarning("Dir %s doesn't exist!",
                 qUtf8Printable(dbDirName())
                 );

        if ( !dir.mkdir(dbDirName()) )
        {
            qCritical("Cannot create subdir %s in %s!",
                      qUtf8Printable(dbDirName()),
                      qUtf8Printable(dir.currentPath())
                      );

        }
        dir.cd(dbDirName());
    }

    return dir.filePath(dbFileName());
}

QString DatabaseController::dbCreateScheme() const
{
    return {
        "CREATE TABLE Status ("
        "   id 		INTEGER	PRIMARY KEY,"
        "  _table	TEXT 	NOT NULL,"
        "   name	TEXT 	NOT NULL,"
        "   rgb		TEXT"
        ");\n"
        "CREATE TABLE Client ("
        "   id 		INTEGER PRIMARY KEY,"
        "   name 	TEXT 	NOT NULL,"
        "   phone 	TEXT,"
        "   descr 	TEXT,"
        "   stat_id INTEGER NOT NULL,"
        "   dateReg TEXT 	NOT NULL,"
        "   FOREIGN KEY (stat_id)"
        "   REFERENCES Status (id)"
        ");\n"
        "CREATE INDEX cl_main ON Client (name, phone);\n"
        "CREATE TABLE Srv_Category ("
        "   id 		INTEGER PRIMARY KEY,"
        "   name 	TEXT 	NOT NULL,"
        "   rgb		TEXT"
        ");\n"
        "CREATE TABLE Service ("
        "   id 		INTEGER PRIMARY KEY,"
        "   name 	TEXT 	NOT NULL,"
        "   catg_id INTEGER,"
        "   price 	REAL	NOT NULL,"
        "   dateReg TEXT,"
        "   "
        "   CONSTRAINT catg_id"
        "       OREIGN KEY (catg_id)"
        "       REFERENCES Srv_Category (id)"
        "       ON DELETE CASCADE"
       ");\n"
        "CREATE UNIQUE INDEX srv_name ON Service (name, catg_id);\n"
        "CREATE TABLE Worker ("
        "   id 		INTEGER PRIMARY KEY,"
        "   name 	TEXT 	NOT NULL    UNIQUE,"
        "   dateReg TEXT 	NOT NULL"
       ");\n"
        "CREATE INDEX wrk_name ON Worker (name);\n"
        "CREATE TABLE Rcrd ("
        "   id 		INTEGER PRIMARY KEY,"
        "   cln_id INTEGER REFERENCES Note    (id),"
        "   work_id INTEGER REFERENCES Worker  (id),"
        "   srv_id  INTEGER REFERENCES Service (id),"
        "   stat_id INTEGER REFERENCES Status (id),"
        "   time 	TEXT 	NOT NULL,"
        "   date 	TEXT 	NOT NULL"
       ");\n"
       "CREATE INDEX Rcrd_date ON Rcrd (date);"
    };
}

bool DatabaseController::execQuery(QSqlQuery &query) const
{
    if (!query.exec()) {
        qWarning("Error executing query \n[%s]\n[%s]!",
                qPrintable(query.lastError().text()),
                qPrintable(query.executedQuery())
                );
        return false;
    }
    query.next();
    return true;
}

bool DatabaseController::selectQuery(QSqlQuery &query) const
{
    if (!query.exec() && !query.isValid()) {
        qWarning("Error executing query \n[%s]\n[%s]!",
                qPrintable(query.lastError().text()),
                qPrintable(query.executedQuery())
                );
        return false;
    }
//    else if (query.size())
//    query.next();
    return true;
}

QString DatabaseController::getColor(const QString &table, int idStatus) const
{
    QSqlQuery query(mDatabase);
    query.prepare("SELECT rgb FROM :table WHERE id = :id");
    query.bindValue(":table", table);
    query.bindValue(":id", idStatus);

    if (execQuery(query)) {
        return query.value(0).toString();
    }

    return {};
}

void DatabaseController::createDatabase()
{
    qDebug() << "Trying to create Database!" << dbFileName();

    QFile dbFile (mDatabase.databaseName());
    if (!dbFile.open(QIODevice::WriteOnly))
    {
        qFatal ("Cannot create db file!\n%s",
                qUtf8Printable(mDatabase.databaseName())
                );
    }
    dbFile.close();

    if (!mDatabase.open()) {
        qFatal ("Cannot open created db file!\n%і",
                qUtf8Printable(mDatabase.databaseName())
                );
    }

    QSqlQuery query(mDatabase);

    if (!query.exec("PRAGMA foreign_keys = ON;")) {
        qWarning("This sqlite version doesn't suport foreign keys!");
    }

    const auto queryList = dbCreateScheme().split('\n', Qt::SkipEmptyParts);
    for (auto &strQuery : queryList) {        
        if (!query.exec(strQuery))
        {
            qFatal("[%s] Error while creating database sheme\n[%s]\n[%s]!",
                   qPrintable(mDatabase.databaseName()),
                   qPrintable(query.lastError().text()),
                   qPrintable(strQuery)
                   );
        }
    }

    qDebug() << "Database has been created!" << dbFileName();
}

void DatabaseController::initTables()
{
    QSqlQuery query(mDatabase);

    QString head ("INSERT INTO Status (_table, name, rgb) VALUES");
    QString val  ("('%1', '%2', '%3'),");

    QStringList bff;
    auto makeBff = [&bff]() {
        auto str = bff.join(' ');
        str.chop(1);
        bff.clear();
        return str;
    };

    auto bVal = val.arg("Client");
    bff << head
        << bVal.arg("Regular", "#40ff00") << bVal.arg("Average", "#ffff00")
        << bVal.arg("Banned", "#ff0040")  << bVal.arg("Usual", "#a9a9a9")
        << bVal.arg("First Time", "#ff00ff");

    query.prepare(makeBff());
    execQuery(query);

    bVal = val.arg("Rcrd");
    bff << head
        << bVal.arg("Waiting", "#dcdcdc")  << bVal.arg("Completed", "#b3ff66")
        << bVal.arg("Rejected", "#cc0052") << bVal.arg("Reserved", "#9999ff");

    query.prepare(makeBff());
    execQuery(query);

    val = "(\'%1\', \'%2\'),";
    bff << "INSERT INTO Srv_Category (name, rgb) VALUES"
        << val.arg("Lashes", "#0066ff") << val.arg("MakeUp", "#99ff99")
        << val.arg("Cosmetic", "#ff66cc");

    query.prepare(makeBff());
    execQuery(query);

// Loading Initial values:
    QList<Category> list;
    loadCategories(list);
    val = QString("(%0, %1, 0.0, '%2'),").arg(InitServName
                   , QDate::currentDate().toString(DateFormat) );

    bff << "INSERT INTO Service (name, catg_id, price, descr, dateReg) VALUES";
    while (list.size() > 0) {
        auto ctg = list.takeFirst();
        bff << val.arg(ctg.id).arg(ctg.name);
    }

    query.prepare(makeBff());
    execQuery(query);
}
