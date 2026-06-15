#include <SDL.h>
#include <SDL_image.h>
#include <windows.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "database.h"
#include <string>
#include <vector>
#include <cstdio>

void DebugLog(const char* msg) {
    OutputDebugStringA(msg);
}

void DebugLogFormatted(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OutputDebugStringA(buffer);
}

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        DebugLog("SDL_Init error: ");
        DebugLog(SDL_GetError());
        DebugLog("\n");
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Game Tracker",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window) {
        DebugLog("SDL_CreateWindow error: ");
        DebugLog(SDL_GetError());
        DebugLog("\n");
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        DebugLog("SDL_CreateRenderer error: ");
        DebugLog(IMG_GetError());
        DebugLog("\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Загружаем шрифт с поддержкой кириллицы
    static const ImWchar glyph_ranges[] = {
        0x0020, 0x00FF,
        0x0400, 0x044F,
        0,
    };
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 16.0f, nullptr, glyph_ranges);

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    Database db;
    if (!db.init()) {
        DebugLog("Database init error!\n");
        return 1;
    }
    DebugLog("Database initialized successfully!\n");

    // Переменные для добавления игры
    bool show_add_window = false;
    static char input_title[256] = "";
    static int input_status = 0;
    static int input_progress = 0;
    static float input_hours = 0.0f;
    static int selected_game_id = -1;
    static char add_error_message[256] = "";

    // Переменные для редактирования
    bool show_edit_window = false;
    static int edit_game_id = -1;
    static char edit_title[256] = "";
    static int edit_status = 0;
    static int edit_progress = 0;
    static float edit_hours = 0.0f;
    static char edit_error_message[256] = "";

    bool running = true;

    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
        ImGui::Begin("Game Tracker");

        ImGui::Text("Управление играми");
        ImGui::Separator();

        if (ImGui::Button("Добавить игру", ImVec2(150, 30))) {
            show_add_window = true;
            input_title[0] = '\0';
            input_status = 0;
            input_progress = 0;
            input_hours = 0.0f;
            add_error_message[0] = '\0';
        }

        ImGui::SameLine();
        if (selected_game_id != -1 && ImGui::Button("Удалить выбранную", ImVec2(150, 30))) {
            if (db.deleteGame(selected_game_id)) {
                selected_game_id = -1;
                DebugLog("Game deleted successfully\n");
            }
        }

        ImGui::SameLine();
        if (selected_game_id != -1 && ImGui::Button("Редактировать", ImVec2(150, 30))) {
            // Заполняем поля для редактирования
            auto games = db.getAllGames();
            for (const auto& game : games) {
                if (game.id == selected_game_id) {
                    edit_game_id = game.id;
                    snprintf(edit_title, sizeof(edit_title), "%s", game.title.c_str());
                    edit_progress = game.progress_percent;
                    edit_hours = static_cast<float>(game.hours_played);

                    // Находим индекс статуса
                    const char* statuses[] = { "Не начата", "Играю", "Пройдена", "Заброшена" };
                    edit_status = 0;
                    for (int i = 0; i < 4; i++) {
                        if (game.status == statuses[i]) {
                            edit_status = i;
                            break;
                        }
                    }

                    edit_error_message[0] = '\0';
                    show_edit_window = true;
                    break;
                }
            }
        }

        ImGui::Separator();

        // Получаем все игры из БД
        auto games = db.getAllGames();

        DebugLogFormatted("Total games in database: %d\n", (int)games.size());

        ImGui::Text("Всего игр: %d", (int)games.size());
        ImGui::Separator();

        // Таблица игр
        if (ImGui::BeginTable("GamesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Название", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Статус", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableSetupColumn("Прогресс", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableSetupColumn("Часы", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();

            for (const auto& game : games) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", game.id);

                ImGui::TableSetColumnIndex(1);
                if (ImGui::Selectable(game.title.c_str(), selected_game_id == game.id,
                    ImGuiSelectableFlags_SpanAllColumns)) {
                    selected_game_id = game.id;
                    DebugLogFormatted("Selected game ID: %d\n", selected_game_id);
                }

                ImGui::TableSetColumnIndex(2);
                ImVec4 status_color;
                if (game.status == "Пройдена") status_color = ImVec4(0, 1, 0, 1);
                else if (game.status == "Заброшена") status_color = ImVec4(1, 0, 0, 1);
                else if (game.status == "Играю") status_color = ImVec4(1, 1, 0, 1);
                else status_color = ImVec4(1, 1, 1, 1);
                ImGui::TextColored(status_color, "%s", game.status.c_str());

                ImGui::TableSetColumnIndex(3);
                char progress_text[32];
                snprintf(progress_text, sizeof(progress_text), "%d%%", game.progress_percent);
                ImGui::ProgressBar(game.progress_percent / 100.0f, ImVec2(100, 0), progress_text);

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.1f", game.hours_played);
            }
            ImGui::EndTable();
        }

        ImGui::End();

        // Окно добавления игры
        if (show_add_window) {
            ImGui::OpenPopup("Добавить игру");
            show_add_window = false;
        }

        if (ImGui::BeginPopupModal("Добавить игру", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Название игры##title", input_title, 256);

            const char* statuses[] = { "Не начата", "Играю", "Пройдена", "Заброшена" };
            ImGui::Combo("Статус##status", &input_status, statuses, 4);

            ImGui::SliderInt("Прогресс (0-100%)##progress", &input_progress, 0, 100);
            ImGui::InputFloat("Часов сыграно##hours", &input_hours, 0.0f, 0.0f, "%.1f");
            if (input_hours < 0.0f) {
                snprintf(add_error_message, sizeof(add_error_message), "Ошибка: количество часов не может быть отрицательным!");
            }
            else {
                add_error_message[0] = '\0';
            }

            // Показываем ошибку если есть
            if (add_error_message[0] != '\0') {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", add_error_message);
                ImGui::Separator();
            }

            if (ImGui::Button("Сохранить", ImVec2(100, 30))) {
                DebugLogFormatted("Save clicked! Title: '%s'\n", input_title);

                if (strlen(input_title) > 0 && input_hours >= 0.0f) {
                    DebugLogFormatted("Calling db.addGame with title='%s', status='%s'\n",
                        input_title, statuses[input_status]);

                    bool success = db.addGame(input_title, statuses[input_status]);
                    DebugLogFormatted("addGame result: %s\n", success ? "SUCCESS" : "FAILED");

                    if (success) {
                        // Обновляем прогресс для последней добавленной игры
                        auto all_games = db.getAllGames();
                        if (!all_games.empty()) {
                            int last_id = all_games.back().id;
                            DebugLogFormatted("Updating progress for game ID %d\n", last_id);
                            db.updateProgress(last_id, input_progress, input_hours);
                            DebugLogFormatted("Progress updated for game ID %d\n", last_id);
                        }
                    }
                }
                else {
                    DebugLog("Title is empty or hours are negative!\n");
                }

                input_title[0] = '\0';
                input_status = 0;
                input_progress = 0;
                input_hours = 0.0f;
                add_error_message[0] = '\0';

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Отмена", ImVec2(100, 30))) {
                input_title[0] = '\0';
                input_status = 0;
                input_progress = 0;
                input_hours = 0.0f;
                add_error_message[0] = '\0';
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        // Окно редактирования игры
        if (show_edit_window) {
            ImGui::OpenPopup("Редактировать игру");
            show_edit_window = false;
        }

        if (ImGui::BeginPopupModal("Редактировать игру", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Название игры##edit_title", edit_title, 256);

            const char* statuses[] = { "Не начата", "Играю", "Пройдена", "Заброшена" };
            ImGui::Combo("Статус##edit_status", &edit_status, statuses, 4);

            ImGui::SliderInt("Прогресс (0-100%)##edit_progress", &edit_progress, 0, 100);
            ImGui::InputFloat("Часов сыграно##edit_hours", &edit_hours, 0.0f, 0.0f, "%.1f");
            if (edit_hours < 0.0f) {
                snprintf(edit_error_message, sizeof(edit_error_message), "Ошибка: количество часов не может быть отрицательным!");
            }
            else {
                edit_error_message[0] = '\0';
            }

            // Показываем ошибку если есть
            if (edit_error_message[0] != '\0') {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", edit_error_message);
                ImGui::Separator();
            }

            if (ImGui::Button("Сохранить", ImVec2(100, 30))) {
                if (strlen(edit_title) > 0 && edit_hours >= 0.0f) {
                    if (db.updateGame(edit_game_id, edit_title, statuses[edit_status],
                        edit_progress, edit_hours)) {
                        DebugLog("Game updated successfully\n");
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Отмена", ImVec2(100, 30))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::Render();
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    DebugLog("Shutting down...\n");

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}