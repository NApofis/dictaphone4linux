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

    using time_ptr = const std::chrono::system_clock::time_point*;
    using comparator = bool (*)(time_ptr val1, time_ptr val2);

    unsigned int delete_records(const std::string& path, const std::chrono::system_clock::time_point* time, comparator com)
    {
        std::list<std::filesystem::path> will_delete;

        if(!std::filesystem::exists(path.c_str()))
        {
            return 0;
        }

        for(auto& p: std::filesystem::directory_iterator(path.c_str()))
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
            strptime(time_created.c_str(), RECORD_FILE_NAME_POSTFIX.c_str(), &tm);
            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            if(com(&tp, time))
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


    unsigned int delete_records_after(const std::shared_ptr<std::string>& path, const std::chrono::system_clock::time_point* time)
    {
        if(!path || path->empty())
        {
            return 0;
        }
        return delete_records(*path, time, [](time_ptr val1, time_ptr val2) { return val2 == nullptr || *val1 >= *val2; } );
    }

    unsigned int delete_records_before(const std::string& path, const std::chrono::system_clock::time_point* time)
    {
        return delete_records(path, time, [](time_ptr val1, time_ptr val2) { return *val1 < *val2; } );
    }


}
