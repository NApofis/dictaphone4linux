//
// Created by ntikhonov on 20.11.24.
//

#pragma once
#include <syslog.h>
#include <list>
#include <vector>

#include "config.h"
#include "portaudio.h"

const std::string DAEMON_NAME = "dictaphone4linux"; // Название демона и виртуальных устройств
constexpr size_t BUFFERS_COUNT = 20; // Количество буфферов для накапливания звука. Каждый буффер будет вмещать примерно 1 секунду звука
constexpr size_t SAMPLE_RATE = 44100; // Частота дискретизации
constexpr size_t NUM_CHANNELS = 1; // Количество каналоз записи
constexpr size_t CHUNK_BUFFER_SIZE = 512; // Часть буффера которая будет накоплена и записана в буффер

using SAMPLE_TYPE = short; // Качество звука
constexpr unsigned long SAMPLE_VAL = []() constexpr -> unsigned long
{
    if(std::is_same_v<SAMPLE_TYPE, int>)
    {
        return paInt32;
    }
    else if(std::is_same_v<SAMPLE_TYPE, short>)
    {
        return paInt16;
    }
    else
    {
        return paFloat32;
    }
}();

/*
 * Статусы устройства
 */
enum class Status
{
    created, // новое устройство
    unchanged, // данные устройства не изменились
    brocken // соединение с устройством было петерено
};

using dev_status = std::pair< Status, std::string >;
using record_sample_data = std::shared_ptr<std::vector<SAMPLE_TYPE>>;
using device_sample = std::pair<SAMPLE_TYPE*, size_t>;
using records_storage = std::shared_ptr<std::list<record_sample_data>>;

/*
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
