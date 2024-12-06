//
// Created by ntikhonov on 05.12.24.
//

#pragma once
#include <list>
#include <string>
#include <memory>
#include <mixer.h>
#include <utility>
#include <vector>

#include "config_handler.h"
#include "common.h"

/**
 * Общие данные передаваемые между классами цепочки
 */
struct Data
{
    std::shared_ptr<std::mutex> storage_records_lock = std::make_shared<std::mutex>();
    records_storage storage_records = std::make_shared<records_storage::element_type>(); // Звук который нужно записать в файл
    unsigned int minute_size;

    std::string path_for_records;

    dev_status input_device = {Status::unchanged, ""};
    dev_status output_device = {Status::unchanged, ""};

};

/**
 * Интерфейс для элементов цепочки
 */
class iChain
{

protected:
    std::shared_ptr<Data> data;
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
