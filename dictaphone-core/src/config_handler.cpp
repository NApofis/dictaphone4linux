//
// Created by nik on 21.11.24.
//

#include <chrono>
#include <portaudio.h>

#include "config_handler.h"
#include "records.h"
#include "common.h"


std::string CoreConfigHandler::get_path_for_records() const
{
    auto result = config->path_for_records();
    if(result.empty() || !std::filesystem::exists(result))
    {
        return {};
    }
    return result;
}

void CoreConfigHandler::delete_old_records() const
{
    auto days = config->shelf_life();
    if(days == 0)
    {
        // Срок хранения == 0 => храним всегда
        return;
    }
    auto path = get_path_for_records();
    if(path.empty())
    {
        Loger::error("Папка для хранения записей отсутствует");
        return;
    }

    auto now = std::chrono::system_clock::now();
    now -= std::chrono::days(days);
    auto result = records::delete_records_before(path, &now);
    Loger::info("Удалено " + std::to_string(result) + " записей");
}

std::string CoreConfigHandler::receive_input_device(const pulseaudio::DeviceInfo& other)
{
    std::string my_dev_human_name = other.human_name + " " + DAEMON_NAME;
    auto one_of_vector = check_portaudio_device({other.human_name, my_dev_human_name});
    if(!one_of_vector.empty())
    {
        return one_of_vector;
    }

    pulseaudio::create_input_device_module({0, DAEMON_NAME, my_dev_human_name, &other});
    one_of_vector = check_portaudio_device({my_dev_human_name});
    if(!one_of_vector.empty())
    {
        return one_of_vector;
    }
    Loger::error("Ошибка при создании модуля для устройства ввода " + my_dev_human_name);
    return {};
}


std::string CoreConfigHandler::get_input_device_module() const
{
    auto device_list = pulseaudio::list_input_devices();
    decltype(device_list)::const_iterator iter;
    bool found = false;
    for(auto i = device_list.cbegin(); i != device_list.cend(); i++)
    {
        if(i->device == config->input_device())
        {
            found = true;
            iter = i;
            break;
        }
    }
    if(!found)
    {
        Loger::error("Выбранное устройстро ввода не найдено");
        return {};
    }
    return receive_input_device(*iter);
}


std::string CoreConfigHandler::get_output_device_module() const
{
    auto device_list = pulseaudio::list_output_devices();
    decltype(device_list)::const_iterator iter;

    bool found = false;
    for(auto i = device_list.cbegin(); i != device_list.cend(); i++)
    {
        if(i->device == config->input_device())
        {
            found = true;
            iter = i;
            break;
        }
    }
    if(!found)
    {
        Loger::error("Выбранное устройстро ввода не найдено");
        return {};
    }

    auto combiner_device = DAEMON_NAME + " combiner";
    auto combiner_human_name = iter->human_name + " combiner";
    auto one_of_vector = check_portaudio_device({combiner_human_name});
    if(!one_of_vector.empty())
    {
        return receive_input_device({0, combiner_device + ".monitor", combiner_human_name});
    }

    pulseaudio::create_output_device_module({0, DAEMON_NAME + " combiner", combiner_human_name, &*iter});
    one_of_vector = check_portaudio_device({combiner_human_name});
    if(!one_of_vector.empty())
    {
        return receive_input_device({0, combiner_device + ".monitor", combiner_human_name});
    }
    Loger::error("Ошибка при создании модуля для устройства вывода " + combiner_human_name);
    return {};
}


std::string CoreConfigHandler::check_portaudio_device(const std::vector<std::string>& names)
{
    freopen("/dev/null","w",stderr);
    PaError err = Pa_Initialize();
    freopen("/dev/tty","w",stderr);
    if (err != paNoError) {
        Loger::error("Не удалось получить список устройств portaudio");
        return {};
    }

    for (int i = 0; i < Pa_GetDeviceCount(); i++)
    {
        auto deviceInfo = Pa_GetDeviceInfo(i);
        for(const auto& name : names)
        {
            if (deviceInfo->name == name)
            {
                return name;
            }
        }
    }
    return {};
}

bool CoreConfigHandler::load_new_version()
{
    auto write_time = std::filesystem::last_write_time(CONFIG_FILE_PATH);
    if(write_time > last_check)
    {
        last_check = write_time;
        config = std::make_shared<Config>();
        return true;
    }
    return false;
}
