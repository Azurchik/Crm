#include "DatabaseController.h"

#include <QDir>
#include <QDate>
#include <QDebug>
#include <QFileInfo>
#include <QApplication>

#include <QSqlError>
#include <QSqlQuery>
#include <QColor>

#include "CrmConsts.h"

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

int DatabaseController::contWorkers() const
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

    query.prepare("SELECT *, Note.date,"
                  "     Note.time, Client.name,"
                  "     Service.name, Worker.name "
                  "FROM Rcrd "
                  "LEFT JOIN Note ON"
                  "     Note.id = Rcrd.note_id "
                  "LEFT JOIN Service ON"
                  "     Service.id = Rcrd.srv_id "
                  "LEFT JOIN Worker ON"
                  "     Worker.id = Rcrd.work_id "
                  "LEFT JOIN Client ON"
                  "     Client.id = ("
                  "         SELECT cln_id FROM Note"
                  "         WHERE id = Rcrd.note_id ) "
                  "WHERE note_id = ("
                  "    SELECT id FROM Note"
                  "    WHERE date LIKE '__.:month.:year'"
                  "    ORDER BY date )");

    if (month < 10) {
        auto tMonth = '0' + QString::number(month);
        query.bindValue(":month", tMonth);
    }
    else {
        query.bindValue(":month", month);
    }
    query.bindValue(":year" , year);

    if (!execQuery(query)) {
        qWarning("Error while getting records. Date: %i.%i", month, year);
        return;
    }
    if (!query.isValid()) {
        qDebug("There are no records by the date: %i.%i", month, year);
        return;
    }

    list.clear();    
    do {
        list.push_back({});
        auto &item = list.last();

        for (int i = 0; i < 5; ++i) {
            item.ids[i] = query.value(i).toInt();
        }

        item.date = QDate::fromString(query.value(5).toString(), DateFormat);
        item.time = QTime::fromString(query.value(6).toString(), TimeFormat);
        item.client  = query.value(7).toString();
        item.service = query.value(8).toString();
    }
    while (query.next());

    for (auto &item : list) {
        query.prepare("SELECT Rcd_Status.rgb,"
                      "     Srv_Category.rgb,"
                      "     Cln_Status.rgb "
                      "FROM Rcrd "
                      "LEFT JOIN Rcd_Status ON"
                      "     Rcd_Status.id = Rcrd.stat_id "
                      "LEFT JOIN Srv_Category ON"
                      "     Srv_Category.id = Rcrd.srv_id "
                      "LEFT JOIN Cln_Status ON"
                      "     Cln_Status.id = ("
                      "         SELECT stat_id FROM Client"
                      "         WHERE id = ("
                      "             SELECT cln_id FROM Note"
                      "             WHERE id = Rcrd.note_id )) "
                      "WHERE Rcrd.id = :id");
        query.bindValue(":id", item.ids[Rcrd::Id]);

        if (!execQuery(query)) {
            qWarning("Error while getting color. id: %i",
                     item.ids[Rcrd::Id]);
            continue;
        }

        item.cRecord  = QColor{query.value(0).toString()};
        item.cService = QColor{query.value(1).toString()};
        item.cClient  = QColor{query.value(2).toString()};
    }
}

//QColor DatabaseController::clientColor(int idStatus) const
//{
//    return {getColor("Cln_Status", idStatus)};
//}

//QColor DatabaseController::recordColor(int idStatus) const
//{
//    return {getColor("Rcd_Status", idStatus)};
//}

