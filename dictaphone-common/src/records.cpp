//
// Created by ntikhonov on 19.11.24.
//

#include "records.h"
#include "config.h"

#include <filesystem>
#include <iostream>
#include <list>

namespace records
{

    unsigned int delete_records_after(std::shared_ptr<std::string> path, const std::chrono::system_clock::time_point* time)
    {
        std::list<std::filesystem::path> will_delete;

        if(!std::filesystem::exists(path->c_str()))
        {
            return 0;
        }

        for(auto& p: std::filesystem::directory_iterator(path->c_str()))
        {
            std::string filename = p.path().filename();
            if(!p.is_regular_file() ||
                filename.substr(0, RECORD_FILE_NAME_MASK.size()) != RECORD_FILE_NAME_MASK ||
                p.path().extension() != RECORD_FILE_NAME_EXTENSION )
            {
                continue;
            }
            std::string time_created = filename.substr(RECORD_FILE_NAME_MASK.size(),
                filename.size() - RECORD_FILE_NAME_MASK.size() - RECORD_FILE_NAME_EXTENSION.size());

            std::tm tm = {};
            strptime(time_created.c_str(), "%Y-%m-%d_%H-%M", &tm);
            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            if(time == nullptr || tp >= *time)
            {
                will_delete.push_back(p.path());
            }
        }

        for(auto& p: will_delete)
        {
            std::filesystem::remove(p);
        }
        return will_delete.size();
    }


}
