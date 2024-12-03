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
        return execute_command("dictaphone-core --status") != NONE_DEVICE;
    }

    void start()
    {
        execute_command("dictaphone-core --start");
    }


    void stop()
    {
        execute_command("dictaphone-core --stop");
    }
}