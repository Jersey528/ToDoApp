#ifndef CREATETASKDIALOG_H
#define CREATETASKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateTimeEdit>

#include "task.h"
#include "user.h"

class CreateTaskDialog : public QDialog {
    Q_OBJECT

public:
    CreateTaskDialog(int currentUserId, QWidget *parent = nullptr);
    Task getTask() const;
    void loadTaskForEdit(const Task &task);

private:
    QLineEdit *titleEdit;
    QTextEdit *descriptionEdit;
    QComboBox *assigneeCombo;
    QDateTimeEdit *deadlineEdit;
    QList<User> assignableUsers;
    int currentUserId;
    //QPushButton *createButton;

    bool editing = false;
    Task originalTask;

signals:
    void taskCreated(const Task &task);
    void taskEdited(const Task &task);

};


#endif // CREATETASKDIALOG_H
