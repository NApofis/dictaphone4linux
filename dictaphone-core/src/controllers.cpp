//
// Created by ntikhonov on 20.11.24.
//

#include "controllers.h"

#include <common.h>

void ConfigController::process_device(bool is_input_dev, dev_status& status) const
{
   std::string name;
   if(is_input_dev)
   {
      name = config_handler.get_input_device_module();
   }
   else
   {
      name = config_handler.get_output_device_module();
   }

   if(name != saved_input_device_name)
   {
      status.second = name;
      status.first = Status::created;
   }
   else if(status.first == Status::brocken)
   {
      status.first = Status::created;
   }
}


void ConfigController::execute()
{
   bool is_new = config_handler.load_new_version();
   data->path_for_records = config_handler.get_path_for_records();
   config_handler.delete_old_records();
   if(is_new)
   {
      if(data->input_device.first == Status::brocken)
      {
         process_device(true, data->input_device);
      }

      if(data->output_device.first == Status::brocken)
      {
         process_device(false, data->output_device);
      }
   }
   return next_execute_if_exists();
}


void DeviceController::verification(dev_status& new_dev, DeviceHandlerInfo& dev_info)
{
   if(new_dev.first == Status::unchanged)
   {
      // Устройства не изменились. Продолжаем запись
      if(dev_info.used && dev_info.stop_flag->load())
      {
         Loger::warning("Потеря соединения с устройством");
         new_dev.first = Status::brocken;
      }
      return;
   }

   if(new_dev.first != Status::created)
   {
      Loger::error("Устройство не было подключено после разъединения");
      return;
   }

   // Устройство изменилось => нужно остановить текущую запись
   if(dev_info.used && dev_info.thread.joinable())
   {
      dev_info.stop_flag->store(true);
      dev_info.thread.join();
   }

   new_dev.first = Status::unchanged;

   if(new_dev.second.empty())
   {
      // Запись устройства отключили.
      dev_info.used = false;
      return;
   }

   // Выбрали новое устройство
   dev_info.used = true;
   dev_info.stop_flag->store(false);
   dev_info.obj = std::make_shared<DeviceHandler>(new_dev.second, nullptr, output_device.stop_flag);
   dev_info.thread = std::thread([this](){input_device.obj->connect();});
}

void DeviceController::execute()
{
   verification(data->input_device, input_device);
   verification(data->output_device, output_device);
   return next_execute_if_exists();
}

void DeviceController::cancel()
{
   // Завершаем запись усех устройств по очереди
   bool in = false, out = false;
   if(input_device.used && input_device.thread.joinable())
   {
      input_device.stop_flag->store(true);
      in = true;
   }
   if(output_device.used && output_device.thread.joinable())
   {
      output_device.stop_flag->store(true);
      out = true;
   }
   if(in)
   {
      input_device.thread.join();
   }
   if(out)
   {
      output_device.thread.join();
   }
}


