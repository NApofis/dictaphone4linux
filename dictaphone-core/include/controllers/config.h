//
// Created by ntikhonov on 05.12.24.
//

#pragma once

#include "controllers/icontroller.h"

/**
 * Элемент цепочки отслеживающий изменения в конфигурационном файле
 */
class ConfigController : public iChain
{
    std::shared_ptr<CoreConfigHandler> config_handler;
    std::string saved_input_device_name, saved_output_device_name;
    void process_device(bool is_input_dev, dev_status& status);

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override { next_cancel_if_exists(); };
};

