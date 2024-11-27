//
// Created by ntikhonov on 20.11.24.
//

#include <mutex>
#include <cstring>

#include "mixer.h"

void Mixer::insert_back(std::vector<SAMPLE_TYPE>& place, std::vector<SAMPLE_TYPE>& data)
{
    place.insert(place.end(), data.begin(), data.end());
    memset(data.data(), 0, SAMPLE_RATE);
}

void Mixer::mixer(std::vector<SAMPLE_TYPE>& place, size_t& start, std::vector<SAMPLE_TYPE>& data)
{
    for(auto& val: data)
    {
        auto sound = val + place[start];
        if(sound > std::numeric_limits<SAMPLE_TYPE>::max())
        {
            sound = std::numeric_limits<SAMPLE_TYPE>::max();
        }
        else if(sound < std::numeric_limits<SAMPLE_TYPE>::min())
        {
            sound = std::numeric_limits<SAMPLE_TYPE>::min();
        }
        val = 0;
        place[start] = sound;
        start++;
    }
}

size_t Mixer::calculate_index(const unsigned int buffer_number)
{
    unsigned int i = buffer_number / BUFFERS_COUNT;
    i = buffer_number - (i * BUFFERS_COUNT);
    return i;
}

void Mixer::extra_mixer(std::vector<SAMPLE_TYPE>& place, size_t& start, const size_t size, std::vector<SAMPLE_TYPE>& data)
{
    if(data.size() <= size)
    {
        return mixer(place, start, data);
    }
    std::vector<SAMPLE_TYPE> mixer_part{data.begin(), data.begin() + size};
    mixer(place, start, mixer_part);
    std::vector<SAMPLE_TYPE> insert_part{data.begin() + size, data.end()};
    insert_back(place, insert_part);
    memset(place.data(), 0, SAMPLE_RATE);
}




bool Mixer::add_device(const std::string& name)
{
    std::lock_guard local_lock(mixer_lock);
    if(!residue_buffer.empty())
    {
        return false;
    }

    for(int i = 0; i < BUFFERS_COUNT; i++)
    {
        buffers[name].emplace_back(std::make_shared<std::vector<SAMPLE_TYPE>>(SAMPLE_RATE, 0));
    }
    return true;
}

void Mixer::remove_device(const std::string& name)
{
    std::lock_guard<std::mutex> local_lock(mixer_lock);
    const unsigned int max = max_buffer_number.load();
    bool residue_empty = residue_buffer.empty();
    size_t start = 0;
    for(unsigned int i = device_buffer_number; i < max; i++)
    {
        if(residue_empty)
        {
            insert_back(residue_buffer, *buffers[name][calculate_index(i)]);
        }
        else
        {
            mixer(residue_buffer, start, *buffers[name][calculate_index(i)]);
        }
    }
    buffers.erase(name);
}



std::shared_ptr<std::vector<SAMPLE_TYPE>> Mixer::get_place(const std::string& name, unsigned int& prev)
{
    prev++;
    auto max = max_buffer_number.load();
    while(true)
    {
        if((max < prev && max_buffer_number.compare_exchange_strong(max, prev))
            || (max == prev))
        {
            break;
        }
        else if (max > prev)
        {
            prev = max;
            break;
        }
    }
    auto result = buffers[name][calculate_index(prev)];
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
        const unsigned int max = max_buffer_number.load();

        if(device_buffer_number >= max)
        {
            continue;
        }

        if(!result)
        {
            result = std::make_shared<std::vector<SAMPLE_TYPE>>();
        }

        size_t result_size = result->size();
        bool first_dev = true;
        for(auto& [name, value]: buffers)
        {
            size_t start = result_size;
            for(unsigned int j = device_buffer_number; j < max; j++)
            {
                if(first_dev)
                {
                    insert_back(*result, *value[calculate_index(j)]);
                }
                else
                {
                    mixer(*result, start, *value[calculate_index(j)]);
                }
            }
            first_dev = false;
        }

        if(!residue_buffer.empty())
        {
            extra_mixer(*result, result_size, result->size() - result_size, residue_buffer);
        }

        if(buffers.empty())
        {
            device_buffer_number = 0;
            max_buffer_number.store(0);
        }
        else
        {
            device_buffer_number = max;
        }

        if(result->size() >= save_size)
        {
            std::lock_guard lock_storage(*result_locker);
            result_storage->push_back(result);
            result = nullptr;
        }
    }

}
