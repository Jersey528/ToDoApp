#include "mainwindow.h"
#include "databasemanager.h"
#include "taskwidget.h"
#include "createtaskdialog.h"

#include <QEvent>
#include <QTimer>

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
    QPushButton *menuButton = createStyledButton("≡"); //QPushButton *menuButton = new QPushButton("≡", this);
    menuButton->setFixedSize(30, 30);

    // Кнопка створення задачі
    QPushButton *createTaskButton = createStyledButton("＋ Створити задачу"); //QPushButton *createTaskButton = new QPushButton("＋ Створити задачу", this);
    createTaskButton->setFixedHeight(30);

    connect(createTaskButton, &QPushButton::clicked, this, &MainWindow::openCreateTaskDialog);


    // Ім’я користувача
    QString userName = "👤 " + DatabaseManager::instance().getUserName(currentUserId);

    QPushButton *userLabel = createStyledButton(userName); //QLabel *userLabel = new QLabel(userName, this);

    //userLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    //userLabel->setStyleSheet("color: black; font-weight: bold; padding-right: 10px;");

    // Додаємо в layout
    topLayout->addWidget(menuButton);
    topLayout->addStretch(); // розділення між кнопками
    topLayout->addWidget(createTaskButton);
    topLayout->addStretch();
    topLayout->addWidget(userLabel);


    // Індикація поточно вибраної вкладки
    viewTypeLabel = new QLabel(this);
    viewTypeLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    viewTypeLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #000; border: none;");


    // Створюємо нижню основну панель
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);  // щоб автоматично масштабувалось
    scrollArea->setFrameShape(QFrame::NoFrame);

    // Вміст для прокрутки
    scrollWidget = new QWidget();
    taskListLayout = new QVBoxLayout(scrollWidget);

    // Плейсхолдер
    placeholderLabel = new QLabel("Задач поки немає", scrollWidget);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet("color: gray; font-size: 16px;");
    placeholderLabel->hide();  // за замовчуванням прихований

    taskListLayout->addWidget(placeholderLabel, 0, Qt::AlignCenter);

    // Додаємо stretch, щоб задачі притискались вгору
    taskListLayout->addStretch();

    scrollWidget->setLayout(taskListLayout);
    scrollArea->setWidget(scrollWidget);


    // ======== Фон для відкритого бокового меню ==========

    clickCatcher = new QWidget(this);
    clickCatcher->setStyleSheet("background-color: rgba(0, 0, 0, 60);"); // затемнення
    clickCatcher->hide();
    clickCatcher->installEventFilter(this); // оброблятиме кліки

    // У стилі — зробимо прозорим
    //clickCatcher->setStyleSheet("background-color: rgba(0,0,0,0);");

    clickCatcher->installEventFilter(this); // Обробка кліка на цей віджет


    // ========== Side Menu ==========

    sideMenu = new QFrame(this);
    sideMenu->setFixedWidth(200);
    sideMenu->setStyleSheet("background-color: #f0f0f0; color: white; border: 1px solid #555;");
    sideMenu->move(-200, 0);  // спочатку приховано зліва
    sideMenu->setParent(this);
    sideMenu->raise();

    QVBoxLayout *sideLayout = new QVBoxLayout(sideMenu);

    // Заголовок бокового меню
    QLabel *menuTitleLabel = new QLabel("Оберіть вкладку:", sideMenu);
    menuTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #000; border: none;");

    // Кнопка закриття
    QPushButton *btnCloseMenu = new QPushButton("✕", sideMenu);
    btnCloseMenu->setFixedSize(16, 16);
    btnCloseMenu->setStyleSheet("border: none; font-weight: bold; color: #000;");

    // Горизонтальний лейаут заголовка
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->addWidget(menuTitleLabel);
    headerLayout->addStretch();               // Відштовхує кнопку вправо
    headerLayout->addWidget(btnCloseMenu);

    // Додаємо його до основного вертикального лейауту меню
    sideLayout->addLayout(headerLayout);

    //QPushButton *btnAssignedByMe = new QPushButton("Поставлені задачі", sideMenu);
    //QPushButton *btnAssignedToMe = new QPushButton("Отримані задачі", sideMenu);

    QPushButton *btnAssignedByMe = createStyledButton("Поставлені задачі");
    QPushButton *btnAssignedToMe = createStyledButton("Отримані задачі");

    btnAssignedByMe->setFixedHeight(25);
    btnAssignedToMe->setFixedHeight(25);

    //btnAssignedByMe->setStyleSheet("background-color: #bbb; color: #000; font-weight: bold; padding-right: 10px;");
    //btnAssignedToMe->setStyleSheet("background-color: #bbb; color: #000; font-weight: bold; padding-right: 10px;");
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

    // Обробка натискання ✕
    connect(btnCloseMenu, &QPushButton::clicked, this, &MainWindow::toggleSideMenu);

    //Додаємо все в layout
    layout->addWidget(topPanel, 9);
    layout->addWidget(viewTypeLabel, 1);
    layout->addWidget(scrollArea, 90);

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

    // Ініціалізація таймера
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, [=]() {
        loadTasks(currentView); // повторне завантаження задач
    });
    refreshTimer->start(2000);


}

