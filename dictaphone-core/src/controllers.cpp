//
// Created by ntikhonov on 20.11.24.
//

#include "controllers.h"
#include "common.h"
#include "date/tz.h"


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

   if(is_new)
   {
      data->path_for_records = config_handler->get_path_for_records();
      data->minute_size = config_handler->get_file_minute_size();
   }
   config_handler->delete_old_records();

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
      dev_info.name.clear();
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
   dev_info.thread = std::thread([this, &dev_info, name=new_dev.second](){pulseaudio::connect(name, mix, dev_info.stop_flag);});
}

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
      input_device.stop_flag->store(true);
      in = true;
   }
   if(!output_device.name.empty() && output_device.thread.joinable())
   {
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
   mix->stop();

}

std::string SoundSaveController::gen_filename()
{
   auto t = date::make_zoned(date::current_zone(), std::chrono::system_clock::now());
   return data->path_for_records + '/' + RECORD_FILE_NAME_MASK + date::format(RECORD_FILE_NAME_POSTFIX, t) + RECORD_FILE_NAME_EXTENSION;
}

void SoundSaveController::read_header(std::fstream& file, WavHeaders& header)
{
   file.seekg(0, std::ios::beg);
   const auto header_str = reinterpret_cast<char *>(&header);
   file.readsome(header_str, sizeof(WavHeaders));
}

void SoundSaveController::write_header(std::fstream& file, WavHeaders& header)
{
   file.seekp(0, std::ios::beg);
   const auto header_str = reinterpret_cast<char *>(&header);
   file.write(header_str, sizeof(WavHeaders));
}

void SoundSaveController::execute()
{
   write_sound();
   return next_execute_if_exists();
}
void SoundSaveController::cancel()
{
   write_sound();
   return next_cancel_if_exists();
}

size_t SoundSaveController::max_file_size() const
{
   return (data->minute_size * 60) * SAMPLE_RATE * sizeof(SAMPLE_TYPE);
}


void SoundSaveController::write_sound()
{

   size_t storage_count = 0;
   records_storage need_write;
   {
      std::lock_guard<std::mutex> lock(*data->storage_records_lock);
      if(data->storage_records->empty())
      {
         return;
      }
      need_write = std::make_shared<records_storage::element_type>();
      for(auto& record : *data->storage_records)
      {
         if(record && !record->empty())
         {
            storage_count += record->size();
            need_write->push_back(record);
         }
      }
      data->storage_records->clear();
   }

   WavHeaders headers;
   std::fstream out;
   if(current_file.empty())
   {
      current_file = gen_filename();
      out.open(current_file, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
   }
   else
   {
      out.open(current_file, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
      read_header(out, headers);
   }


   headers.chunk_size = file_size + (storage_count  * sizeof(SAMPLE_TYPE)) + sizeof(WavHeaders) - 8;
   headers.subchunk_2_size = file_size + (storage_count * sizeof(SAMPLE_TYPE)) + sizeof(WavHeaders) - 44;
   write_header(out, headers);
   out.seekp(0, std::ios::end);
   for(auto& rec: *need_write)
   {
      out.write(reinterpret_cast<const char*>(rec->data()), sizeof(SAMPLE_TYPE) * rec->size());
   }
   out.close();

   file_size += storage_count * sizeof(SAMPLE_TYPE);
   if(file_size > max_file_size())
   {
      current_file.clear();
      file_size = 0;
   }
}

//21399588
//37044000