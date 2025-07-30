#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>

#include "task.h"
#include "taskviewtype.h"

class TaskWidget : public QWidget
{
    Q_OBJECT
public:
    TaskWidget(const Task &task, const QString &counterpartyName, TaskViewType type, QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QLabel *titleLabel;
    QLabel *statusLabel;
    QLabel *counterpartyLabel;
    QLabel *deadlineLabel;

    Task task;
    TaskViewType viewType;
    QString counterpartyLabelText;

    QPushButton *menuButton;

    void showContextMenuAt(const QPoint &globalPos);

signals:
    void requestDelete(int taskId);
    void requestEdit(const Task &task);
    void requestStatusChange(int taskId, QString newStatus);
    void requestDetails(const Task &task);
};

#endif // TASKWIDGET_H
