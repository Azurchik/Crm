#include "DatabaseController.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QApplication>

#include <QSqlError>
#include <QSqlQuery>
#include <QColor>

DatabaseController::DatabaseController(QObject *parent)
    : QObject (parent)
    , mDatabase(QSqlDatabase::addDatabase("QSQLITE"))
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

QColor DatabaseController::clientColor(int idStatus) const
{
    return {getColor("Cln_Status", idStatus)};
}

QColor DatabaseController::recordColor(int idStatus) const
{
    return {getColor("Rcd_Status", idStatus)};
}

QColor DatabaseController::serviceColor(int idStatus) const
{
    return {getColor("Srv_Category", idStatus)};
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
    }
    return query.next();
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


    bff << table.arg("Cln_Status")      << val.arg("Usual", "a9a9a9")
        << val.arg("Regular", "40ff00") << val.arg("Average", "ffff00")
        << val.arg("Banned", "ff0040")  << val.arg("First Time", "ff00ff");

    query.prepare(makeBff());
    execQuery(query);

    bff << table.arg("Srv_Category")   << val.arg("Lashes", "0066ff")
        << val.arg("MakeUp", "99ff99") << val.arg("Cosmetc", "ff66cc");

    query.prepare(makeBff());
    execQuery(query);

    bff << table.arg("Rcd_Status")        << val.arg("General", "dcdcdc")
        << val.arg("Completed", "b3ff66") << val.arg("Rejected", "cc0052")
        << val.arg("Reserved", "9999ff");

    query.prepare(makeBff());
    execQuery(query);

}
