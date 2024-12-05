//
// Created by ntikhonov on 17.11.24.
//

#pragma once

#include <string>
#include <vector>

std::string execute_command(const std::string& cmd);

/**
* Методы для работы с аудио устройствами через драйвер pulseaudio
*/
namespace portaudio
{
    /*
     * Данные устройства pulseaudio
     */
    struct DeviceInfo
    {
        int id;
        std::string device; // Програмное название
        std::string human_name;
        std::string master; // Родительское устройство
        bool real;
    };

    DeviceInfo* find(std::vector<DeviceInfo>& devices,
        const std::string* device, const std::string* human_name, const bool* real);

    std::vector<DeviceInfo> list_input_devices();
    std::vector<DeviceInfo> list_output_devices();

    bool create_input_device_module(const DeviceInfo& dev);
    bool create_output_device_module(const DeviceInfo& device);

}

