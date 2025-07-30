#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QCryptographicHash>

#include "task.h"
#include "taskviewtype.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance(); // Singleton

    bool connect(const QString &ip, const int &port, const QString &dbName,
                 const QString &user, const QString &password);

    std::optional<int> authenticate(const QString &login, const QString &password);

    QString getUserRole(const QString &login);
    QString getUserName(int userId);
    QList<Task> getTasksByUser(int userId, TaskViewType type);

private:
    DatabaseManager();
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
