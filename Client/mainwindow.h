#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSlider>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QScrollArea>
#include <QPropertyAnimation>

#include "taskviewtype.h"
#include "task.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int userId, QWidget *parent = nullptr);
    ~MainWindow();
    QFrame *createStylizedFrame(QWidget *parent);
    QPushButton* createStyledButton(const QString &text);

private:
    int currentUserId;

    QFrame *sideMenu;
    QPropertyAnimation *menuAnimation;

    QWidget *scrollWidget;
    QVBoxLayout *taskListLayout;

    TaskViewType currentView;

    void toggleSideMenu();
    void loadTasks(TaskViewType type);
    void openCreateTaskDialog();

    QWidget *clickCatcher = nullptr;

    QTimer *refreshTimer;

    void pauseAutoRefresh();
    void resumeAutoRefresh();

    QLabel *viewTypeLabel;
    QLabel *placeholderLabel;

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void editTask(const Task &task);
    void deleteTask(int taskId);
    void changeTaskStatus(int taskId, const QString &newStatus);
    void showTaskDetails(const Task &task);

};
#endif // MAINWINDOW_H
