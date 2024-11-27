//
// Created by nik on 21.11.24.
//

#pragma once
#include <memory>
#include <string>
#include <bits/stl_vector.h>
#include <filesystem>

#include "pulseaudio_device.h"
#include "config.h"

struct CoreConfigHandler
{

    [[nodiscard]] std::string get_input_device_module() const;
    [[nodiscard]] std::string get_output_device_module() const;
    [[nodiscard]] std::string get_path_for_records() const;
    [[nodiscard]] unsigned int get_file_minute_size() const { return config->file_minute_size(); }

    void delete_old_records() const;
    bool load_new_version();

    CoreConfigHandler()
    {
        last_check = std::filesystem::last_write_time(CONFIG_FILE_PATH);
        config = std::make_shared<Config>();
    };

private:

    std::shared_ptr<Config> config;
    std::chrono::time_point<std::chrono::file_clock> last_check;

    [[nodiscard]] static std::string receive_input_device(const pulseaudio::DeviceInfo& other);
    [[nodiscard]] static std::string check_portaudio_device(const std::vector<std::string>& names) ;
};