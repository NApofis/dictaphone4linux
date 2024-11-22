//
// Created by ntikhonov on 20.11.24.
//

#pragma once

#include <atomic>
#include <list>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <stdatomic.h>
#include <unordered_map>

#include <common.h>
#include <device_handler.h>


class Mixer
{
    struct BuffersList
    {
        struct Buffer
        {
            std::vector<char> data; // TODO size
            char* prt = data.data();
            size_t size = 0;
        };
        bool full = false;
        size_t max_size = 0;
        std::pmr::unordered_map<std::string, Buffer> buffers;
    };
    std::list<BuffersList> buffers;

    std::shared_ptr<std::atomic_bool> stop;
    records_storage result;
    unsigned int minutes;
    std::shared_ptr<std::mutex> result_locker;

public:
    Mixer(const std::shared_ptr<atomic_bool>& stop_flag,
          records_storage storage,
          std::shared_ptr<std::mutex> lock,
          unsigned long size_minute) :
    stop(stop_flag), result(std::move(storage)), result_locker(std::move(lock)), minutes(size_minute)
    {

    };

    void registration(const std::string& name);

    void* get_place(const std::string& name);


};
