# Game Tracker

Приложение для отслеживания прогресса в играх с графическим интерфейсом и базой данных SQLite.

## Возможности
- Добавление, редактирование и удаление игр
- Отслеживание прогресса (%) и времени (часы)
- Статусы: Не начата, Играю, Пройдена, Заброшена
- Автосохранение в базу данных SQLite
- Валидация вводимых данных

## Технологии
- C++17
- SDL2 + SDL2_image
- Dear ImGui
- SQLite3
- CMake

## Сборка и запуск

### Требования
- Windows 10/11
- Visual Studio 2022 с компонентом "C++ Desktop Development"
- CMake 3.15+

### Установка зависимостей

1. **SDL2**: Скачать [SDL2-devel-2.30.0-VC.zip](https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-devel-2.30.0-VC.zip) и распаковать в `C:\SDL2`

2. **SDL2_image**: Скачать [SDL2_image-devel-2.8.0-VC.zip](https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.8.0-VC.zip) и распаковать в `C:\SDL2_image`

### Сборка
1. Открой Visual Studio 2022
2. **Файл → Открыть → Папка** → выбери папку проекта
3. Дождись автоматической настройки CMake
4. Нажми **F5** для запуска

## Структура проекта
- `main.cpp` — точка входа, графический интерфейс (ImGui)
- `database.h/cpp` — класс для работы с базой данных
- `sqlite3.c/h` — библиотека SQLite
- `imgui/` — библиотека Dear ImGui
- `CMakeLists.txt` — конфигурация сборки

## Автор / группа
Хусаинов Игорь, П2-24