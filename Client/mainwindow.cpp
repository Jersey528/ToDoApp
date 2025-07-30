#include "mainwindow.h"
#include "databasemanager.h"
#include "taskwidget.h"

//#include <QVBoxLayout>
//#include <QLabel>
//#include <QFrame>

MainWindow::MainWindow(int userId, QWidget *parent)
    : QMainWindow(parent), currentUserId(userId)
{
    // Створюємо центральний віджет
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // Створюємо центральний лейаут
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);



    // Створюємо верхню панель
    QFrame *topPanel = createStylizedFrame(this);
    QHBoxLayout *topLayout = new QHBoxLayout();
    topPanel->setLayout(topLayout);

    // Кнопка бокового меню
    QPushButton *menuButton = new QPushButton("≡", this);
    menuButton->setFixedSize(30, 30);

    // Кнопка створення задачі
    QPushButton *createTaskButton = new QPushButton("＋ Створити задачу", this);
    createTaskButton->setFixedHeight(30);

    // Ім’я користувача + (умовно) іконка
    QString userName = DatabaseManager::instance().getUserName(currentUserId);
    //userLabel->setText("👤 " + userName);

    QLabel *userLabel = new QLabel(userName, this);
    userLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    userLabel->setStyleSheet("color: black; font-weight: bold; padding-right: 10px;");

    // Додаємо в layout
    topLayout->addWidget(menuButton);
    topLayout->addStretch(); // розділення між кнопками
    topLayout->addWidget(createTaskButton);
    topLayout->addStretch();
    topLayout->addWidget(userLabel);



    // Створюємо нижню основну панель
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);  // щоб автоматично масштабувалось
    scrollArea->setFrameShape(QFrame::NoFrame);

    // Вміст для прокрутки
    scrollWidget = new QWidget();
    taskListLayout = new QVBoxLayout(scrollWidget);

/*
    // Приклад кількох задач (тимчасово)
    for (int i = 1; i <= 10; ++i) {
        QLabel *task = new QLabel(QString("📝 Задача #%1").arg(i));
        task->setStyleSheet("padding: 10px; background: #f0f0f0; border-radius: 5px;");
        taskListLayout->addWidget(task);
    }
*/

    // Додаємо stretch, щоб задачі притискались вгору
    taskListLayout->addStretch();

    scrollWidget->setLayout(taskListLayout);
    scrollArea->setWidget(scrollWidget);




    // ========== Side Menu ==========

    sideMenu = new QFrame(this);
    sideMenu->setFixedWidth(200);
    sideMenu->setStyleSheet("background-color: #ddd; color: white; border: 1px solid #555;");
    sideMenu->move(-200, 0);  // спочатку приховано зліва
    sideMenu->setParent(this);
    sideMenu->raise();

    QVBoxLayout *sideLayout = new QVBoxLayout(sideMenu);
    QPushButton *btnAssignedByMe = new QPushButton("Поставлені задачі", sideMenu);
    QPushButton *btnAssignedToMe = new QPushButton("Отримані задачі", sideMenu);
    btnAssignedByMe->setStyleSheet("background-color: #bbb; color: #000; font-weight: bold; padding-right: 10px;");
    btnAssignedToMe->setStyleSheet("background-color: #bbb; color: #000; font-weight: bold; padding-right: 10px;");
    sideLayout->addWidget(btnAssignedByMe);
    sideLayout->addWidget(btnAssignedToMe);
    sideLayout->addStretch();

    // ========== Анімація меню ==========
    menuAnimation = new QPropertyAnimation(sideMenu, "pos", this);
    menuAnimation->setDuration(300);

    // ========== Заміна кнопки "≡" ==========
    connect(menuButton, &QPushButton::clicked, this, &MainWindow::toggleSideMenu);

    // ========== Обробка перемикання ==========
    connect(btnAssignedByMe, &QPushButton::clicked, [=]() {
        loadTasks(TaskViewType::AssignedByMe);
        toggleSideMenu();
    });
    connect(btnAssignedToMe, &QPushButton::clicked, [=]() {
        loadTasks(TaskViewType::AssignedToMe);
        toggleSideMenu();
    });




    //Додаємо все в layout
    layout->addWidget(topPanel, 11);
    layout->addWidget(scrollArea, 89);
    //Присвоюємо головний лейаут центральному віджету
    centralWidget->setLayout(layout);

    // Приклад простого інтерфейсу
    //QLabel *label = new QLabel("Вітаємо в ToDoApp!", this);
    //layout->addWidget(label);


    sideMenu->raise();  // Щоб бокове меню було поверх усіх елементів
    sideMenu->show();   // Переконайся, що воно показується

    currentView = TaskViewType::AssignedByMe;
    loadTasks(currentView);
    sideMenu->setFixedHeight(this->height());

}

