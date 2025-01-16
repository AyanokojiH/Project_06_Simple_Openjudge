CREATE DATABASE OPENJUDGE;
USE OPENJUDGE;

CREATE TABLE users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL
);

CREATE TABLE questions (
    question_id INT AUTO_INCREMENT PRIMARY KEY,
    question_content TEXT NOT NULL
);

CREATE TABLE user_question_records (
    record_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    question_id INT NOT NULL,
    status ENUM('pass', 'trying') NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id),
    FOREIGN KEY (question_id) REFERENCES questions(question_id),
    UNIQUE (user_id, question_id)
);