//
// Created by ntikhonov on 21.11.24.
//

#pragma once

#include <chrono>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <filesystem>

#include "common.h"
#include "controllers/config.h"

/**
 * Блокировка не позволяющая запустить два экземпляра демона одновременно. Сделана для использования вместо мьютекса
 */
class LockFile
{
    std::string filename = PROGRAM_ROOT_PATH + "/" + DAEMON_NAME + ".lock"; // файл блокировки
    int file_descriptor = 0;
    const int file_length = 10;
    void delete_file() const { std::filesystem::remove(filename); }

public:
    LockFile() = default;
    void lock();
    void unlock() const;
    [[nodiscard]] bool try_lock();

    int get_pid_from_lockfile ();
};

/**
 *  Демон синглтон для записи звука в фане
 */
class Daemon
{
    using DaemonDuration = std::chrono::high_resolution_clock::duration;

    static Daemon* instance;
    LockFile lock_file;
    std::unique_lock<LockFile> lock_file_hnd;

    std::chrono::high_resolution_clock::duration update_duration{};
    std::atomic<bool> m_is_running{false};
    std::atomic<bool> m_is_stop{false};

    std::condition_variable update_cv;
    std::mutex update_local_mutex;

    std::int32_t exit_code = EXIT_SUCCESS;

    std::unique_ptr<iChain> start;
    std::shared_ptr<Data> data;
    pid_t thread_pid;

public:
    static Daemon* create(const DaemonDuration& update_duration = std::chrono::seconds(10));

    virtual ~Daemon()
    {
        std::exit(exit_code);
    }

    void run(int argc, const char* argv[]);
    void stop(int code = EXIT_SUCCESS);


private:
    Daemon() : lock_file_hnd(lock_file, std::defer_lock) {}

    void on_start();
    void on_update() const;
    void on_stop() const;


    static void signal_handler(int sig);
    void daemonize();
    void start_daemon();

};

