//
// Created by ntikhonov on 17.11.24.
//

#include "ui.h"
#include "records.h"

#include <chrono>


UIConfigHandler::UIConfigHandler()
{
    config = std::make_shared<Config>();
    path_for_records = std::make_shared<std::string>(config->path_for_records());
    shelf_life = std::to_string(config->shelf_life());
    file_minute_size = std::to_string(config->file_minute_size());
}
bool UIConfigHandler::save_config(std::string& message)
{
    for(const auto& [fst, snd]: input_devices)
    {
        if(fst == selected_input_device)
        {
            config->input_device(snd.device);
            break;
        }
    }

    for(const auto& [fst, snd]: output_devices)
    {
        if(fst == selected_output_device)
        {
            config->output_device(snd.device);
            break;
        }
    }
    if(!config->path_for_records(*path_for_records))
    {
        message = "Ошибка: папка " + *path_for_records + " отсутствует";
        return false;
    }

    if(!config->shelf_life(stoi(shelf_life)))
    {
        message = "Ошибка: время хранения записей не может быть более 365 дней";
        return false;
    }

    if(!config->file_minute_size(stoi(file_minute_size)))
    {
        message = "Ошибка: минутный размер файла должен быть больше 1 и меньше 180 минут.";
        return false;
    }

    if(!config->save())
    {
        throw std::runtime_error("Ошибка при записи конфигурационного файла");
    }
    message = "Успешно сохранено";
    return true;
}



void UIDeleterRecords::delete_records(std::string& message) const
{
    const auto value = std::stoi(selected_delete_value);
    if(value < 0 && selected_delete_period != 0)
    {
        message = "Значение должно быть больше 0";
        return;
    }
    unsigned int deleted = 0;
    switch (selected_delete_period)
    {
    case 0:
        {
            deleted = delete_all_records();
            break;
        }
    case 1:
        {
            deleted = delete_by_hour(value);
            break;
        }
    case 2:
        {
            deleted = delete_by_days(value);
            break;
        }
    case 3:
        {
            deleted = delete_by_month(value);
            break;
        }
    }
    message = "Удалено " + std::to_string(deleted) + " записей";
}

unsigned int UIDeleterRecords::delete_by_hour(const int hour) const
{
    auto now = std::chrono::system_clock::now();
    now -= std::chrono::hours(hour);
    return records::delete_records_after(path_records, &now);
}

unsigned int UIDeleterRecords::delete_by_days(const int days) const
{
    auto now = std::chrono::system_clock::now();
    now -= std::chrono::days(days);
    return records::delete_records_after(path_records, &now);
}

unsigned int UIDeleterRecords::delete_by_month(const int month) const
{
    auto now = std::chrono::system_clock::now();
    now -= std::chrono::months(month);
    return records::delete_records_after(path_records, &now);
}

 unsigned int UIDeleterRecords::delete_all_records() const
 {
    return records::delete_records_after(path_records, nullptr);
 }
