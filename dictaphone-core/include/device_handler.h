//
// Created by ntikhonov on 20.11.24.
//

#pragma once

#include <atomic>
#include <memory>
#include <mixer.h>
#include <string>
#include <portaudio.h>

namespace pulseaudio
{
    struct StreamData
    {
        std::string name;
        unsigned int buffer_id = 0;
        std::shared_ptr<Keeper> keep;
        device_sample data = {nullptr, 0};
        device_sample::first_type data_ptr = nullptr;
        unsigned int data_id = 0;
    };

    void connect(const std::string& device, const std::shared_ptr<Keeper>& keeper, const std::shared_ptr<std::atomic_bool>& flag);
    static int callback(const void* input_data, void*, unsigned long frames_per_Buffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* user_data);
}



