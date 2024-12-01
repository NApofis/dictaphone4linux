//
// Created by ntikhonov on 20.11.24.
//

#include <mutex>
#include <cstring>

#include "mixer.h"

void Mixer::insert_back(record_sample_data place, device_sample data)
{
    place->insert(place->end(), data.first, data.first + data.second);
    memset(data.first, 0, SAMPLE_RATE);
}

void Mixer::mixer(record_sample_data place, size_t& start, device_sample data)
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
        data.first[i] = 0;
        (*place)[start] = sound;
        start++;
    }
}

size_t Mixer::calculate_index(const unsigned int buffer_number)
{
    unsigned int i = buffer_number / BUFFERS_COUNT;
    i = buffer_number - (i * BUFFERS_COUNT);
    return i;
}

void Mixer::residue_mixer(record_sample_data place, size_t& start, const size_t size, record_sample_data data)
{
    if(data->size() <= size)
    {

        return mixer(place, start, {data->data(), data->size()});
    }
    device_sample mixer_part(data->data(), size);
    mixer(place, start, mixer_part);
    device_sample insert_part(data->data() + size, data->size() - size);
    insert_back(place, insert_part);
    data->clear();
}




bool Mixer::add_device(const std::string& name)
{
    std::lock_guard local_lock(mixer_lock);
    if(!residue_buffer->empty())
    {
        return false;
    }

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
    const unsigned int max = max_buffer_number.load();
    bool residue_empty = residue_buffer->empty();
    size_t start = 0;
    for(unsigned int i = device_buffer_number; i < max; i++)
    {
        if(residue_empty)
        {
            insert_back(residue_buffer, buffers[name][calculate_index(i)]);
        }
        else
        {
            mixer(residue_buffer, start, buffers[name][calculate_index(i)]);
        }

    }

    for(auto buf: buffers[name])
    {
        delete buf.first;
    }
    buffers.erase(name);
}



device_sample Mixer::get_place(const std::string& name, unsigned int& prev)
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
            result = std::make_shared<record_sample_data::element_type>();
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
                    insert_back(result, value[calculate_index(j)]);
                }
                else
                {
                    mixer(result, start, value[calculate_index(j)]);
                }
            }
            first_dev = false;
        }

        if(!residue_buffer->empty())
        {
            residue_mixer(result, result_size, result->size() - result_size, residue_buffer);
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
