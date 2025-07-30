#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>

class Task
{
public:
    int id;
    QString title;
    QString description;
    int creatorId;
    int assigneeId;
    QString status;
    QDateTime deadline;
    QDateTime createdAt;

    Task(int id,
         const QString &title,
         const QString &description,
         int creatorId,
         int assigneeId,
         const QString &status,
         const QDateTime &deadline,
         const QDateTime &createdAt);

    Task();
};

#endif // TASK_H
