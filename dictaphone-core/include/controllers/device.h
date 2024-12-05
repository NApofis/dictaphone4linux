//
// Created by ntikhonov on 05.12.24.
//

#pragma once

#include <thread>

#include "controllers/icontroller.h"

/*
 * Элемент цепочки запускающий запись звука с аудио устройств
 */
class DeviceController : public iChain
{

    struct DeviceHandlerInfo
    {
        std::string name;
        std::thread thread;
        std::shared_ptr<std::atomic_bool> stop_flag;
    };

    DeviceHandlerInfo input_device;
    DeviceHandlerInfo output_device;

    std::shared_ptr<Mixer> mix = nullptr;

    void verification(dev_status& new_dev, DeviceHandlerInfo& dev_info) const;

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override;
};

