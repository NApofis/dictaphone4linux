//
// Created by ntikhonov on 20.11.24.
//

#include <iostream>
#include <cstring>
#include <functional>

#include "device_handler.h"
#include "common.h"

namespace portaudio
{
    void connect(const PaStreamParameters params, const std::string& device, const std::shared_ptr<Keeper>& keeper, const std::shared_ptr<std::atomic_bool>& flag)
    {
        PaStream* stream;
        StreamData data = {device, 0, keeper};
        PaError err = Pa_OpenStream(
            &stream,
            &params,
            nullptr,
            SAMPLE_RATE,
            CHUNK_BUFFER_SIZE,
            paClipOff,
            callback,
            &data);

        if (err != paNoError)
        {
            Loger::error("Устройство найдено " + device + " но к нему не удалось подключиться");
            flag->store(true);
            return;
        }

        err = Pa_StartStream(stream);
        if (err != paNoError)
        {
            Loger::error("Не начать чтение с устройства  " + device);
            flag->store(true);
            return;
        }
        Loger::info("Подключено устройство " + device);

        try
        {
            while (!flag->load())
            {
                Pa_Sleep(1000);
            }
            Pa_CloseStream(stream);
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    static int callback(const void* input_data, void*, const unsigned long frames_per_Buffer, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* user_data)
    {
        auto* info = static_cast<StreamData*>(user_data);

        if(!info->data.first || info->data_id + frames_per_Buffer > info->data.second)
        {
            try
            {
                const auto place = info->keep->get_place(info->name, info->buffer_id);
                if(place.first == nullptr)
                {
                    return 1;
                }

                info->data = place;
                info->data_id = 0;
                info->data_ptr = info->data.first;
            }
            catch (std::exception& e)
            {
                Loger::error(std::string("Ошибка при записи звука. ") + e.what());
                return 1;
            }
        }

        const auto* input = static_cast<const SAMPLE_TYPE*>(input_data);

        memcpy(info->data_ptr, input, frames_per_Buffer * sizeof(SAMPLE_TYPE));
        info->data_id += frames_per_Buffer;
        info->data_ptr += frames_per_Buffer;
        return 0;
    }
}
