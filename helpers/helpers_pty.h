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

#include "helpers.h"

/** Basic interface of a pseudoterminal client. 

    Provides simple blocking API for sending and receiving on the pseudoconsole. This leaves the users of the PTY to ensure proper multi-threaded handling, where applicable. 
    */
class PTY {
public:

    struct Size {
        int cols;
        int rows;
    }; // PTY::Size

    struct Done{};

    using ReceiveResult = std::variant<Size, size_t, Done>;

    /** Sends data to the pseudoterminal. 
     */
    virtual void send(char const * buffer, size_t numBytes) = 0;

    /** Reads from the pseudoterminal, blocking the caller thread. 
     
     */
    virtual ReceiveResult receive(char * buffer, size_t numBytes) = 0;

    /** Returns the pseudoterminal's dimensions. 
     */
    virtual Size size() const = 0; 

    virtual ~PTY() = default;

}; // t2::PTY

inline std::ostream & operator << (std::ostream & s, PTY::Size const & size) {
    s << "[" << size.cols << ";" << size.rows << "]";
    return s;
}

#if (defined ARCH_UNIX)
/** Standard input and output backed pseudoterminal on unix systems. 
 
    As the pseudoterminal on linux connects to the standard file input and output, there can be only one PTY per process and the LocalPTY class is thus a singleton. 

    For data sending, the default write is used, throwing an OSError upon failure. Receiving is more interesting since we need to return both incoming data sent on the stdin as well as terminal size updates received via the SIGWINCH signal.

 */
class LocalPTY : PTY {
public:

    LocalPTY() {
        ASSERT(pipe_[0] == 0 && pipe_[1] == 0 && "LocalPTY is singleton");
        // terminal setup on the stdin file - first get backup of the tc attrs to be restored when the PTY dies so that we do not leave the the pty in some weird state, then set up own needs such as disabling canonical and echo modes, and so on
        OSCHECK(tcgetattr(STDIN_FILENO, & backup_) == 0);
        termios raw = backup_;
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        OSCHECK(tcsetattr(STDIN_FILENO, TCSAFLUSH, & raw) == 0);
        // create the pipe
        OSCHECK(pipe(pipe_) == 0);
        // install the SIGWINCH signal handler and block its processing
        struct sigaction sa;
        sa.sa_flags = 0;
        sa.sa_handler = SIGWINCH_handler;
        sigemptyset(&sa.sa_mask);
        OSCHECK(sigaction(SIGWINCH, &sa, nullptr) == 0);
    }        

    //TODO make the destructor thread-safe by first terinating the receiver that might run in other thread. DO I need to? Or maybe kill stdinfileno?
    ~LocalPTY() override {
        // tell the potential receiver thread, no need to throw errors from destructor
        ::write(pipe_[1], & TERMINATE_EVENT, 1);
        close(pipe_[1]);
        // unregister the SIGWINCH handler (with no active PTY there is no need to react to the signal)
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler = SIG_DFL;
        sa.sa_flags = 0;        
        sigaction(SIGWINCH, &sa, nullptr);        
        // restore the terminal settings from the backup we took when creating the pty
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &backup_);
    }

    /** Sends the given buffer to the terminal. 
     
        This is the simpler method, simply calls the write syscall. Throws OSError when the call fails, such as 
    */
    void send(char const * buffer, size_t numBytes) override {
        OSCHECK(::write(STDOUT_FILENO, buffer, numBytes) == static_cast<int>(numBytes));
    }

    /** Reads from the pseudoterminal, blocking the thread.
     */
    ReceiveResult receive(char * buffer, size_t numBytes) override {
        while (true) {
            fd_set rd;
            FD_ZERO(&rd);
            FD_SET(STDIN_FILENO, &rd);
            FD_SET(pipe_[0], &rd);
            int max_fd = std::max(STDIN_FILENO, pipe_[0]) + 1;

            OSCHECK(select(max_fd, &rd, nullptr, nullptr, nullptr) >= 0);
            if (FD_ISSET(pipe_[0], &rd)) {
                char x;
                OSCHECK(::read(pipe_[0], & x, 1) == 1);
                switch (x) {
                    case RESIZE_EVENT:
                        return size();
                    case TERMINATE_EVENT:
                        // close the receiving end, reset the pipe descriptors
                        OSCHECK(close(pipe_[0]) == 0);
                        pipe_[0] = 0;
                        pipe_[1] = 0;
                        return Done{};
                }
            }
            if (FD_ISSET(STDIN_FILENO, &rd))
                return static_cast<size_t>(::read(STDIN_FILENO, buffer, numBytes));
        }
    }

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

    

}; // LocalPTY

#endif
