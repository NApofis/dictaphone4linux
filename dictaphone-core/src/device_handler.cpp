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
    void connect(const std::string& device, const std::shared_ptr<Keeper>& keeper, const std::shared_ptr<std::atomic_bool>& flag)
    {

        freopen("/dev/null","w",stderr);
        PaError err = Pa_Initialize();
        freopen("/dev/tty","w",stderr);
        if (err != paNoError) {
            Loger::error("Не удалось подключится к устройству " + device);
            flag->store(true);
            return;
        }

        bool found = false;
        PaStreamParameters inputParameters;
        for (int i = 0; i < Pa_GetDeviceCount(); i++)
        {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
            if(std::string(deviceInfo->name).find(device) != std::string::npos)
            {
                inputParameters.device = i;
                found = true;
            }
        }

        if (!found)
        {
            Loger::error("Не удалось найти устройство " + device);
            flag->store(true);
            return;
        }

        inputParameters.channelCount = 1;
        inputParameters.sampleFormat = SAMPLE_VAL;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        PaStream* stream;
        StreamData data = {device, 0, keeper};
        err = Pa_OpenStream(
            &stream,
            &inputParameters,
            nullptr,
            SAMPLE_RATE,
            CHUNK_BUFFER_SIZE,
            paClipOff, /* we won't output out of range samples so don't bother clipping them */
            callback,
            &data);

        if (err != paNoError)
        {
            Loger::error("Не удалось подключится к устройству " + device);
            flag->store(true);
            return;
        }

        err = Pa_StartStream(stream);
        if (err != paNoError)
        {
            Loger::error("Не удалось подключится к устройству " + device);
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
            Pa_Terminate();
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
