//
// Created by nik on 25.11.24.
//

#include "daemon_command.h"
#include "pulseaudio_device.h"
#include "config.h"

namespace daemon_command
{
    bool is_work()
    {
        auto status =  execute_command("dictaphone4linux-core --status");
        return status.find(NONE_DEVICE) == std::string::npos;
    }

    void start()
    {
        execute_command("dictaphone4linux-core --start");
    }


    void stop()
    {
        execute_command("dictaphone4linux-core --stop");
    }
}