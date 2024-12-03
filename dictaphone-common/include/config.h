//
// Created by ntikhonov on 16.11.24.
//

#pragma once

#include <fstream>
#include <string>
#include <map>
#include "dictaphone-common/const.h"

const std::string PROGRAM_ROOT_PATH = PROGRAM_ROOT_DIRECTORY;
const std::string CONFIG_FILE_PATH = PROGRAM_ROOT_PATH+"/config.conf";
const std::string START_STRING = "# Config file for dictaphone";
const std::string JOIN_STRING = ": ";
const std::string RECORD_FILE_NAME_MASK = "dictaphone_record_";
const std::string RECORD_FILE_NAME_POSTFIX = "%Y-%m-%d_%H-%M";
const std::string RECORD_FILE_NAME_EXTENSION = ".wav";

const std::string NONE_DEVICE = "None";

class Config
{
    friend std::ofstream& operator<<(std::ofstream& os, const Config* rhs);
    friend std::ifstream& operator>>(std::ifstream& is, Config* rhs);

    std::map<std::string, std::string> fields
    {
        {"input device", "None"},
        {"output device", "None"},
        {"path for records", PROGRAM_ROOT_PATH+"/records"},
        {"shelf life", "7"},
        {"file minute size", "30"},
        {"path to music file", ""}
    };

public:
    Config();

    [[nodiscard]] bool save() const;

    std::string input_device() { return fields["input device"]; }
    void input_device(const std::string& val) { fields["input device"] = val; }

    std::string output_device() { return fields["output device"]; }
    void output_device(const std::string& val) { fields["output device"] = val; }

    std::string path_for_records() { return  fields["path for records"]; }
    bool path_for_records(const std::string& val);

    unsigned int shelf_life() { return stoi(fields["shelf life"]); }
    bool shelf_life(unsigned int val);

    unsigned int file_minute_size() { return stoi(fields["file minute size"]); }
    bool file_minute_size(unsigned int val);

};

