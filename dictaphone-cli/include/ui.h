//
// Created by ntikhonov on 17.11.24.
//
#pragma once

#include <string>
#include <memory>
#include "pulseaudio_device.h"
#include "config.h"


/**
 * Декоратор для работы с файлом конфигурации в UI
 */
struct UIConfigHandler
{
    int selected_input_device = 0;
    int selected_output_device = 0;
    std::shared_ptr<std::string> path_for_records;
    std::string path_to_file;
    std::string shelf_life;
    std::string file_minute_size;

    std::vector<std::pair<unsigned int, portaudio::DeviceInfo>> input_devices;
    std::vector<std::pair<unsigned int, portaudio::DeviceInfo>> output_devices;

    bool save_config(std::string& message);
    [[nodiscard]] std::string selected_input_device_name() const { return config->input_device(); }
    [[nodiscard]] std::string selected_output_device_name() const { return config->output_device(); }
    UIConfigHandler();


private:
    std::shared_ptr<Config> config;
};

/**
 * Класс с методами для удаления записей(файлов) за определенный промежуток времени
 */
struct UIDeleterRecords
{
    std::string selected_delete_value = "30";
    int selected_delete_period = 0;
    std::shared_ptr<std::string> path_records;
    std::vector<std::string> period = {"все время", "часы", "дни", "месяцы"};

    void delete_records(std::string& message) const;
    explicit UIDeleterRecords(const std::shared_ptr<std::string>& path) : path_records(path) {}

private:
    [[nodiscard]] unsigned int delete_by_hour(int hour) const;
    [[nodiscard]] unsigned int delete_by_days(int days) const;
    [[nodiscard]] unsigned int delete_by_month(int month) const;
    [[nodiscard]] unsigned int delete_all_records() const;
};