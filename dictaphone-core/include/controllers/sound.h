//
// Created by ntikhonov on 05.12.24.
//
#pragma once

#include "controllers/icontroller.h"

/**
 * Элемент цепочки сохраняющий записанный звук в файл
 */
class SoundSaveController final : public iChain
{
    /**
     * Заголовки необходимые для формата wav файлов
     */
    struct WavHeaders
    {
        uint8_t RIFF[4] = {'R', 'I', 'F', 'F'};
        uint32_t chunk_size ;
        uint8_t WAVE[4] = {'W', 'A', 'V', 'E'};

        uint8_t fmt[4] = {'f', 'm', 't', ' '};
        uint32_t subchunk_size = 16;
        uint16_t audio_format = 1;
        uint16_t num_of_channels = NUM_CHANNELS;
        uint32_t samples_per_sec = SAMPLE_RATE;
        uint32_t bytes_per_sec = SAMPLE_RATE * 2;
        uint16_t block_align = 2;
        uint16_t bits_per_sample = 16;
        uint8_t subchunk_2_ID[4] = {'d', 'a', 't', 'a'};
        uint32_t subchunk_2_size;
    };

    static void read_header(std::fstream& file, WavHeaders& header);
    static void write_header(std::fstream& file, WavHeaders& header);

    size_t max_file_size() const;

    std::string current_file;
    std::string path_for_records;
    size_t file_size = 0;

    std::string gen_filename();

    void write_sound(bool clear = false);

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override;
};