MainWindow::~MainWindow() {}

QFrame* MainWindow::createStylizedFrame(QWidget *parent) {
    QFrame *frame = new QFrame(parent);
    frame->setStyleSheet(
        "QFrame {"
        "    background-color: #f0f0f0;"
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

QPushButton* MainWindow::createStyledButton(const QString &text) {
    QPushButton *button = new QPushButton(text);

    // Встановлення стилю для кнопки
    button->setStyleSheet(
        "QPushButton {"
        "    background-color: #eaeaea;"
        "    color: black;"
        "    border: none;"
        "    border: 1px solid #654b25;"
        "    border-radius: 5px;"
        "    padding: 4px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #dadada;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c9c9c9;"
        "}"
        );


    // Встановлення параметрів тексту та іконки
    button->setText(text);
    //button->setLayoutDirection(Qt::AlignCenter);
    //button->setToolTip(text);

    return button;
}

void MainWindow::openCreateTaskDialog() {
    CreateTaskDialog dialog(currentUserId, this);
    if (dialog.exec() == QDialog::Accepted) {
        Task newTask = dialog.getTask();
        if (DatabaseManager::instance().createTask(newTask)) {
            loadTasks(currentView);  // Оновити список
        }
    }
}

void MainWindow::toggleSideMenu() {
    int startX = sideMenu->x();
    int endX = (startX < 0) ? 0 : -200; // Показати або сховати
    menuAnimation->stop();
    menuAnimation->setStartValue(QPoint(startX, 0));
    menuAnimation->setEndValue(QPoint(endX, 0));
    menuAnimation->start();

    if (endX == 0) {
        // Меню відкривається
        clickCatcher->setGeometry(this->rect());
        clickCatcher->raise();
        sideMenu->raise(); // щоб було поверх
        clickCatcher->show();
    } else {
        // Меню закривається
        clickCatcher->hide();
    }
}

void MainWindow::loadTasks(TaskViewType type) {
    currentView = type;

    if (currentView == TaskViewType::AssignedByMe) viewTypeLabel->setText("Поставлені задачі");
    else if (currentView == TaskViewType::AssignedToMe) viewTypeLabel->setText("Отримані задачі");
    else viewTypeLabel->setText("Отримані задачі");

    // Очистка попередніх задач
    QLayoutItem *item;
    while ((item = taskListLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != placeholderLabel) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Завантаження задач
    QList<Task> tasks = DatabaseManager::instance().getTasksByUser(currentUserId, type);

    if (tasks.isEmpty()) {
        placeholderLabel->show();
        taskListLayout->addWidget(placeholderLabel, 0, Qt::AlignCenter);
    } else {
        placeholderLabel->hide();
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

            connect(widget, &TaskWidget::requestPauseRefresh, this, &MainWindow::pauseAutoRefresh);
            connect(widget, &TaskWidget::requestResumeRefresh, this, &MainWindow::resumeAutoRefresh);

            taskListLayout->addWidget(widget);
        }
    }
    taskListLayout->addStretch();
}

void MainWindow::editTask(const Task &task)
{
    CreateTaskDialog *dialog = new CreateTaskDialog(currentUserId, this);
    dialog->loadTaskForEdit(task);

    connect(dialog, &CreateTaskDialog::taskEdited, this, [=](const Task &updatedTask) {
        if (DatabaseManager::instance().updateTask(updatedTask)) {
            qDebug() << "Задача оновлена. ID:" << updatedTask.id;
            loadTasks(currentView);
        } else {
            QMessageBox::warning(this, "Помилка", "Не вдалося оновити задачу.");
        }
    });

    dialog->exec();
}


void MainWindow::deleteTask(int taskId) {
    if (DatabaseManager::instance().deleteTask(taskId)) {
        qDebug() << "Задача видалена з ID:" << taskId;
        loadTasks(currentView);  // Оновити список
    } else {
        QMessageBox::warning(this, "Помилка", "Не вдалося видалити задачу.");
    }
}

void MainWindow::changeTaskStatus(int taskId, const QString &newStatus) {
    if (DatabaseManager::instance().updateTaskStatus(taskId, newStatus)) {
        qDebug() << "Статус задачі оновлено:" << taskId << "→" << newStatus;
        loadTasks(currentView);  // Перезавантажити список
    } else {
        QMessageBox::warning(this, "Помилка", "Не вдалося змінити статус задачі.");
    }
}

void MainWindow::showTaskDetails(const Task &task)
{
    QMessageBox::information(
        this,
        QString("Деталі задачі: %1").arg(task.title),
        task.description.isEmpty() ? "(Опис відсутній)" : task.description
        );
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == clickCatcher && event->type() == QEvent::MouseButtonPress) {
        toggleSideMenu(); // закриваємо меню
        return true; // подія оброблена
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::pauseAutoRefresh() {
    if (refreshTimer->isActive()) {
        refreshTimer->stop();
    }
}

void MainWindow::resumeAutoRefresh() {
    if (!refreshTimer->isActive()) {
        refreshTimer->start(2000); // або твій інтервал
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    sideMenu->setFixedHeight(this->height());
}
