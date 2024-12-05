//
// Created by ntikhonov on 05.12.24.
//

#include "controllers/config.h"

#include "common.h"
#include <iostream>


void ConfigController::process_device(const bool is_input_dev, dev_status& status)
{
    std::string name;
    std::string* saved;
    if(is_input_dev)
    {
        name = config_handler->get_input_device_module(nullptr);
        saved = &saved_input_device_name;
    }
    else
    {
        name = config_handler->get_output_device_module();
        saved = &saved_output_device_name;
    }

    if(name != *saved)
    {
        status.second = name;
        *saved = name;
        status.first = Status::created;
    }
    else if(status.first == Status::brocken)
    {
        status.first = Status::created;
    }
}


void ConfigController::execute()
{
    bool is_new;
    if(!config_handler)
    {
        config_handler = std::make_shared<CoreConfigHandler>();
        is_new = true;
    }
    else
    {
        is_new = config_handler->load_new_version();
    }

    config_handler->delete_old_records();

    if(is_new)
    {
        data->path_for_records = config_handler->get_path_for_records();
        data->minute_size = config_handler->get_file_minute_size();
    }
    if(is_new || data->input_device.first == Status::brocken)
    {
        process_device(true, data->input_device);
    }

    if(is_new || data->output_device.first == Status::brocken)
    {
        process_device(false, data->output_device);
    }
    return next_execute_if_exists();
}

