#pragma once

#include "helpers/helpers.h"
#include "helpers/helpers_pretty.h"

namespace tpp {

    /** A stream the reader can read from */
    class Reader {
    public:

        class EOF : public std::runtime_error {
        public:
            EOF(): std::runtime_error{"Unexpected end of file"} {}
        }; // Reader::EOF

        class Error: public std::runtime_error {
        public:
            Error(std::string const & what): std::runtime_error{what} {}
        }; // Reader::Error

        Reader(char const * buffer, char const * end): buffer_{buffer}, end_{end} {
            ASSERT(end_ >= buffer_);
        }

        Reader(char const * buffer, size_t numBytes): buffer_{buffer}, end_{buffer + numBytes} {}

        /** \name Stream API. 
         */
        //@{
        bool eof() const { return buffer_ >= end_; }

        char top() const {
            if (buffer_ >= end_)
                throw EOF{};
            return *buffer_;
        }

        char pop() {
            if (buffer_ >= end_)
                throw EOF{};
            return *(buffer_++);
        }

        char peek(size_t offset) const {
            if (buffer_ + offset >= end_)
                throw EOF{};
            return *(buffer_ + offset);
        }

        void advance(sie_t offset) {
            ASSERT(buffer_ < end_ && "Running advance on eof'd buffer is not recommended");
            buffer_ += offset;
        }

        //@}

    private:
       char const * buffer_;
       char const * end_;
    }; // tpp::Reader

    class BufferedReader {

    }; // tpp::BufferedReader

    /** A simple byte reader from a buffer. 
     
        Useful for parsing, also an interface specification for other readers. Since reader is to be used in performance critical loops, virtual functions are not used and templates are preffered instead. 
    */
    class Reader {
    public:

        class Error: public std::runtime_error {
        public:
            Error(std::string const & what): std::runtime_error{what} {}
        }; // ByteReader::Error

        ByteReader(char const * buffer, char const * end): buffer_{buffer}, end_{end} {
            ASSERT(end_ >= buffer_);
        }

        ByteReader(char const * buffer, size_t numBytes): buffer_{buffer}, end_{buffer + numBytes} {}

        bool eof() const { return buffer_ >= end_; }

        char top() const {
            if (buffer_ >= end_)
                throw EOF{};
            return *buffer_;
        }

        char pop() {
            if (buffer_ >= end_)
                throw EOF{};
            return *(buffer_++);
        }

        char peek(size_t offset) const {
            if (buffer_ + offset >= end_)
                throw EOF{};
            return *(buffer_ + offset);
        }

        void advance(sie_t offset) {
            ASSERT(buffer_ < end_ && "Running advance on eof'd buffer is not recommended");
            buffer_ += offset;
        }

        template<typename T>
        static void expectPop(T & reader, char expected) {
            if (reader.top() != expected)
                throw Error{"Expected " << PRETTY(expected) << " but " << PRETTY(*buffer_) << " found"};
            return reader.pop();
        }

        template<typename T>
        static bool condPop(T & reader, char expected) {
            if (reader.top() == expected) {
                reader.pop();
                return true;
            } else {
                return false;
            }
        }

    private:

       char const * buffer_;
       char const * end_;
    }; // tpp::ByteReader

} // namespace tpp