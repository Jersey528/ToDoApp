#include "taskwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QContextMenuEvent>
#include <QDebug>

TaskWidget::TaskWidget(const Task &task, const QString &counterpartyName,TaskViewType type, QWidget *parent)
    : QWidget(parent)
{
    // Зовнішній лейаут TaskWidget
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Стилізована обгортка (frameContainer)
    QWidget *frameContainer = new QWidget(this);
    frameContainer->setObjectName("frameContainer");
    frameContainer->setStyleSheet(R"(
        #frameContainer {
            background-color: white;
            border: 1px solid #654b25;
            border-radius: 5px;
            padding: 4px;
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(frameContainer);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    // Верхній ряд: Назва та Статус
    QHBoxLayout *topRow = new QHBoxLayout();

    // Кнопка з крапками ⋮
    this->viewType = type;

    menuButton = new QPushButton("⋮", frameContainer);
    menuButton->setFixedSize(5, 24);
    menuButton->setStyleSheet("QPushButton { border: none; font-weight: bold; font-size: 22px; }");
    topRow->addWidget(menuButton);

    connect(menuButton, &QPushButton::clicked, this, [this]() {
        QPoint globalPos = menuButton->mapToGlobal(QPoint(0, menuButton->height()));
        showContextMenuAt(globalPos);
    });

    // Назва задачі
    QLabel *titleLabel = new QLabel(task.title, frameContainer);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");

    //QLabel *statusLabel = new QLabel(task.status, frameContainer);
    //statusLabel->setStyleSheet("font-size: 12px; color: #666;");

    QString statusText;
    QString statusColor;

    if (task.status == "new") {
        statusText = "• В черзі";
        statusColor = "#e6b800"; // жовта крапка
    } else if (task.status == "in_progress") {
        statusText = "• Виконується";
        statusColor = "#009933"; // зелена крапка
    } else if (task.status == "done") {
        statusText = "• Завершено";
        statusColor = "#800080"; // фіолетова крапка
    } else {
        statusText = "• Невідомо";
        statusColor = "#999999"; // сірий, якщо щось пішло не так
    }

    QLabel *statusLabel = new QLabel(statusText, frameContainer);
    statusLabel->setStyleSheet(QString(
                                   "color: %1;"
                                   "font-size: 12px;"
                                   "font-weight: bold;"
                                   ).arg(statusColor));




    topRow->addWidget(titleLabel);
    topRow->addStretch();
    topRow->addWidget(statusLabel);


    // Нижній ряд: Контрагент + Термін
    QHBoxLayout *bottomRow = new QHBoxLayout();
    QLabel *counterpartyLabel = new QLabel(counterpartyName, frameContainer);


    //QLabel *deadlineLabel = new QLabel(task.deadline.toString("dd.MM.yyyy HH:mm"), frameContainer);
    //deadlineLabel->setStyleSheet("color: #888; font-size: 12px;");

    QDateTime now = QDateTime::currentDateTime();
    QString deadlineText;
    QString deadlineColor = "#888";  // стандартний сірий колір

    if (task.deadline < now && task.status != "done") {
        deadlineText = "Протерміновано: " + task.deadline.toString("dd.MM.yyyy HH:mm");
        deadlineColor = "#cc0000";  // червоний
    } else {
        deadlineText = "Термін виконання: " + task.deadline.toString("dd.MM.yyyy HH:mm");
    }

    QLabel *deadlineLabel = new QLabel(deadlineText, frameContainer);
    deadlineLabel->setStyleSheet(QString(
                                     "color: %1;"
                                     "font-size: 12px;"
                                     ).arg(deadlineColor));


    bottomRow->addWidget(counterpartyLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(deadlineLabel);

    // Додаємо рядки до лейауту
    layout->addLayout(topRow);
    layout->addLayout(bottomRow);

    // Додаємо стилізовану обгортку до головного лейауту
    outerLayout->addWidget(frameContainer);

    // Мінімальна висота для кращого вигляду
    this->setMinimumHeight(60);
}

void TaskWidget::contextMenuEvent(QContextMenuEvent *event)
{
    showContextMenuAt(event->globalPos());
}

void TaskWidget::showContextMenuAt(const QPoint &globalPos)
{
    //qDebug() << "Context menu requested at" << globalPos;
    //qDebug() << "Task ID:" << task.id << "ViewType:" << static_cast<int>(viewType);

    QMenu menu(this);

    menu.setStyleSheet(
        "QMenu {"
        "  background-color: white;"
        "  border: 1px solid gray;"
        "}"
        "QMenu::item {"
        "  padding: 6px 10px;"
        "  margin: 0px;"
        "  border: none;"
        "  color: black;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #d0d0d0;"  // або будь-який колір наведення
        "}"
        "QMenu::icon {"
        "  width: 0px;"
        "}"
        "QMenu::indicator {"
        "  width: 0px;"
        "}"
    );

    if (viewType == TaskViewType::AssignedByMe) {

        menu.addAction("Змінити задачу", [this]() {
            emit requestEdit(task);
        });
        menu.addAction("Видалити задачу", [this]() {
            emit requestDelete(task.id);
        });
    } else if (viewType == TaskViewType::AssignedToMe) {
        QMenu *statusMenu = menu.addMenu("Змінити статус  ");

        statusMenu->addAction("В черзі", [this]() {
            emit requestStatusChange(task.id, "new");
        });
        statusMenu->addAction("Виконується", [this]() {
            emit requestStatusChange(task.id, "in_progress");
        });
        statusMenu->addAction("Завершено", [this]() {
            emit requestStatusChange(task.id, "done");
        });
    }

    menu.addAction("Проглянути деталі", [this]() {
        emit requestDetails(task);
    });

    menu.exec(globalPos);
}
