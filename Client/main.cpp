#include "mainwindow.h"
#include "loginwindow.h"
#include "databasemanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DatabaseManager::instance().connect("127.0.0.1", 3306, "todo_app", "app_user", "1234");

    LoginWindow loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {

        int userId = loginDialog.getAuthenticatedUserId();

        MainWindow w(userId);

        w.setMinimumSize(600, 450);
        w.setMaximumSize(600, 450);

        w.setWindowTitle("To-Do App");

        w.show();
        return a.exec();
    }

    return 0;
}
