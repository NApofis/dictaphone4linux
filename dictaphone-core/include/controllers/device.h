//
// Created by ntikhonov on 05.12.24.
//

#pragma once

#include <thread>

#include "controllers/icontroller.h"

/**
 * Элемент цепочки запускающий запись звука с аудио устройств
 */
class DeviceController : public iChain
{

    struct DeviceHandlerInfo
    {
        std::string name;
        std::thread thread;
        std::shared_ptr<std::atomic_bool> stop_flag = std::make_shared<std::atomic_bool>(false);
    };

    DeviceHandlerInfo input_device;
    DeviceHandlerInfo output_device;

    bool initialized = false;

    std::shared_ptr<Mixer> mix = nullptr;

    void connect_device(dev_status& new_dev, DeviceHandlerInfo& dev_info);
    void device_disconnect(DeviceHandlerInfo& dev_info);
    bool active_device() const;

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override;
};

