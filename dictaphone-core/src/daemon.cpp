//
// Created by ntikhonov on 21.11.24.
//

#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <csignal>
#include <sys/stat.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>

#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/wait.h>

#include "daemon.h"
#include "controllers/sound.h"
#include "controllers/device.h"


Daemon* Daemon::instance = nullptr;


bool LockFile::try_lock()
{
    try
    {
        lock();
    }
    catch (std::system_error& e)
    {
        return false;
    }
    return true;
}

void LockFile::lock()
{
    file_descriptor = open(filename.c_str(), O_RDWR|O_CREAT, 0644);

    if (file_descriptor == -1)
    {
        std::cerr << "Не удалось создать файл " << filename << ". Попробуйте еще раз" << std::endl;
        delete_file();
        exit(-1);
    }

    if (flock(file_descriptor, LOCK_EX|LOCK_NB) == -1)
    {
        close(file_descriptor);
        throw std::system_error();
    }
    auto cpid = std::to_string(getpid());
    cpid.resize(file_length, ' ');
    write(file_descriptor, cpid.c_str(), file_length);
    Loger::info("Блокировка успешно создана для потока " + cpid);
}

void LockFile::unlock() const
{
    if (flock(file_descriptor, LOCK_UN) == -1)
    {
        Loger::error("Не удалось разблокировать файл " + filename);
        throw std::system_error();
    }
    close(file_descriptor);
    delete_file();
    Loger::info("Блокировка удалена");
}

int LockFile::get_pid_from_lockfile()
{
    file_descriptor = open(filename.c_str(), O_RDONLY, 0444);
    if (file_descriptor == -1)
    {
        exit(EXIT_FAILURE);
    }

    std::string cpid(file_length, ' ');
    const auto read_size = read(file_descriptor, cpid.data(), file_length);
    if (read_size <= 0)
    {
        Loger::error("Неизвестен pid демона");
        close(file_descriptor);
        exit(EXIT_FAILURE);
    }
    close(file_descriptor);
    auto pid = std::stoi(cpid);
    return pid;

}


Daemon* Daemon::create(const DaemonDuration& update_duration)
{
    if (!instance)
    {
        instance = new Daemon();
        instance->update_duration = update_duration;
    }
    else
    {
        Loger::warning("Попытка запуска второго демона " + DAEMON_NAME);
        std::exit(EXIT_FAILURE);
    }
    return instance;
}

void Daemon::signal_handler(const int sig)
{
    Loger::info("Получен сигнал " + std::to_string(sig));
    switch (sig)
    {
        // daemon.service handler: ExecStop=/bin/kill -s SIGTERM $MAINPID
        // When daemon is stopped, system sends SIGTERM first, if daemon didn't respond during 90 seconds, it will send a SIGKILL signal
    case SIGTERM:
    case SIGINT:
    case SIGKILL:
        {
            instance->stop();
            break;
        }
    default:
        break;
    }
}

void Daemon::daemonize()
{
    auto pid = getpid();
    thread_pid = fork();
    if (thread_pid > 0)
    {
        Loger::info("Создан отдельный поток для демона " + std::to_string(thread_pid) + " - " + std::to_string(pid));
        std::exit(EXIT_SUCCESS);
    }
    else if (thread_pid < 0)
    {
        std::exit(EXIT_FAILURE);
    }

    umask(0);

    pid_t sid = setsid();
    if (sid < 0)
    {
        Loger::error(" Не удалось установить SID для дочернего процесса: " + std::string(std::strerror(errno)));
        std::exit(EXIT_FAILURE);
    }

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void Daemon::stop(const int code)
{
    Loger::info("Демон завершает работу");
    exit_code = code;
    m_is_running.store(false);
    update_cv.notify_all();
}

void Daemon::start_daemon()
{
    if (m_is_running.load())
    {
        Loger::info("Демон " + DAEMON_NAME + " уже запущен");
        return;
    }

    daemonize();
    if(!lock_file_hnd.try_lock())
    {
        Loger::info("Демон " + DAEMON_NAME + " уже запущен");
        return;
    }

    m_is_running = true;

    Loger::info("Демон " + DAEMON_NAME + " успешно запущен");

    on_start();
    while (m_is_running.load())
    {
        on_update();
        std::unique_lock lock(update_local_mutex);
        update_cv.wait_for(lock, update_duration, [this]()
        {
            return !m_is_running.load();
        });
    }
    on_stop();
    lock_file_hnd.unlock();
    Loger::info("Демон завершил работу");
}

void Daemon::run(int argc, const char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Необходимо передать один из параметров: --start, --stop, --reload" << std::endl;
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1], "--start") == 0)
    {
        start_daemon();
    }
    else if(strcmp(argv[1], "--stop") == 0)
    {
        if(!lock_file_hnd.try_lock())
        {
            kill(lock_file.get_pid_from_lockfile(), SIGTERM);
        }
    }
    else if(strcmp(argv[1], "--reload") == 0)
    {
        if(!lock_file_hnd.try_lock())
        {
            kill(lock_file.get_pid_from_lockfile(), SIGTERM);
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        start_daemon();
    }
    else if(strcmp(argv[1], "--status") == 0)
    {
        if(lock_file_hnd.try_lock())
        {
            std::cout << NONE_DEVICE << std::endl;
        }
        else
        {
            std::cout << lock_file.get_pid_from_lockfile() << std::endl;
        }
    }
    else
    {
        std::cerr << "Необходимо передать один из параметров: --start, --stop, --reload" << std::endl;
        exit(EXIT_FAILURE);
    }

}

void Daemon::on_start()
{
    data = std::make_shared<Data>();
    start = std::make_unique<SoundSaveController>(data);
    start = std::make_unique<DeviceController>(data, start);
    start = std::make_unique<ConfigController>(data, start);
}

void Daemon::on_update() const
{
    start->execute();
}

void Daemon::on_stop() const
{
    start->cancel();
}
