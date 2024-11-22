//
// Created by ntikhonov on 20.11.24.
//

#pragma once
#include "config.h"

#include <syslog.h>

const std::string DAEMON_NAME = "dictaphone4linux";

enum class Status
{
    created, unchanged, brocken
};

// Состоянеие устройства
// название устройства
using dev_status = std::pair< Status, std::string >;
using records_storage = std::shared_ptr<std::list<std::shared_ptr<std::vector<char>>>>;

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
        out.open("/home/nik/test.txt", std::ios::app);
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
