#include "sequence.h"

#include "pty.h"



namespace tpp::pty {

#if (defined ARCH_UNIX)

    LocalClient::LocalClient() {
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

    LocalClient::~LocalClient() {
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

    size_t LocalClient::receive(char * buffer, size_t bufferLength) {
        ASSERT(bufferLength >= sizeof(TerminalResize) && "Buffer must be big enough for at least TerminalResize sequence");
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
                    case RESIZE_EVENT: {
                        auto s{size()};
                        new (buffer) TerminalResize{s.first, s.second};
                        return sizeof(TerminalResize);
                    }
                    case TERMINATE_EVENT:
                        // close the receiving end, reset the pipe descriptors
                        OSCHECK(close(pipe_[0]) == 0);
                        pipe_[0] = 0;
                        pipe_[1] = 0;
                        return 0;
                }
            }
            if (FD_ISSET(STDIN_FILENO, &rd))
                return static_cast<size_t>(::read(STDIN_FILENO, buffer, bufferLength));
        }
    }

#endif // ARCH_UNIX
} // namespace tpp::pty
