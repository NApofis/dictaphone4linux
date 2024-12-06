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
    const auto result = records::delete_records_before(path, &now);
    if(result > 0)
    {
        Loger::info("Удалено " + std::to_string(result) + " записей");
    }
}

std::string CoreConfigHandler::get_input_device_module(portaudio::DeviceInfo* ptr) const
{
    auto device_list = portaudio::list_input_devices();
    std::string dev_name;
    if(!ptr)
    {
        const auto device = config->input_device();
        if(device == NONE_DEVICE)
        {
            Loger::info("Устройство ввода не выбрано");
            return {};
        }

        ptr = portaudio::find(device_list, &device, nullptr, nullptr);
        if(!ptr)
        {
            Loger::error("Выбранное устройство ввода не найдено");
            return {};
        }
        ptr->master = ptr->device;
        ptr->device = DAEMON_NAME;
        ptr->human_name = DAEMON_NAME + " " + ptr->human_name;
    }


    constexpr bool real = false;
    if(!portaudio::find(device_list, &ptr->device, &ptr->human_name, &real))
    {
        if(!portaudio::create_input_device_module(*ptr))
        {
            Loger::error("Ошибка при создании виртуального устройства3 " + ptr->human_name);
            return {};
        }
    }

    if(check_portaudio_device(ptr->human_name))
    {
        return ptr->human_name;
    }
    Loger::error("Ошибка при создании виртуального устройства2 " + ptr->human_name);
    return {};
}


std::string CoreConfigHandler::get_output_device_module() const
{
    auto device_list = portaudio::list_output_devices();
    const auto device = config->output_device();
    if(device == NONE_DEVICE)
    {
        Loger::info("Устройство выводе не выбрано");
        return {};
    }
    const auto ptr = portaudio::find(device_list, &device, nullptr, nullptr);
    if(!ptr)
    {
        Loger::error("Выбранное устройство выводе не найдено");
        return {};
    }

    const auto combiner_device = DAEMON_NAME + ".combiner";
    const auto combiner_human_name = DAEMON_NAME + " " + ptr->human_name + " combiner";

    constexpr bool real = false;
    if(!portaudio::find(device_list, &combiner_device, &combiner_human_name, &real))
    {
        if(!portaudio::create_output_device_module({0, combiner_device, combiner_human_name, ptr->device}))
        {
            Loger::error("Ошибка при создании виртуального устройства1 " + combiner_human_name);
            return {};
        }
    }

    portaudio::DeviceInfo device_info{0, DAEMON_NAME + "-output-remap", DAEMON_NAME + " " + ptr->human_name + " remap", combiner_device + ".monitor"};
    return get_input_device_module(&device_info);
}


bool CoreConfigHandler::check_portaudio_device(const std::string& name)
{
    freopen("/dev/null","w", stderr);
    PaError err = Pa_Initialize();
    freopen("/dev/tty","w", stderr);
    if (err != paNoError)
    {
        Loger::error("Не удалось получить список устройств portaudio");
        return {};
    }

    std::string dev_names;
    for (int i = 0; i < Pa_GetDeviceCount(); i++)
    {
        const auto deviceInfo = Pa_GetDeviceInfo(i);
        dev_names += std::string(deviceInfo->name) + ", ";
        if (deviceInfo->name == name)
        {
            return true;
        }
    }
    Loger::warning("Устройство " + name + " не найдено в списке [" + dev_names + "]");
    return false;
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