MainWindow::~MainWindow() {}

QFrame *MainWindow::createStylizedFrame(QWidget *parent) {
    QFrame *frame = new QFrame(parent);
    frame->setStyleSheet(
        "QFrame {"
        //"    background-color: #2b2b2b;"
        "    color: white;"
        "    border: none;"
        "    border: 1px solid #654b25;"
        "    border-radius: 5px;"
        "    padding: 4px;"
        "}"
        );

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setOffset(5, 5);
    shadowEffect->setColor(QColor(0, 0, 0, 30));
    frame->setGraphicsEffect(shadowEffect);

    return frame;
}

void MainWindow::toggleSideMenu() {
    int startX = sideMenu->x();
    int endX = (startX < 0) ? 0 : -200; // Показати або сховати
    menuAnimation->stop();
    menuAnimation->setStartValue(QPoint(startX, 0));
    menuAnimation->setEndValue(QPoint(endX, 0));
    menuAnimation->start();
}

void MainWindow::loadTasks(TaskViewType type) {
    currentView = type;

    // Очистка попередніх задач
    QLayoutItem *item;
    while ((item = taskListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QList<Task> tasks = DatabaseManager::instance().getTasksByUser(currentUserId, type);

    for (const Task &task : tasks) {
        int counterpartyId = (type == TaskViewType::AssignedByMe) ? task.assigneeId : task.creatorId;
        QString counterparty = DatabaseManager::instance().getUserName(counterpartyId);

        QString labelPrefix = (type == TaskViewType::AssignedByMe) ? "Для: " : "Від: ";
        QString counterpartyName = labelPrefix + counterparty;

        TaskWidget *widget = new TaskWidget(task, counterpartyName, type, this);



        connect(widget, &TaskWidget::requestEdit, this, &MainWindow::editTask);
        connect(widget, &TaskWidget::requestDelete, this, &MainWindow::deleteTask);
        connect(widget, &TaskWidget::requestStatusChange, this, &MainWindow::changeTaskStatus);
        connect(widget, &TaskWidget::requestDetails, this, &MainWindow::showTaskDetails);



        //widget->setStyleSheet("background-color: white; border: 2px solid #444; border-radius: 8px;");
        taskListLayout->addWidget(widget);
    }

    taskListLayout->addStretch();
}

void MainWindow::editTask(const Task &task) {
    // TODO: Відкрити вікно редагування
    qDebug() << "Редагування задачі з ID:" << task.id;
}

void MainWindow::deleteTask(int taskId) {
    // TODO: Видалити з БД
    qDebug() << "Видалення задачі з ID:" << taskId;
}

void MainWindow::changeTaskStatus(int taskId, const QString &newStatus) {
    // TODO: Оновити статус в БД
    qDebug() << "Зміна статусу задачі " << taskId << " на " << newStatus;
}

void MainWindow::showTaskDetails(const Task &task)
{
    // TODO: Зробити вікно проглядання опису
    qDebug() << "Проглядання опису:" << task.id;
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    sideMenu->setFixedHeight(this->height());
}
