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

class LockFile
{
    std::string filename = PROGRAM_ROOT_PATH + "/" + DAEMON_NAME + ".lock";
    int file_descriptor = 0;
    void delete_file() const { std::filesystem::remove(filename); }

public:
    LockFile() = default;
    void lock();
    void unlock() const;
    [[nodiscard]] bool try_lock();
    ~LockFile() {delete_file();}

    pid_t get_pid_from_lockfile ();
};

class Daemon
{
    using DaemonDuration = std::chrono::high_resolution_clock::duration;

    static Daemon* instance;
    LockFile lock_file;
    std::unique_lock<LockFile> lock_file_hnd;

    std::chrono::high_resolution_clock::duration update_duration{};
    std::atomic<bool> m_is_running{false};

    std::condition_variable update_cv;
    std::mutex local_mutex;

    std::int32_t exit_code = EXIT_SUCCESS;
    static pid_t thread_pid;

public:
    static Daemon* create(const DaemonDuration& update_duration = std::chrono::seconds(10));

    virtual ~Daemon()
    {
        Loger::shutdown();
        std::exit(exit_code);
    }

    void run(int argc, const char* argv[]);
    void stop(int code = EXIT_SUCCESS);


private:
    Daemon() : lock_file_hnd(lock_file, std::defer_lock) {}

    void on_start()
    {
        std::ofstream out;
        out.open("/home/nik/test.txt", std::ios::app);
        out << "start" << std::endl;
        out.close();
    };
    void on_update()
    {
        std::ofstream out;
        out.open("/home/nik/test.txt", std::ios::app);
        out << "update" << std::endl;
        out.close();
    };
    void on_stop()
    {
        std::ofstream out;
        out.open("/home/nik/test.txt", std::ios::app);
        out << "end" << std::endl;
        out.close();
    };
    void on_reload() {};

    static void signal_handler(int sig);
    static void daemonize();
    void start_daemon();

};

