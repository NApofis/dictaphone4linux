//
// Created by ntikhonov on 20.11.24.
//

#include <iostream>
#include "device_handler.h"

#include <cstring>

void DeviceHandler::connect()
{
    std::cout << "Connecting to device..." << std::endl;
    callback(nullptr, nullptr, 0, {}, {}, nullptr);
}

void DeviceHandler::callback(const void* input_data, void*, const unsigned long frames_per_Buffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) const
{



}

