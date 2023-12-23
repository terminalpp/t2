#pragma once

#include "helpers/helpers.h"

#include "pty.h"
#include "sequence.h"

namespace tpp::pty {

    /** Buffered reader and decoder of PTY data stream. 
     
        Takes the the stream in and parses its contents on demand. The contents is multiplexed to data and control sequences with automatic buffering. 
     */
    class Reader {
    public:

        Reader(std::unique_ptr<PTY> && pty): pty_{std::move(pty)} {
            ASSERT(buffer_.size() != 0 && "Zero buffer capacity is not possible");
        }

        /** Sends the given buffer to the underlying pty. 
         */
        void send(char const * buffer, size_t numBytes) { pty_->send(buffer, numBytes); }

        /** Receives next data or control sequence from the PTY. 
         
            If there is enough data for a correct answer in the local buffer, returns immediately. Otherwise the function might block by calling pty's receive() to read more data. 
        */
        Sequence receive();


        char top() {

        }

        char pop() {

        }

        char peek(size_t index) {

        }


    private:

        void readNext() {
            // first adjust the buffer position and 
        }

        std::unique_ptr<PTY> pty_;
        std::vector<char> buffer_;
        size_t bufferPos_{0};
    }; // tpp::pty::Reader

} // namespace tpp