//
// Created by ntikhonov on 20.11.24.
//

#include "mixer.h"

void Mixer::registration(const std::string& name)
{
    if(buffers.empty())
    {

    }
}


void* Mixer::get_place(const std::string& name)
{
    stop->store(false);
    while(!stop_flag)
    {
        std::unique_lock lock(console_mutex);
        while (!stop_flag && console_queue.empty())
        {
            console_condition.wait(lock);
        }
        if(!console_queue.empty())
        {
            std::cout << console_queue.front();
            console_queue.pop();
            std::cout.flush();
        }
    }
}
