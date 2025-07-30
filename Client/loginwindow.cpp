#include "loginwindow.h"
#include "databasemanager.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Вхід");
    resize(200, 100);
    //this->setMaximumHeight(30);

    QVBoxLayout *layout = new QVBoxLayout(this);

    loginField = new QLineEdit(this);
    loginField->setPlaceholderText("Логін");

    passwordField = new QLineEdit(this);
    passwordField->setPlaceholderText("Пароль");
    passwordField->setEchoMode(QLineEdit::Password);

    loginButton = new QPushButton("Увійти", this);
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red");

    layout->addWidget(loginField);
    layout->addWidget(passwordField);
    layout->addWidget(loginButton);
    layout->addWidget(errorLabel);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::tryLogin);
}

QString LoginWindow::getLogin() const {
    return login;
}

void LoginWindow::tryLogin()
{
    QString enteredLogin = loginField->text();
    QString enteredPassword = passwordField->text();

    auto result = DatabaseManager::instance().authenticate(enteredLogin, enteredPassword);
    if (result.has_value()) {
        authenticatedUserId = result.value();
        accept(); // закриває вікно та повертає Accepted
    } else {
        //QMessageBox::warning(this, "Помилка", "Неправильний логін або пароль");
        errorLabel->setText("Неправильний логін або пароль");
    }
}

int LoginWindow::getAuthenticatedUserId() const {
    return authenticatedUserId;
}
