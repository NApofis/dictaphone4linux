//
// Created by ntikhonov on 20.11.24.
//

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <portaudio.h>


using place_method = void*(*)(const std::string& name, unsigned long size);

class DeviceHandler
{
    std::string device_name;
    std::shared_ptr<std::atomic_bool> stop_flag;
    place_method get_place;
    void callback(const void* input_data, void*, unsigned long frames_per_Buffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) const;

public:
    DeviceHandler(std::string  device, const place_method m, std::shared_ptr<std::atomic_bool> flag) :
        device_name(std::move(device)),
        get_place(m),
        stop_flag(std::move(flag))
    {}

    void connect();

};



