#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QQueue>

DatabaseManager::DatabaseManager()
{
    db = QSqlDatabase::addDatabase("QMYSQL");  // драйвер MySQL
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::connect(const QString &ip, const int &port, const QString &dbName,
                              const QString &user, const QString &password)
{
    db.setHostName(ip);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);
    if (!db.open()) {
        qWarning() << "Помилка з'єднання з базою:" << db.lastError().text();
        return false;
    }
    return true;
}

std::optional<int> DatabaseManager::authenticate(const QString &login, const QString &password) {
    QSqlQuery query;
    query.prepare("SELECT id, password_hash FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (query.exec() && query.next()) {
        int userId = query.value("id").toInt();
        QString storedHash = query.value("password_hash").toString();

        // Перевірка пароля
        if (storedHash == QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex()) {
            return userId;
        }
    }

    return std::nullopt;  // авторизація неуспішна
}

QString DatabaseManager::getUserRole(const QString &login)
{
    QSqlQuery query;
    query.prepare("SELECT role FROM users WHERE login = ?");
    query.addBindValue(login);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "";
}

QString DatabaseManager::getUserName(int userId) {
    QSqlQuery query;
    query.prepare("SELECT name FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return QString("Невідомий користувач");
}

QList<Task> DatabaseManager::getTasksByUser(int userId, TaskViewType type) {
    QList<Task> tasks;
    QSqlQuery query;

    QString queryString = R"(
        SELECT id, title, description, creator_id, assignee_id, status, deadline, created_at
        FROM tasks
        WHERE %1 = :userId
        ORDER BY deadline
    )"; // Можливо ORDER BY created_at

    // В залежності від типу задачі змінюємо фільтр
    queryString = queryString.arg(type == TaskViewType::AssignedByMe ? "creator_id" : "assignee_id");

    query.prepare(queryString);
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "Помилка виконання запиту на отримання задач:" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Task task;
        task.id = query.value("id").toInt();
        task.title = query.value("title").toString();
        task.description = query.value("description").toString();
        task.creatorId = query.value("creator_id").toInt();
        task.assigneeId = query.value("assignee_id").toInt();
        task.status = query.value("status").toString();
        task.deadline = query.value("deadline").toDateTime();
        task.createdAt = query.value("created_at").toDateTime();

        tasks.append(task);
    }

    return tasks;
}

QList<User> DatabaseManager::getAssignableUsers(int supervisorId) {
    QList<User> result;
    QSet<int> visited;
    QQueue<int> queue;
    queue.enqueue(supervisorId);
    visited.insert(supervisorId);

    while (!queue.isEmpty()) {
        int current = queue.dequeue();

        QSqlQuery query(db);
        query.prepare("SELECT id, name FROM users WHERE supervisor_id = :id");
        query.bindValue(":id", current);
        query.exec();

        while (query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();
            result.append(User{id, name});

            if (!visited.contains(id)) {
                queue.enqueue(id);
                visited.insert(id);
            }
        }
    }

    return result;
}

bool DatabaseManager::createTask(const Task &task)
{
    QSqlQuery query(db);

    query.prepare(R"(
        INSERT INTO tasks (title, description, creator_id, assignee_id, status, deadline)
        VALUES (:title, :description, :creator_id, :assignee_id, :status, :deadline)
    )");

    query.bindValue(":title", task.title);
    query.bindValue(":description", task.description);
    query.bindValue(":creator_id", task.creatorId);
    query.bindValue(":assignee_id", task.assigneeId);
    query.bindValue(":status", task.status);  // Має бути "new" при створенні
    query.bindValue(":deadline", task.deadline.toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "Помилка створення задачі:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteTask(int taskId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM tasks WHERE id = :id");
    query.bindValue(":id", taskId);
    if (!query.exec()) {
        qDebug() << "Помилка видалення задачі:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateTaskStatus(int taskId, const QString &newStatus) {
    QSqlQuery query(db);
    query.prepare("UPDATE tasks SET status = :status WHERE id = :id");
    query.bindValue(":status", newStatus);
    query.bindValue(":id", taskId);
    if (!query.exec()) {
        qDebug() << "Помилка оновлення статусу:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateTask(const Task &task)
{
    QSqlQuery query(db);
    query.prepare("UPDATE tasks SET title = :title, description = :description, deadline = :deadline, assignee_id = :assignee_id WHERE id = :id");
    query.bindValue(":title", task.title);
    query.bindValue(":description", task.description);
    query.bindValue(":deadline", task.deadline);
    query.bindValue(":assignee_id", task.assigneeId);
    query.bindValue(":id", task.id);

    if (!query.exec()) {
        qDebug() << "Помилка оновлення задачі:" << query.lastError().text();
        return false;
    }
    return true;
}
