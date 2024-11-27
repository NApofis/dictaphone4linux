//
// Created by ntikhonov on 20.11.24.
//

#pragma once
#include <list>
#include <string>
#include <memory>
#include <mixer.h>
#include <utility>
#include <vector>
#include <thread>

#include "config_handler.h"
#include "device_handler.h"
#include "common.h"


struct Data
{
    std::shared_ptr<std::mutex> storage_records_lock = std::make_shared<std::mutex>();
    records_storage storage_records = std::make_shared<records_storage::element_type>();
    unsigned int minute_size;

    std::string path_for_records;

    dev_status input_device = {Status::unchanged, ""};
    dev_status output_device = {Status::unchanged, ""};

};

class iChain {

protected:
    std::shared_ptr<Data> data = std::make_shared<Data>();
    std::unique_ptr<iChain> next = nullptr;

    void next_execute_if_exists() const
    {
        if (next)
        {
            return next->execute();
        }
    }
    void next_cancel_if_exists() const
    {
        if (next)
        {
            return next->cancel();
        }
    }


public:
    iChain(std::shared_ptr<Data> d, std::unique_ptr<iChain>& chain): data(std::move(d)), next(chain.release()) {};
    explicit iChain(std::shared_ptr<Data> d): data(std::move(d)) {};

    virtual void execute() = 0;
    virtual void cancel() = 0;

    virtual ~iChain() = default;
};

class ConfigController : public iChain
{
    std::shared_ptr<CoreConfigHandler> config_handler;
    std::string saved_input_device_name, saved_output_device_name;
    void process_device(bool is_input_dev, dev_status& status);

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override { next_cancel_if_exists(); };
};

class DeviceController : public iChain
{

    struct DeviceHandlerInfo
    {
        std::string name;
        std::thread thread;
        std::shared_ptr<std::atomic_bool> stop_flag;
    };

    DeviceHandlerInfo input_device;
    DeviceHandlerInfo output_device;

    std::shared_ptr<Mixer> mix = nullptr;

    void verification(dev_status& new_dev, DeviceHandlerInfo& dev_info) const;

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override ;
};

class SoundSaveController final : public iChain
{
    struct WavHeaders
    {
        uint8_t RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header Magic header
        uint32_t chunk_size{};                     // RIFF Chunk Size
        uint8_t WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header

        /* "fmt" sub-chunk */
        uint8_t fmt[4] = {'f', 'm', 't', ' '}; // FMT header
        uint32_t subchunk_size = 16;           // Size of the fmt chunk
        uint16_t audio_format = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM

        // Mu-Law, 258=IBM A-Law, 259=ADPCM
        uint16_t num_of_channels = NUM_CHANNELS;   // Number of channels 1=Mono 2=Sterio
        uint32_t samples_per_sec = SAMPLE_RATE;   // Sampling Frequency in Hz
        uint32_t bytes_per_sec = SAMPLE_RATE * 2; // bytes per second
        uint16_t block_align = 2;          // 2=16-bit mono, 4=16-bit stereo
        uint16_t bits_per_sample = 16;      // Number of bits per sample

        /* "data" sub-chunk */
        uint8_t subchunk_2_ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
        uint32_t subchunk_2_size{};                        // Sampled data length
    };

    static void read_header(std::ifstream& file, WavHeaders& header);
    static void write_header(std::ofstream& file, WavHeaders& header);

    size_t max_file_size();

    std::string current_file;
    size_t file_size = 0;

    std::string gen_filename();

    void write_sound();

public:
    using iChain::iChain;
    void execute() override;
    void cancel() override;
};