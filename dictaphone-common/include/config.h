//
// Created by ntikhonov on 16.11.24.
//

#pragma once

#include <fstream>
#include <string>
#include <map>
#include <syslog.h>
#include "dictaphone-common/const.h"

const std::string PROGRAM_ROOT_PATH = PROGRAM_ROOT_DIRECTORY; // Папка с файлами пдля программы
const std::string CONFIG_FILE_PATH = PROGRAM_ROOT_PATH+"/config.conf"; // Полный путь до конфигурационного файла
const std::string START_STRING = "# Config file for dictaphone"; // Первая строка конфига
const std::string JOIN_STRING = ": "; // Разделитель ключей и значений в конфиге
const std::string RECORD_FILE_NAME_MASK = "dictaphone_record_"; // Название айдио файла
const std::string RECORD_FILE_NAME_POSTFIX = "%Y-%m-%d_%H-%M"; // Формат времени для названия аудио файла
const std::string RECORD_FILE_NAME_EXTENSION = ".wav"; // Расширение аудио файлов с записями

const std::string NONE_DEVICE = "None"; // Обозначение отсутствия устройства

/**
 * Основной класс для работы с конфигурационным файлом
 */
class Config
{
    friend std::ofstream& operator<<(std::ofstream& os, const Config* rhs);
    friend std::ifstream& operator>>(std::ifstream& is, Config* rhs);

    std::map<std::string, std::string> fields
    {
        {"input device", "None"},
        {"output device", "None"},
        {"path for records", PROGRAM_ROOT_PATH+"/records"},
        {"shelf life", "7"},
        {"file minute size", "30"},
        {"path to music file", ""}
    };

public:
    Config();

    [[nodiscard]] bool save() const;

    std::string input_device() { return fields["input device"]; }
    void input_device(const std::string& val) { fields["input device"] = val; }

    std::string output_device() { return fields["output device"]; }
    void output_device(const std::string& val) { fields["output device"] = val; }

    std::string path_for_records() { return  fields["path for records"]; }
    bool path_for_records(const std::string& val);

    unsigned int shelf_life() { return stoi(fields["shelf life"]); }
    bool shelf_life(unsigned int val);

    unsigned int file_minute_size() { return stoi(fields["file minute size"]); }
    bool file_minute_size(unsigned int val);

};



/**
 * Логер будет сохранять логи работы в файл в папке программы
 */
class Loger
{
public:

    static void info(const std::string& message)
    {
        log(message, LOG_INFO);
    }

    static void warning(const std::string& message)
    {
        log(message, LOG_WARNING);
    }

    static void error(const std::string& message)
    {
        log(message, LOG_ERR);
    }


private:

    static void log(const std::string& message, std::int32_t priority)
    {
        std::ofstream out;
        out.open(PROGRAM_ROOT_PATH+"/log.txt", std::ios::app);
        out << "[" << priority_str(priority) << "] " << message << std::endl;
        out.close();
    }

    static std::string priority_str(std::int32_t priority)
    {
        switch (priority)
        {
        case LOG_ERR: return "error";
        case LOG_WARNING: return "warning";
        case LOG_INFO: return "info";
        default: return "unknown_priority";
        }
    }
};