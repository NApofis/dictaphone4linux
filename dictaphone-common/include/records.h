//
// Created by ntikhonov on 19.11.24.
//

#pragma once
#include <chrono>
#include <string>
#include <memory>


namespace records
{
    unsigned int delete_records_after(const std::shared_ptr<std::string>& path, const std::chrono::system_clock::time_point* time);
    unsigned int delete_records_before(const std::string& path, const std::chrono::system_clock::time_point* time);
}