//
// Created by ntikhonov on 20.11.24.
//

#pragma once
#include <list>
#include <string>
#include <memory>
#include <vector>
#include <thread>

#include "config_handler.h"
#include "device_handler.h"
#include "common.h"


struct Data
{
    std::shared_ptr<std::mutex> storage_records_lock;
    records_storage storage_records;

    std::string path_for_records;

    dev_status input_device = {Status::unchanged, ""};
    dev_status output_device = {Status::unchanged, ""};

};

class iChain {

protected:
    Data* data;
    std::unique_ptr<iChain> next = nullptr;

    void next_execute_if_exists()
    {
        if (next)
        {
            return next->execute();
        }
    }

public:
    iChain(Data* d, std::unique_ptr<iChain>& chain): data(d), next(chain.release()) {};
    explicit iChain(Data* d): data(d) {};

    virtual void execute() = 0;
    virtual void cancel()
    {
        if (next)
        {
            return next->execute();
        }
    };

    virtual ~iChain() = default;
};

class ConfigController : public iChain
{
    CoreConfigHandler config_handler;
    std::string saved_input_device_name, saved_output_device_name;
    void process_device(bool is_input_dev, dev_status& status) const;

public:
    using iChain::iChain;
    void execute() override;
};

class DeviceController : public iChain
{

    struct DeviceHandlerInfo
    {
        std::thread thread;
        std::shared_ptr<std::atomic_bool> stop_flag{false};
        bool used = false;
        std::shared_ptr<DeviceHandler> obj;
    };

    DeviceHandlerInfo input_device;
    DeviceHandlerInfo output_device;

    void verification(dev_status& new_dev, DeviceHandlerInfo& dev_info);

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override ;
};