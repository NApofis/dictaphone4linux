//
// Created by ntikhonov on 17.11.24.
//
#include <iostream>
#include <string>
#include <array>
#include <fstream>

#include "pulseaudio_device.h"

#include <complex>


/*
 * Выполнить команду в консоле
 */
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



namespace portaudio
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
        result.push_back(str.substr(start - SEPARATOR.size()));
        return result;
    }

    std::vector<DeviceInfo> load_list_devices(const std::string& command)
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
        std::vector<DeviceInfo> result;
        for(const auto& device : devices)
        {
            if(auto current_name = read_value(device, "name: ", '<'); !current_name.empty())
            {
                auto& dev = result.emplace_back();
                dev.real = device.find("ports:") != std::string::npos && device.find("active port:") != std::string::npos;
                dev.id = stoi(read_value(device, SEPARATOR, 0));
                dev.device = current_name;
                dev.human_name = read_value(device, "device.description = ", '"');
            }
        }
        return result;
    }

    DeviceInfo* find(std::vector<DeviceInfo>& devices,
        const std::string* device, const std::string* human_name, const bool* real)
    {
        for (auto& info : devices)
        {
            if((!device || *device == info.device) &&
                (!human_name || *human_name == info.human_name) &&
                (!real || *real == info.real))
            {
                return &info;
            }
        }
        return nullptr;
    }

    std::vector<DeviceInfo> list_input_devices()
    {
        return load_list_devices("pacmd list-sources");
    }
    std::vector<DeviceInfo> list_output_devices()
    {
        return load_list_devices("pacmd list-sinks");
    }


    bool create_input_device_module(const DeviceInfo& dev)
    {
        std::string cmd = "pacmd load-module module-remap-source master=" + dev.master +
            " source_name=" + dev.device +
            R"( source_properties="'device.description=\")" + dev.human_name + R"(\"'")";
        execute_command(cmd);
        auto tests = list_input_devices();
        constexpr bool real = false;
        return find(tests, &dev.device, &dev.human_name, &real);
    }

    bool create_output_device_module(const DeviceInfo& dev)
    {
        const std::string cmd = "pacmd load-module module-combine-sink sink_name=" + dev.device +
            " slaves=" + dev.master +
            R"( sink_properties="'device.description=\")" + dev.human_name + R"(\"'")";

        execute_command(cmd);
        auto tests = list_output_devices();
        constexpr bool real = false;
        return find(tests, &dev.device, &dev.human_name, &real);
    }
}

