#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QHash>

#include <QColor>
#include <QTime>

struct Record;
struct Worker;
struct Client;
struct Status;
struct Service;
struct Category;

class DatabaseController
{  
public:
    DatabaseController();

    Status getStatus(int id) const;
    Record getRecord(int id) const;

    int servIdByNameCatg(const QString &name, int catg_id) const;
    int categoryIdByName(const QString &name) const;
    int workerIdByName(const QString &name) const;

    bool updateClient(const Client &cln);
    bool updateWorker(const Worker &wrk);
    bool updateRecord(const Record &rec);
    bool updateService(const Service &srv);
    bool updateCategory(const Category &ctg);

    bool deleteClient(int id);
    bool deleteWorker(int id);
    bool deleteRecord(int id);
    bool deleteService(int id);
    bool deleteCategory(int id);

    bool addRecord(const Record &rcd);
    bool addClient(const Client &cln);
    bool addWorker(const Worker &wrk);
    bool addService(const Service &srv);
    bool addCategory(const Category &ctg);

    int  countWorkers() const;
    void initRecordList(QList<Record> &list, int month, int year) const;
    void searchClientByName(const QString &name, QList<Client> &result) const;
    void searchClientByPhone(const QString &phone, QList<Client> &result,
                             bool wasStrongPhoneValidation ) const;

    void loadClients(QList<Client> &list) const;
    void loadWorkers(QList<Worker> &list) const;
    void loadStatuses(QList<Status> &list, const QString &nTable) const;
    void loadServicies(QList<Service> &list) const;
    void loadCategories(QList<Category> &list) const;
    void loadRecordsByClient(QList<Record> &list, int clnId) const;
    void loadServicesByCateg(QList<Service> &list, int ctgId) const;    

    Category category(int id) const;

    QColor colorByRcrdId(int id) const;
    QColor colorByClntId(int id) const;
    QColor colorBySrvcId(int id) const;

    QString dbDirName()  const;
    QString dbFileName() const;
    QString dbFilePath() const;
    QString dbCreateScheme() const;

private:
    bool execQuery(QSqlQuery &query) const;
    bool selectQuery(QSqlQuery &query) const;

    QString getColor(const QString &table, int idStatus) const;
    void createDatabase();
    void initTables();

    QSqlDatabase mDatabase;
};

struct Status {
    int id = 0;
    QString table;
    QString name;
    QColor rgb;
};


struct Worker {
    int id = 0;
    QString name;
    QDate dateReg = QDate::currentDate();
};

struct Service {
    int id = 0;
    QString name;
    int categ_id = 0;
    float price = 0.0;
    QDate dateReg = QDate::currentDate();
};

struct Category {
    int id = 0;
    QString name;
    QColor rgb;

//    Category& operator= (const Category &other);
};

struct Client {
    int id = 0;
    QString name;
    QString phone;
    QString descr;
    int stat_id = 0;
    QDate dateReg = QDate::currentDate();
};

struct Record
{
    int id[5] = {0};

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString client;
    QString phone;
    QString service;
    QString worker;
    QString status;    

    enum identifier {
        Id = 0,
        ClnId,
        WorkId,
        SrvId,
        StatId
    };
};



#endif // DATABASECONTROLLER_H
