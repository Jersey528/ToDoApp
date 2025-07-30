#include "task.h"

Task::Task(int id,
           const QString &title,
           const QString &description,
           int creatorId,
           int assigneeId,
           const QString &status,
           const QDateTime &deadline,
           const QDateTime &createdAt)
    : id(id),
    title(title),
    description(description),
    creatorId(creatorId),
    assigneeId(assigneeId),
    status(status),
    deadline(deadline),
    createdAt(createdAt)
{}

Task::Task()
    : id(-1), creatorId(-1), assigneeId(-1),
    status("new"), deadline(QDateTime()), createdAt(QDateTime()) {}
