-- Таблиця користувачів
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    login VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(256) NOT NULL,
    role ENUM('employee', 'manager', 'admin') NOT NULL DEFAULT 'employee',
    supervisor_id INT DEFAULT NULL,
    FOREIGN KEY (supervisor_id) REFERENCES users(id)
);

-- Таблиця задач
CREATE TABLE tasks (
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    creator_id INT NOT NULL,
    assignee_id INT NOT NULL,
    status ENUM('new', 'in_progress', 'done') NOT NULL DEFAULT 'new',
    deadline DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (creator_id) REFERENCES users(id),
    FOREIGN KEY (assignee_id) REFERENCES users(id)
);
