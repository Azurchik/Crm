#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QHash>

class DatabaseController : public QObject
{
    Q_OBJECT    
public:
    explicit DatabaseController(QObject *parent = nullptr);

    QColor clientColor(int idStatus) const;
    QColor recordColor(int idStatus) const;
    QColor serviceColor(int idStatus) const;

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

#endif // DATABASECONTROLLER_H
