//
// Created by ntikhonov on 16.11.24.
//

#include <iostream>
#include <filesystem>

#include "config.h"

std::ofstream& operator<<(std::ofstream& os, const Config* rhs)
{
    os << START_STRING << std::endl;
    os << "#" << std::endl << "#" << std::endl;
    for(const auto& [fst, snd] : rhs->fields)
    {
        os << fst << JOIN_STRING << snd << std::endl;
    }
    return os;
};


std::ifstream& operator>>(std::ifstream& is, Config* rhs)
{
    std::string line;
    std::getline(is, line);

    if(line != START_STRING)
    {
        throw std::runtime_error("Неизвестный формат конфигурационного файла");
    }
    std::getline(is, line); // #
    std::getline(is, line); // #

    for(auto& [name, value] : rhs->fields)
    {
        std::getline(is, line);
        if(line.empty())
        {
            continue;
        }
        if(line.substr(0, name.size()) != name)
        {
            throw std::runtime_error("Неизвестный формат конфигурационного файла");
        }
        const size_t start = JOIN_STRING.size() + name.size();
        value = line.substr(start, line.size() - start ); // + переход на след строку
    }
    return is;
};

Config::Config()
{
    if(!std::filesystem::exists(CONFIG_FILE_PATH))
    {
        if(!save())
        {
            std::cerr << "Не удалось сохранить конфигурационный файла" << std::endl;
        };
        std::filesystem::create_directory(fields["path for records"]);
    }
    else
    {
        std::ifstream in(CONFIG_FILE_PATH);
        in >> this;
    }
}

bool Config::save() const
{
    std::ofstream out;
    try
    {
        out.open(CONFIG_FILE_PATH);
    }
    catch (...)
    {
        return false;
    }
    out << this << std::endl;
    out.close();
    return true;
}

bool Config::path_for_records(const std::string& val)
{
    if(std::filesystem::exists(val))
    {
        fields["path for records"] = val;
        return true;
    }
    return false;
}

bool Config::shelf_life(const unsigned int val)
{
    if(val < 365)
    {
        fields["shelf life"] = std::to_string(val);
        return true;
    }
    return false;
}

bool Config::file_minute_size(const unsigned int val)
{
    if(val < 180 && val > 1)
    {
        fields["file minute size"] = std::to_string(val);
        return true;
    }
    return false;
}
