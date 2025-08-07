#include "createtaskdialog.h"
#include "databasemanager.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

CreateTaskDialog::CreateTaskDialog(int currentUserId, QWidget *parent)
    : QDialog(parent), currentUserId(currentUserId)
{

    setWindowTitle("Створити нову задачу");
    setModal(true);
    resize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ===== Назва задачі =====
    QLabel *titleLabel = new QLabel("Назва задачі:", this);
    titleEdit = new QLineEdit(this);

    // ===== Опис задачі =====
    QLabel *descriptionLabel = new QLabel("Опис задачі:", this);
    descriptionEdit = new QTextEdit(this);

    // ===== Виконавець =====
    QLabel *assigneeLabel = new QLabel("Оберіть виконавця:", this);
    assigneeCombo = new QComboBox(this);

    // Завантаження користувачів, яким можна ставити задачі
    assignableUsers = DatabaseManager::instance().getAssignableUsers(currentUserId);
    for (const User &user : assignableUsers) {
        assigneeCombo->addItem(user.name, user.id);
    }

    // ===== Дедлайн =====
    QLabel *deadlineLabel = new QLabel("Дедлайн:", this);
    deadlineEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), this);
    deadlineEdit->setCalendarPopup(true);
    deadlineEdit->setDisplayFormat("dd.MM.yyyy HH:mm");

    // ===== Кнопки =====
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *createButton = new QPushButton("Готово", this);
    QPushButton *cancelButton = new QPushButton("Відміна", this);

    connect(createButton, &QPushButton::clicked, [this]() {
        if (titleEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Помилка", "Назва задачі не може бути порожньою.");
            return;
        }
        if (assigneeCombo->currentIndex() < 0) {
            QMessageBox::warning(this, "Помилка", "Оберіть виконавця.");
            return;
        }

        if (editing) {
            Task editedTask = getTask();
            editedTask.id = originalTask.id;  // зберігаємо ID оригінальної задачі
            emit taskEdited(editedTask);
        } else {
            emit taskCreated(getTask());
        }

        accept();
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(cancelButton);

    // ===== Збірка =====
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(titleEdit);

    mainLayout->addWidget(descriptionLabel);
    mainLayout->addWidget(descriptionEdit);

    mainLayout->addWidget(assigneeLabel);
    mainLayout->addWidget(assigneeCombo);

    mainLayout->addWidget(deadlineLabel);
    mainLayout->addWidget(deadlineEdit);

    mainLayout->addLayout(buttonLayout);
}

Task CreateTaskDialog::getTask() const {
    Task task = originalTask;
    task.title = titleEdit->text().trimmed();
    task.description = descriptionEdit->toPlainText().trimmed();
    task.creatorId = currentUserId;
    task.assigneeId = assigneeCombo->currentData().toInt();
    task.deadline = deadlineEdit->dateTime();
    return task;
}

void CreateTaskDialog::loadTaskForEdit(const Task &taskToEdit)
{
    editing = true;
    originalTask = taskToEdit;

    titleEdit->setText(taskToEdit.title);
    descriptionEdit->setText(taskToEdit.description);
    deadlineEdit->setDateTime(taskToEdit.deadline);

    for (int i = 0; i < assigneeCombo->count(); ++i) {
        if (assigneeCombo->itemData(i).toInt() == taskToEdit.assigneeId) {
            assigneeCombo->setCurrentIndex(i);
            break;
        }
    }

    setWindowTitle("Редагувати задачу");
}
