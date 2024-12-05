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

    verification(data->input_device, input_device);
    verification(data->output_device, output_device);
    return next_execute_if_exists();
}

void DeviceController::cancel()
{
    // Завершаем запись усех устройств по очереди
    bool in = false, out = false;
    if(!input_device.name.empty() && input_device.thread.joinable())
    {
        Loger::info("Остановка записи устройства " + input_device.name);
        input_device.stop_flag->store(true);
        in = true;
    }
    if(!output_device.name.empty() && output_device.thread.joinable())
    {
        Loger::info("Остановка записи устройства " + output_device.name);
        output_device.stop_flag->store(true);
        out = true;
    }
    if(in)
    {
        input_device.thread.join();
        mix->remove_device(input_device.name);
    }
    if(out)
    {
        output_device.thread.join();
        mix->remove_device(output_device.name);
    }
    Loger::info("Остановка работы микшера");
    mix->stop();
    return next_cancel_if_exists();
}


void DeviceController::verification(dev_status& new_dev, DeviceHandlerInfo& dev_info) const
{
    if(new_dev.first == Status::unchanged)
    {
        // Устройства не изменились. Продолжаем запись
        if(!dev_info.name.empty() && dev_info.stop_flag->load())
        {
            Loger::warning("Потеря соединения с устройством");
            new_dev.first = Status::brocken;
            mix->remove_device(dev_info.name);
        }
        return;
    }

    if(new_dev.first != Status::created)
    {
        Loger::error("Устройство не было подключено после разъединения");
        return;
    }

    // Устройство изменилось => нужно остановить текущую запись
    if(!dev_info.name.empty())
    {
        dev_info.stop_flag->store(true);
        dev_info.thread.join();

        mix->remove_device(dev_info.name);
        dev_info.name = "";
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
    dev_info.stop_flag = std::make_shared<std::atomic_bool>(false);
    dev_info.thread = std::thread([this, &dev_info, name=new_dev.second](){portaudio::connect(name, mix, dev_info.stop_flag);});
}

