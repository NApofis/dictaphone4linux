//
// Created by ntikhonov on 17.11.24.
//

#pragma once

#include <string>
#include <vector>


namespace portaudio_devices
{
    struct Device
    {
        int id;
        std::string name;
        std::string description;
        bool default_device;

    };

    std::vector<Device> list_input_devices();
    std::vector<Device> list_output_devices();

}
