//
// Created by ntikhonov on 20.11.24.
//

#pragma once
#include <syslog.h>
#include <list>
#include <vector>

#include "config.h"
#include "portaudio.h"

const std::string DAEMON_NAME = "dictaphone4linux";
constexpr size_t BUFFERS_COUNT = 20;
constexpr size_t SAMPLE_RATE = 44100;
constexpr size_t NUM_CHANNELS = 1;
constexpr size_t CHUNK_BUFFER_SIZE = 512; // 1/25

using SAMPLE_TYPE = short;
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

enum class Status
{
    created, unchanged, brocken
};

// Состоянеие устройства
// название устройства
using dev_status = std::pair< Status, std::string >;
using record_sample_data = std::shared_ptr<std::vector<SAMPLE_TYPE>>;
using device_sample = std::pair<SAMPLE_TYPE*, size_t>;
using records_storage = std::shared_ptr<std::list<record_sample_data>>;

class Loger
{
public:
    static void init()
    {
        openlog(DAEMON_NAME.c_str(), LOG_PID, LOG_DAEMON);
    }

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

    static void shutdown()
    {
        closelog();
    }

private:

    static void log(const std::string& message, std::int32_t priority)
    {
        syslog(priority, "%s", message.c_str());
        std::ofstream out;
        out.open(PROGRAM_ROOT_PATH+"/log.txt", std::ios::app);
        out << message << std::endl;
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
