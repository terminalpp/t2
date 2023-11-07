#include "helpers/helpers_pretty.h"
#include "sequence.h"

namespace tpp {

    std::optional<CSISequence> CSISequence::Parse(char const * & buffer, char const * end) {
        if (buffer == end)
            return std::nullopt;
        char const * x = buffer;
        try {
            if (*x != '\033')
                throw SequenceError{STR("Expected sequence start (\\033, ESC) but " << PRETTY(*x) << "found")}; 
            ++x;
            if (x == end)
                return std::nullopt;
            if (*x != '[')
                throw SequenceError{STR("Expected CSI Sequence start ESC [, but ESC " << PRETTY(*x) << "found")};
            ++x;
            CSISequence result;
            while (true) {
                if (x == end)
                    return std::nullopt;
                if (*x == ';') {
                    ++x;
                    result.args_.push_back(std::nullopt);
                } else if (isDecimalDigit(*x)) {
                    int arg = 0;
                    do {
                        arg = (arg * 10) + (*x - '0');
                        if (++x == end)
                            return std::nullopt;
                    } while (isDecimalDigit(*x));
                    result.args_.push_back(arg);
                } else {
                    // can be either unsupported parameter byte, unsupported intermediate bytes, or final bytes
                    break; 
                }
            }
            if (IsFinalByte(*x)) {
                result.suffix_ = *x;
                ++x;
            } else if (IsParameterByte(*x)) {
                throw SequenceError{"Parameter bytes are not supported"};
            } else if (IsIntermediateByte(*x)) {
                throw SequenceError{"Intermediatebytes are not supported"};
            }
            buffer = x;
            return result;
        } catch (...) {
            buffer = x;
            throw;
        }
    }

    std::optional<DECSequence> DECSequence::Parse(char const * & buffer, char const * end) {
        if (buffer == end)
            return std::nullopt;
        char const * x = buffer;
        try {
            if (*x != '\033')
                throw SequenceError{STR("Expected DEC sequence start (\\033, ESC) but " << PRETTY(*x) << "found")}; 
            ++x;
            if (x == end)
                return std::nullopt;
            if (*x != '[')
                throw SequenceError{STR("Expected DEC Sequence start ESC [, but ESC " << PRETTY(*x) << "found")};
            ++x;
            if (x == end)
                return std::nullopt;
            if (*x != '?')
                throw SequenceError{STR("Expected DEC Sequence start ESC [ ?, but ESC [ " << PRETTY(*x) << "found")};
            ++x;
            int id = 0;
            bool idParsed = false;
            while (true) {
                if (x == end)
                    return std::nullopt;
                if (!isDecimalDigit(*x))
                    break;
                id = id * 10 + (*(x++) - '0');
                idParsed = true;                    
            }
            if (!idParsed)
                throw SequenceError{STR("DEC sequence must have an integer id, but " << PRETTY(*x) << " found")};
            switch (*x) {
                case 'h':
                    buffer = x + 1;
                    return DECSequence{id, true};
                case 'l':
                    buffer = x + 1;
                    return DECSequence{id, false};
                default:
                    throw SequenceError{STR("Dec sequence must end with 'h' or 'l', but  " << PRETTY(*x) << " found")};
            }                
        } catch (...) {
            buffer = x;
            throw;
        }
    }

    std::optional<Sequence> ParseSequence(char const * & buffer, char const * end) {
        if (buffer + 3 <= end) {
            if (buffer[1] == '[') {
                if (buffer[2] == '?') {
                    auto seq = DECSequence::Parse(buffer, end);
                    if (!seq.has_value())
                        return std::nullopt;
                    switch (seq->id) {
                        #define DEC(_, NAME, ID) case ID: return NAME{seq.value()};
                        #include "sequences.inc.h"
                        default:
                            return seq.value();
                    }
                } else {
                    auto seq = CSISequence::Parse(buffer, end);
                    if (!seq.has_value())
                        return std::nullopt;
                    switch (seq->suffix()) {
                        #define CSI0(_, NAME, SUFFIX) case SUFFIX: return NAME{std::move(seq.value())}; 
                        #define CSI1(_, NAME, SUFFIX, ...) case SUFFIX: return NAME{std::move(seq.value())}; 
                        #include "sequences.inc.h"
                        default:
                            return seq.value();
                    }
                }
            } else if (buffer[1] == ']') {
                NOT_IMPLEMENTED; // OSR
            } else {
                throw SequenceError{STR("Invalid ANSI escape sequence")};
            }
        } else {
            return std::nullopt;
        }
    }

} // namespace tpp