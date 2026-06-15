#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

// Структура для хранения данных об игре
struct Game {
    int id;
    std::string title;
    std::string status;
    int progress_percent;
    double hours_played;
    std::string last_updated;
};

// Класс для работы с базой данных
class Database {
public:
    Database();
    ~Database();

    // Инициализация БД (создание таблицы)
    bool init();

    // Добавление новой игры
    bool addGame(const std::string& title, const std::string& status);

    // Получение всех игр
    std::vector<Game> getAllGames();

    // Обновление прогресса игры
    bool updateProgress(int id, int progress, double hours);

    // Изменение статуса игры
    bool updateStatus(int id, const std::string& newStatus);

    // Удаление игры
    bool deleteGame(int id);

    // Обновление всех полей игры
    bool updateGame(int id, const std::string& title, const std::string& status,
        int progress, double hours);

private:
    void* db; // Указатель на SQLite базу данных
};

#endif