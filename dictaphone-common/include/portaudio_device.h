//
// Created by ntikhonov on 17.11.24.
//

#pragma once

#include <string>
#include <vector>


namespace portaudio
{
    struct DeviceInfo
    {
        int id;
        std::string device;
        std::string human_name;
        const DeviceInfo* ref_device;
    };

    std::vector<DeviceInfo> list_input_devices();
    std::vector<DeviceInfo> list_output_devices();

    bool create_input_device_module(const DeviceInfo& dev);
    bool create_output_device_module(const DeviceInfo& device);

}
