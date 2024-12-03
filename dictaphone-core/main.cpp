//
// Created by ntikhonov on 16.11.24.
//

#include "daemon.h"

int main(int argc, const char* argv[])
{
    const auto daemon = Daemon::create();
    daemon->run(argc, argv);

}