//
// Created by ntikhonov on 20.11.24.
//

#include <mutex>
#include <cstring>
#include <unordered_map>

#include "mixer.h"

#include <bits/ranges_algo.h>

void Mixer::insert_back(record_sample_data place, device_sample data)
{
    place->insert(place->end(), data.first, data.first + data.second);
    memset(data.first, 0, data.second);
}

void Mixer::mixer(record_sample_data place, size_t start, device_sample data)
{
    for(size_t i = 0; i < data.second; i++)
    {
        long sound = data.first[i] + (*place)[start] - (data.first[i] * (*place)[start] >> 0x10);
        if(sound > std::numeric_limits<SAMPLE_TYPE>::max())
        {
            sound = std::numeric_limits<SAMPLE_TYPE>::max();
        }
        else if(sound < std::numeric_limits<SAMPLE_TYPE>::min())
        {
            sound = std::numeric_limits<SAMPLE_TYPE>::min();
        }
        (*place)[start] = sound;
        start++;
    }
    memset(data.first, 0, data.second);
}

size_t Mixer::calculate_index(const unsigned int buffer_number)
{
    unsigned int i = buffer_number / BUFFERS_COUNT;
    i = buffer_number - (i * BUFFERS_COUNT);
    return i;
}


void Mixer::residue_mixer(record_sample_data place, size_t& start, device_sample data)
{
    size_t free_space = place->size() - start;

    if(data.second <= free_space)
    {
        mixer(place, start, data);
        start += data.second;
        return;
    }

    if(free_space)
    {
        device_sample mixer_part(data.first, free_space);
        mixer(place, start, mixer_part);
    }

    device_sample insert_part(data.first + free_space, data.second - free_space);
    insert_back(place, insert_part);

    memset(data.first, 0, data.second);
    start += data.second;
}

void Mixer::residue_mixer(record_sample_data place, size_t& start, record_sample_data data)
{
    residue_mixer(place, start, {data->data(), data->size()});
}


bool Mixer::add_device(const std::string& name)
{
    std::lock_guard local_lock(mixer_lock);
    if(!residue_buffer->empty())
    {
        return false;
    }
    buffer_numbers[name] = std::make_shared<std::atomic_uint32_t>(0);
    device_buffer_number[name] = 0;
    for(int i = 0; i < BUFFERS_COUNT; i++)
    {
        auto s = SAMPLE_RATE / CHUNK_BUFFER_SIZE;
        buffers[name].emplace_back(std::make_pair(new SAMPLE_TYPE[s * CHUNK_BUFFER_SIZE], s * CHUNK_BUFFER_SIZE));
        memset(buffers[name].back().first, 0, buffers[name].back().second);
    }
    return true;
}

void Mixer::remove_device(const std::string& name)
{
    std::lock_guard<std::mutex> local_lock(mixer_lock);
    size_t start = 0;
    for(unsigned int i = device_buffer_number[name]; i < buffer_numbers[name]->load(); i++)
    {
        residue_mixer(residue_buffer, start, buffers[name][calculate_index(i)]);
    }

    for(auto buf: buffers[name])
    {
        delete buf.first;
    }
    buffers.erase(name);
    buffer_numbers.erase(name);
    device_buffer_number.erase(name);
}



device_sample Mixer::get_place(const std::string& name, unsigned int& prev)
{
    if(buffer_numbers.find(name) == buffer_numbers.end())
    {
        return {nullptr, 0};
    }
    buffer_numbers[name]->store(prev);
    auto result = buffers[name][calculate_index(prev)];
    prev++;
    return result;
}

void Mixer::run()
{
    stop_flag = std::make_shared<std::atomic_bool>(false);
    thread = std::thread([this](){buffering();});
}

void Mixer::stop()
{
    if(thread.joinable())
    {
        stop_flag->store(true);
        thread.join();
    }
}

void Mixer::buffering()
{

    size_t count_sleep = 0;
    while(!stop_flag->load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if(++count_sleep < 3 && !stop_flag->load())
        {
            continue;
        }
        count_sleep = 0;

        std::lock_guard lock_mixer(mixer_lock);

        std::unordered_map<std::string, unsigned int> numbers;
        bool need_check = false;
        for(auto& [name, num] : device_buffer_number)
        {
            numbers[name] = buffer_numbers[name]->load();
            if(numbers[name] > num)
            {
                need_check = true;
            }
        }

        if(!need_check)
        {
            continue;
        }

        if(!result)
        {
            result = std::make_shared<record_sample_data::element_type>();
        }

        size_t result_size = result->size();
        for(auto& [name, value]: buffers)
        {
            size_t start = result_size;
            for(unsigned int j = device_buffer_number[name]; j < numbers[name]; j++)
            {
                residue_mixer(result, start, value[calculate_index(j)]);
            }
            device_buffer_number[name] = numbers[name];
        }

        if(!residue_buffer->empty())
        {
            residue_mixer(result, result_size, residue_buffer);
        }

        if(buffers.empty())
        {
            device_buffer_number.clear();
        }

        if(result->size() >= save_size)
        {
            std::lock_guard lock_storage(*result_locker);
            result_storage->push_back(result);
            result = nullptr;
        }
    }

}
