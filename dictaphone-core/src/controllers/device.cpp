//
// Created by ntikhonov on 05.12.24.
//
#include "controllers/device.h"
#include "device_handler.h"

void DeviceController::execute()
{

    if(!mix)
    {
        mix = std::make_shared<Mixer>(data->storage_records, data->storage_records_lock);
        mix->run();
    }

    if (data->input_device.first == Status::created || data->output_device.first == Status::created)
    {
        device_disconnect(input_device);
        data->input_device.first == Status::created;
        device_disconnect(output_device);
        data->output_device.first == Status::created;
    }

    connect_device(data->input_device, input_device);
    connect_device(data->output_device, output_device);
    return next_execute_if_exists();
}

void DeviceController::device_disconnect(DeviceHandlerInfo& dev_info) const
{
    dev_info.stop_flag->store(true);
    if (dev_info.thread.joinable())
    {
        Loger::info("Остановка записи устройства " + dev_info.name);
        dev_info.thread.join();
    }

    mix->remove_device(dev_info.name);
    dev_info.name = "";
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


void DeviceController::connect_device(dev_status& new_dev, DeviceHandlerInfo& dev_info) const
{
    if(new_dev.first == Status::unchanged)
    {
        // Устройства не изменились. Продолжаем запись
        if(!dev_info.name.empty() && dev_info.stop_flag->load())
        {
            Loger::warning("Потеря соединения с устройством");
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

    // Устройство изменилось => нужно остановить текущую запись
    if(!dev_info.name.empty())
    {
        device_disconnect(dev_info);
    }

    new_dev.first = Status::unchanged;

    if(new_dev.second.empty())
    {
        // Запись устройства отключили.
        return;
    }

    // Выбрали новое устройство
    dev_info.name = new_dev.second;
    if (!mix->add_device(dev_info.name))
    {
        Loger::warning("Устройство не подключено поскольку еще не сохранены записи предыдущего устройства");
        new_dev.first = Status::brocken;
    }
    dev_info.stop_flag->store(false);
    dev_info.thread = std::thread([this, &dev_info, name=new_dev.second](){portaudio::connect(name, mix, dev_info.stop_flag);});
}

