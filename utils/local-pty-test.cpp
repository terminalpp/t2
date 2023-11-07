#include <thread>
#include "helpers/helpers_pty.h"

int main() {

    LocalPTY * pty = new LocalPTY();

    std::thread t([pty] () {
        sleep(10);
        std::cout << "Terminating PTY\r" << std::endl; 
        delete pty;
        
    });
    char buffer[128];
    bool keepOn = true;
    while (keepOn) {
        auto x = pty->receive(buffer, sizeof(buffer));
        std::visit(overloaded{
            [&](size_t bytes) {
                std::cout << "Received " << bytes << " bytes\r" << std::endl;
                switch (buffer[0]) {
                    case 'q':
                        keepOn = false;
                        break;
                }
            },
            [](PTY::Size size) {
                std::cout << "Terminal resized to " << size << "\r" << std::endl;
            },
            [&](PTY::Done) {
                keepOn = false;
            }
        }, x);
    }
    std::cout << "PTY finished\r" << std::endl;
    t.join();
    std::cout << "PTY terminator thread done" << std::endl;
    return EXIT_SUCCESS;
}