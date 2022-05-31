#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QHash>

#include <QColor>
#include <QTime>

struct Rcrd;

class DatabaseController
{  
public:
    DatabaseController();

    int contWorkers() const;
    void initRecordList(QList<Rcrd> &list, int month, int year) const;

    QString dbDirName() const;
    QString dbFileName() const;
    QString dbFilePath() const;
    QString dbCreateScheme() const;

private:
    bool execQuery(QSqlQuery &query) const;

    QString getColor(const QString &table, int idStatus) const;
    void createDatabase();
    void initTables();

    QSqlDatabase mDatabase;
};

struct Rcrd
{
    QDate date;
    QTime time;
    QString client;
    QString service;
    QString worker;

    QColor cClient  {Qt::white};
    QColor cRecord  {Qt::white};
    QColor cService {Qt::white};

    int ids[5] = {0};

    enum ID {
        Id = 0,
        NoteId,
        WorkId,
        SrvId,
        StatId
    };
};



#endif // DATABASECONTROLLER_H
