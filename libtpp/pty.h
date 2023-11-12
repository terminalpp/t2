#pragma once

#if (defined ARCH_UNIX)
    #include <termios.h>
    #include <pthread.h>
    #include <unistd.h>
    #include <signal.h>
    #include <sys/wait.h>
    #include <sys/ioctl.h>
    #include <errno.h>
    #if (defined ARCH_LINUX)
        #include <pty.h>
    #elif (defined ARCH_MACOS)
        #include <util.h>
    #endif
#endif

#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <variant>

#include "helpers/helpers.h"

namespace tpp {
    namespace pty {

        /** Pseudoterminal window size. Described in number of rows and columns per row. 
         */
        struct Size {
            int cols;
            int rows;
        }; // tpp::pty::Size

        /** A pseudoterminal endpoint that simply defines API for both sending and receiving data to and from the pseudoterminal. 
         */
        class Endpoint {
        public:
            virtual ~Endpoint();
            virtual void send(char const * buffer, size_t numBytes) = 0;
            virtual size_t receive(char * buffer, size_t bufferLength) = 0;
            virtual Size size() const = 0;
        }; // tpp::pty::Endpoint

        /** Local pseudoterminal client (app). 
         
            Encapsulates pseudoterminal connection via the standard operating system mode, such as the stdin file and terminal resize signal on Linux. 
         */
        class LocalClient : public Endpoint {
#if (defined ARCH_UNIX)
        public:

            LocalClient();
            ~LocalClient() override;

            void send(char const * buffer, size_t numBytes) override {
                OSCHECK(::write(STDOUT_FILENO, buffer, numBytes) == static_cast<int>(numBytes));
            }

            size_t receive(char * buffer, size_t bufferLength) override;

            Size size() const override {
                winsize size;
                OSCHECK(ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != -1);
                return Size{size.ws_col, size.ws_row};
            }

            /** Check if the current process is running inside a tmux. 
             */
            static bool InsideTmux() {
                return getenv("TMUX") != nullptr;
            }

        private:

            static inline char const RESIZE_EVENT = 1;
            static inline char const TERMINATE_EVENT = 2;

            static void SIGWINCH_handler([[maybe_unused]] int sig) { 
                OSCHECK(::write(pipe_[1], & RESIZE_EVENT, 1) == 1);
            }

            /** Backup terminal settings to be restored when the local PTY is destroyed. 
             */
            static inline termios backup_;

            static inline int pipe_[2] = {0,0};
#endif // ARCH_UNIX
        }; // tpp::pty::LocalClient

    } // namespace pty::tpp
} // namespace tpp