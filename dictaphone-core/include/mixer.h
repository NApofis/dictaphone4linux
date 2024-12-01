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
    virtual device_sample get_place(const std::string& name, unsigned int& prev) = 0;
};

class Mixer final : public Keeper
{

    std::unordered_map<std::string, std::vector<device_sample>> buffers;

    unsigned int device_count = 0;
    std::atomic_uint32_t max_buffer_number = 0;


    std::shared_ptr<std::atomic_bool> stop_flag;

    records_storage result_storage;
    record_sample_data result;
    std::shared_ptr<std::mutex> result_locker;
    size_t save_size = 10485760 / sizeof(SAMPLE_TYPE);

    std::thread thread;
    unsigned int device_buffer_number = 0;
    std::mutex mixer_lock;

    record_sample_data residue_buffer = std::make_shared<record_sample_data::element_type>();

    void buffering();
    static size_t calculate_index(unsigned int buffer_number);
    static void insert_back(record_sample_data place, device_sample data);
    static void mixer(record_sample_data place, size_t& start, device_sample data);
    static void residue_mixer(record_sample_data place, size_t& start, size_t size, record_sample_data data);

public:
    Mixer(records_storage storage,
          std::shared_ptr<std::mutex> lock) :
        result_storage(std::move(storage)), result_locker(std::move(lock)) {};

    ~Mixer() override
    {
        stop();
    };


    bool add_device(const std::string& name);
    void remove_device(const std::string& name);
    device_sample get_place(const std::string& name, unsigned int& prev) override;

    void run();
    void stop();

};
