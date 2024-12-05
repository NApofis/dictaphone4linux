//
// Created by ntikhonov on 05.12.24.
//
#include "controllers/sound.h"
#include "date/tz.h"


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
   write_sound(true);
   Loger::info("Запись остаточных данных");
   return next_cancel_if_exists();
}

size_t SoundSaveController::max_file_size() const
{
   return (data->minute_size * 60) * SAMPLE_RATE * sizeof(SAMPLE_TYPE);
}


void SoundSaveController::write_sound(bool clear)
{

   size_t storage_count = 0;
   records_storage need_write;
   {
      std::lock_guard<std::mutex> lock(*data->storage_records_lock);
      if(data->storage_records->empty())
      {
         if(clear)
         {
            Loger::info("Остаточных данных нет");
         }
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
   if(current_file.empty() || data->path_for_records != path_for_records)
   {
      current_file = gen_filename();
      out.open(current_file, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
      path_for_records = data->path_for_records;
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
   if(file_size > max_file_size() || clear)
   {
      current_file.clear();
      file_size = 0;
   }
}
