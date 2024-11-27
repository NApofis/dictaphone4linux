//
// Created by ntikhonov on 20.11.24.
//

#pragma once

#include <atomic>
#include <utility>
#include <vector>
#include <memory>
#include <thread>
#include <shared_mutex>
#include <unordered_map>

#include "common.h"


class Keeper
{

public:
    Keeper() = default;
    virtual ~Keeper() = default;
    virtual std::shared_ptr<std::vector<SAMPLE_TYPE>> get_place(const std::string& name, unsigned int& prev) = 0;
};

class Mixer final : public Keeper
{

    std::unordered_map<std::string, std::vector<std::shared_ptr<std::vector<SAMPLE_TYPE>>>> buffers;

    unsigned int device_count = 0;
    std::atomic_uint32_t max_buffer_number = 0;


    std::shared_ptr<std::atomic_bool> stop_flag;

    records_storage result_storage;
    std::shared_ptr<std::vector<SAMPLE_TYPE>> result;
    std::shared_ptr<std::mutex> result_locker;
    size_t save_size = 10485760 / sizeof(SAMPLE_TYPE);

    std::thread thread;
    unsigned int device_buffer_number = 0;
    std::mutex mixer_lock;

    std::vector<SAMPLE_TYPE> residue_buffer;
    void buffering();
    static size_t calculate_index(unsigned int buffer_number);
    static void insert_back(std::vector<SAMPLE_TYPE>& place, std::vector<SAMPLE_TYPE>& data);
    static void mixer(std::vector<SAMPLE_TYPE>& place, size_t& start, std::vector<SAMPLE_TYPE>& data);
    static void extra_mixer(std::vector<SAMPLE_TYPE>& place, size_t& start, size_t size, std::vector<SAMPLE_TYPE>& data);

public:
    Mixer(records_storage storage,
          std::shared_ptr<std::mutex> lock) :
        result_storage(std::move(storage)), result_locker(std::move(lock))
    {
    };
    ~Mixer() override
    {
        stop();
    };


    bool add_device(const std::string& name);
    void remove_device(const std::string& name);
    std::shared_ptr<std::vector<SAMPLE_TYPE>> get_place(const std::string& name, unsigned int& prev) override;

    void run();
    void stop();

};
