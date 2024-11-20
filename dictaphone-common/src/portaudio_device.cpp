//
// Created by ntikhonov on 17.11.24.
//
#include <iostream>
#include <string>
#include <array>

#include "portaudio.h"
#include "portaudio_device.h"


std::string execute_command(const std::string& cmd)
{
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Не удалось выполнить команду" << std::endl;
        return {};
    }

    std::string result;
    std::array<char, 1024> buffer{};
    while(!std::feof(pipe))
    {
        const auto bytes = std::fread(buffer.data(), 1, buffer.size(), pipe);
        result.append(buffer.data(), bytes);
    }

    pclose(pipe);
    return result;
}

namespace portaudio_devices
{
    const std::string SEPARATOR = "index: ";

    std::vector<std::string> split_by_index(const std::string& str)
    {
        std::vector<std::string> result;
        size_t start = str.find(SEPARATOR);
        if(start == std::string::npos)
        {
            return result;
        }

        auto find = str.find(SEPARATOR, start);
        while (find != std::string::npos)
        {
            result.push_back(str.substr(start - SEPARATOR.size(), find - start + SEPARATOR.size()));
            start = find + SEPARATOR.size();
            find = str.find(SEPARATOR, start);
        }
        return result;
    }

    std::vector<Device> load_list_devices(const std::string& command)
    {
        const std::string command_result = execute_command(command);
        const auto devices = split_by_index(command_result);

        auto read_value = [](const std::string& str, const std::string& field, const char rem) -> std::string
        {
            if (const size_t start = str.find(field); start != std::string::npos)
            {
                size_t end = str.find('\n', start);
                auto result = str.substr(start + field.size(), end - start - field.size());
                if (rem != 0 && result[0] == rem)
                {
                    result = result.substr(1, result.size() - 2);
                }
                return result;
            }
            return "";
        };
        std::vector<Device> result;
        bool next_default = false;
        for(const auto& device : devices)
        {
            if(device.find("ports:") != std::string::npos && device.find("active port:") != std::string::npos)
            {
                if(auto current_name = read_value(device, "name: ", '<'); !current_name.empty())
                {
                    auto& [id, name, description, default_device] = result.emplace_back();
                    id = stoi(read_value(device, SEPARATOR, 0));
                    default_device = next_default;
                    name = current_name;
                    description = read_value(device, "device.description = ", '"');
                }
            }
            next_default = device.find_first_of('*') != std::string::npos;

        }
        return result;
    }

    std::vector<Device> list_input_devices()
    {
        return load_list_devices("pacmd list-sources");
    }
    std::vector<Device> list_output_devices()
    {
        return load_list_devices("pacmd list-sinks");
    }


}

