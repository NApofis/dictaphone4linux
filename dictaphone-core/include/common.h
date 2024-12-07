//
// Created by ntikhonov on 20.11.24.
//

#pragma once
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

/**
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