//QColor DatabaseController::serviceColor(int idStatus) const
//{
//    return {getColor("Srv_Category", idStatus)};
//}

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
    "CREATE TABLE IF NOT EXISTS Cln_Status ( "
    "   id 		INTEGER	PRIMARY KEY, "
    "   name	TEXT 	NOT NULL, "
    "   rgb		TEXT "
    ");\n "
    "CREATE TABLE IF NOT EXISTS Client ( "
    "   id 		INTEGER PRIMARY KEY, "
    "   name 	TEXT 	NOT NULL, "
    "   phone 	TEXT, "
    "   descr 	TEXT, "
    "   stat_id INTEGER NOT NULL 	DEFAULT 0, "
    "   dateReg TEXT 	NOT NULL, "
    "   FOREIGN KEY (stat_id) "
    "   REFERENCES Сln_Status (id) "
    ");\n "
    "CREATE INDEX IF NOT EXISTS cl_main "
        "ON Client (name, phone);\n "
    "CREATE TABLE IF NOT EXISTS Note ( "
    "   id 	 	INTEGER PRIMARY KEY, "
    "   time 	TEXT 	NOT NULL, "
    "   date 	TEXT 	NOT NULL, "
    "   cln_id 	INTEGER NOT NULL, "
    "   dateReg TEXT 	NOT NULL, "
    "   FOREIGN KEY (cln_id) "
    "   REFERENCES Client (id) "
    ");\n "
    "CREATE INDEX IF NOT EXISTS note_date "
        "ON Note (date);\n "
    "CREATE TABLE IF NOT EXISTS Srv_Category ( "
    "   id 		INTEGER PRIMARY KEY, "
    "   name 	TEXT 	NOT NULL, "
    "   rgb		TEXT "
    ");\n "
    "CREATE TABLE IF NOT EXISTS Service ( "
    "   id 		INTEGER PRIMARY KEY, "
    "   name 	TEXT 	NOT NULL, "
    "   catg_id INTEGER, "
    "   price 	REAL	NOT NULL, "
    "   descr 	TEXT, "
    "   dateReg TEXT, "
    "   FOREIGN KEY (catg_id) "
    "   REFERENCES Srv_Category (id) "
    ");\n "
    "CREATE INDEX IF NOT EXISTS srv_name "
        "ON Service (name);\n "
    "CREATE TABLE IF NOT EXISTS Worker ( "
    "   id 		INTEGER PRIMARY KEY, "
    "   name 	TEXT 	NOT NULL, "
    "   dateReg TEXT 	NOT NULL "
    ");\n "
    "CREATE INDEX IF NOT EXISTS wrk_name "
        "ON Worker (name);\n "
    "CREATE TABLE IF NOT EXISTS Rcd_Status ( "
    "   id 		INTEGER PRIMARY KEY, "
    "   name 	TEXT 	NOT NULL, "
    "   rgb		TEXT "
    ");\n "
    "CREATE TABLE IF NOT EXISTS Rcrd ( "
    "   id 		INTEGER PRIMARY KEY, "
    "   note_id INTEGER REFERENCES Note    (id), "
    "   work_id INTEGER REFERENCES Worker  (id), "
    "   srv_id  INTEGER REFERENCES Service (id), "
    "   stat_id INTEGER REFERENCES Rcd_Status (id) "
    ");"
    };
}

bool DatabaseController::execQuery(QSqlQuery &query) const
{
    if (!query.exec() && !query.isValid()) {
        qWarning("Error executing query \n[%s]\n[%s]!",
                qPrintable(query.lastError().text()),
                qPrintable(query.executedQuery())
                );
        return false;
    }
    query.next();
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

    QString table ("INSERT INTO %1 (name, rgb) VALUES");
    QString val   ("(\'%1\', \'%2\'),");

    QStringList bff;
    auto makeBff = [&bff]() {
        auto str = bff.join(' ');
        str.chop(1);
        bff.clear();
        return str;
    };


    bff << table.arg("Cln_Status")      << val.arg("Usual", "#a9a9a9")
        << val.arg("Regular", "#40ff00") << val.arg("Average", "#ffff00")
        << val.arg("Banned", "#ff0040")  << val.arg("First Time", "#ff00ff");

    query.prepare(makeBff());
    execQuery(query);

    bff << table.arg("Srv_Category")   << val.arg("Lashes", "#0066ff")
        << val.arg("MakeUp", "#99ff99") << val.arg("Cosmetc", "#ff66cc");

    query.prepare(makeBff());
    execQuery(query);

    bff << table.arg("Rcd_Status")        << val.arg("General", "#dcdcdc")
        << val.arg("Completed", "#b3ff66") << val.arg("Rejected", "#cc0052")
        << val.arg("Reserved", "#9999ff");

    query.prepare(makeBff());
    execQuery(query);

}
