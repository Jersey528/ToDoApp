#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
