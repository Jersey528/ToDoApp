#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QCryptographicHash>

#include "task.h"
#include "taskviewtype.h"
#include "user.h"

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
    QList<User> getAssignableUsers(int supervisorId);

    bool createTask(const Task &task);
    bool deleteTask(int taskId);
    bool updateTaskStatus(int taskId, const QString &newStatus);
    bool updateTask(const Task &task);

private:
    DatabaseManager();
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
