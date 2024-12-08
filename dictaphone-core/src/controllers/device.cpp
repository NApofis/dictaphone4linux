//
// Created by ntikhonov on 05.12.24.
//
#include "controllers/device.h"

#include <config.h>

#include "device_handler.h"

void DeviceController::execute()
{

    if(!mix)
    {
        mix = std::make_shared<Mixer>(data->storage_records, data->storage_records_lock);
        mix->run();
    }

    if ((data->input_device.first == Status::created && !data->input_device.second.empty())
        || (data->output_device.first == Status::created && !data->output_device.second.empty()))
    {
        device_disconnect(input_device);
        data->input_device.first = Status::created;
        device_disconnect(output_device);
        data->output_device.first = Status::created;
    }

    connect_device(data->input_device, input_device);
    connect_device(data->output_device, output_device);
    return next_execute_if_exists();
}

void DeviceController::device_disconnect(DeviceHandlerInfo& dev_info)
{
    dev_info.stop_flag->store(true);
    if (dev_info.thread.joinable())
    {
        Loger::info("Отключение устройства " + dev_info.name);
        dev_info.thread.join();
    }

    mix->remove_device(dev_info.name);
    dev_info.name = "";
    if (!active_device())
    {
        Pa_Terminate();
    }
}


void DeviceController::cancel()
{
    // Завершаем запись усех устройств по очереди
    device_disconnect(input_device);
    device_disconnect(output_device);

    Loger::info("Остановка работы микшера");
    mix->stop();
    return next_cancel_if_exists();
}


void DeviceController::connect_device(dev_status& new_dev, DeviceHandlerInfo& dev_info)
{
    if(new_dev.first == Status::unchanged)
    {
        // Устройства не изменились. Продолжаем запись
        if(!dev_info.name.empty() && dev_info.stop_flag->load())
        {
            Loger::warning("Потеря соединения с устройством " + dev_info.name);
            device_disconnect(dev_info);
            new_dev.first = Status::brocken;
        }
        return;
    }
    else if(new_dev.first == Status::brocken)
    {
        Loger::error("Устройство не было подключено после разъединения");
        return;
    }


    if(new_dev.second.empty())
    {
        // Запись устройства отключили.
        new_dev.first = Status::unchanged;
        if (!dev_info.name.empty())
        {
            device_disconnect(dev_info);
        }
        return;
    }

    // Выбрали новое устройство
    if (!active_device())
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        freopen("/dev/null","w",stderr);
        auto err = Pa_Initialize();
        freopen("/dev/tty","w",stderr);
        if (err != paNoError)
        {
            Loger::error("Не удалось запустить библиотеку portaudio");
            return;
        }
    }

    bool found = false;
    PaStreamParameters parameters;
    for (int i = 0; i < Pa_GetDeviceCount(); i++)
    {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if(std::string(deviceInfo->name).find(new_dev.second) != std::string::npos)
        {
            parameters.device = i;
            found = true;
            break;
        }
    }

    if (!found)
    {
        Loger::error("Не удалось найти устройство " + dev_info.name);
        return;
    }

    parameters.channelCount = 1;
    parameters.sampleFormat = SAMPLE_VAL;
    parameters.suggestedLatency = Pa_GetDeviceInfo(parameters.device)->defaultHighInputLatency;
    parameters.hostApiSpecificStreamInfo = nullptr;


    new_dev.first = Status::unchanged;
    dev_info.name = new_dev.second;
    if (!mix->add_device(dev_info.name))
    {
        Loger::warning("Устройство не подключено поскольку еще не сохранены записи предыдущего устройства");
        new_dev.first = Status::brocken;
    }
    dev_info.stop_flag->store(false);
    dev_info.thread = std::thread([this, &dev_info, name=new_dev.second, parameters](){portaudio::connect(parameters, name, mix, dev_info.stop_flag);});
}

bool DeviceController::active_device() const
{
    return !input_device.name.empty() || !output_device.name.empty();
}
