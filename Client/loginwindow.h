#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    QString getLogin() const;
    int getAuthenticatedUserId() const;

private slots:
    void tryLogin();

private:
    int authenticatedUserId = -1;

    QLineEdit *loginField;
    QLineEdit *passwordField;
    QPushButton *loginButton;
    QLabel *errorLabel;
    QString login;
};

#endif // LOGINWINDOW_H
