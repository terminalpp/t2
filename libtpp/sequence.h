#pragma once

#include <vector>
#include <optional>
#include <variant>

#include "helpers/helpers.h"
#include "helpers/helpers_pretty.h"

#include "reader.h"

namespace tpp {


    class SequenceError : public std::runtime_error {
    public:
        SequenceError(std::string const & what): std::runtime_error{what} {}
    }; // tpp::SequenceError

    /** CSI sequence. 
     
        CSI Sequence is characterized by the prefix ESC [, followed by zero or more semicolon separated integers and terminated by a special character that determines the type of the sequence. This class is a generic representation of any such sequence. 

        The CSI prefix is then followed by any number of parameter bytes (0x30 - 0x3f), followed by any number of intermediate bytes (0x20 - 0x2f) and then by a required final byte (0x40 - 0x7e). 
     */
    class CSISequence {
    public:
        using iterator = std::vector<std::optional<int>>::iterator;
        using const_iterator = std::vector<std::optional<int>>::const_iterator;

        size_t numArgs() const { return args_.size(); }
        iterator begin() { return args_.begin(); }    
        const_iterator begin() const { return args_.begin(); }
        iterator end() { return args_.end(); }
        const_iterator end() const { return args_.end(); }

        char suffix() const { return suffix_; }

        int arg(size_t index, int defaultValue) const {
            if (index >= args_.size())
                return defaultValue;
            auto const & i = args_[index];
            return i.has_value() ? i.value() : defaultValue;
        }

        /** Prettyprints the seuence in human readable form. 
         */
        void prettyPrint(std::ostream & s) const {
            s << "ESC [";
            auto i = args_.begin(), e = args_.end();
            if (i != e) {
                s << ' ';
                if (i->has_value())
                    s << i->value();
                ++i;
                while (i != e) {
                    s << "; ";
                    if (i->has_value())
                        s << i->value();
                    ++i;
                }
            }
            s << ' ' << suffix_;
        }

        friend std::ostream & operator << (std::ostream & s, CSISequence const & seq) {
            s << "\033[";
            auto i = seq.begin(), e = seq.end();
            if (i != e) {
                if (i->has_value())
                    s << i->value();
                ++i;
                while (i != e) {
                    s << ";";
                    if (i->has_value())
                        s << i->value();
                    ++i;
                }
            }
            s << seq.suffix();
            return s;
        }

        static std::optional<CSISequence> Parse(char const * & buffer, char const * end);

        template<typename T>
        static std::optional<CSISequence> Parse(T const & reader) {
            try {

            } catch (Reader::EOF)
        }

    private:
        std::vector<std::optional<int>> args_;
        char suffix_;

        static bool IsParameterByte(char c) { return c >= 0x30 && c <= 0x3f; }
        static bool IsIntermediateByte(char c) { return c >= 0x20 && c <= 0x2f; }
        static bool IsFinalByte(char c) { return c >= 0x40 && c <= 0x7f; }

    }; 

    /** DECSET and DECCLR sequences

        The DEC sequences all follow the same format, where ESC [ ? is followed by a single number and then either 'h' or 'l' as the final character. Their purpose is to enable or disable certain terminal features.  
     */
    class DECSequence {
    public:
        int id;
        bool value; 

        void prettyPrint(std::ostream & s) const {
            s << "ESC [ ? " << id << (value ? 'h' : 'l');
        }

        friend std::ostream & operator << (std::ostream & s, DECSequence seq) {
            s << "\033[?" << seq.id << (seq.value ? 'h' : 'l');
            return s;
        }        
        
        static std::optional<DECSequence> Parse(char const * & buffer, char const * end);

    }; // DECSequence

    /** OSCSequences

        OSC sequences start with ESC ], followed by an optional integer. If a number is provided, it must be separated by ';' from the payload, which consists of semicolon separated strings. End of payload is signalled by either ST (ESC \) or a BEL. 
     */
    class OSCSequence {
    public:
        std::optional<int> id;
        std::vector<std::string> values;

        void prettyPrint(std::ostream & s) const {
            s << "ESC ] ";
            if (id.has_value())
                s << id.value() << ";";
            auto i = values.begin(), e = values.end();
            if (i != e) {
                s << *i++;
                while (i != e)
                    s << ';' << *i++;
            }
            s << " BEL";
        }

        friend std::ostream & operator << (std::ostream & s, OSCSequence const & seq) {
            s << "\033]";
            if (seq.id.has_value())
                s << seq.id.value() << ";";
            auto i = seq.values.begin(), e = seq.values.end();
            if (i != e) {
                s << *i++;
                while (i != e)
                    s << ';' << *i++;
            }
            s << '\b';
            return s;
        }

        static std::optional<OSCSequence> Parse(char const * & buffer, char const * end); 

    }; // OSCSequence

    /** Terminal++ Special Sequences

        TppSequences encode the extra t++ features such as data transmission and terminal multiplexing features. Terminal++ sequences hijack the existing Device Control Strings (DCS) escape sequences so that they will be ignored or passed through by non-compliant apps. 

        Each t++ sequence has the following form:

            ESC P id + t payload ESC \

        Where `id` is the identifier of the sequence transmitted and `payload` is the payload of the sequence. The Sequence identifier also describes the encoding used in the payload section, which generally should follow the DCS sequences, i.e. multiple strings separated by semicolons. 

        Non-printable or semantically clashing payload bytes can be encoded using a simple scheme where a byte is encoded as backtick followed by a hexadecimal representation of the encoded byte. 
     */
    class TppSequence {
    public:
        int id;
        std::vector<std::string> args;

        void prettyPrint(std::ostream & s) const {
            s << "ESC P " << id << 't';
            auto i = args.begin(), e = args.end();
            if (i != e) {
                Encode(s, *i++);
                while (i != e) {
                    s << ';';
                    Encode(s, *i++);
                }
            }
            s << " ST";
        }

        friend std::ostream & operator << (std::ostream & s, TppSequence const & seq) {
            s << "\033P" << seq.id << 't';
            auto i = seq.args.begin(), e = seq.args.end();
            if (i != e) {
                Encode(s, *i++);
                while (i != e) {
                    s << ';';
                    Encode(s, *i++);
                }
            }
            s << "\033\\";
            return s;
        }

        static std::optional<TppSequence> Parse(char const * & buffer, char const * end);
        
        static void Encode(std::ostream &s, std::string const & value);

    protected:

        #define TPP2(_, NAME, ...) friend class NAME;
        #include "sequences.inc.h"

        TppSequence(int id): id{id} {}

        template<typename T>
        static std::optional<T> parseArg(char const * & buffer, char const * end); 
        static std::optional<bool> parseSeparator(char const * & buffer, char const * end);
        static std::optional<bool> parseEnd(char const * & buffer, char const * end);

    }; // TppSequence

    template<>
    inline std::optional<int> TppSequence::parseArg<int>(char const * & buffer, char const * end) {
        int result = 0;
        char const * x = buffer;
        while (true) {
            if (x == end)
                return std::nullopt;
            if (isDecimalDigit(*x))
                result = (result * 10) + (*(x++) - '0');
            else
                break;
        }
        buffer = x;
        return result;
    }

    template<>
    inline std::optional<std::string> TppSequence::parseArg<std::string>(char const * & buffer, char const * end) {
        std::stringstream s;
        char const * x = buffer;
        while (x < end) {
            if (*x == ';' || *x == '\033') {
                buffer = x;
                return s.str();
            }
            if (*x == '`') {
                if (x + 2 >= end)
                    break;
                ++x;
                char c = (hexToNibble(*x++) << 4);
                c |= hexToNibble(*x++);
                s << c;
            } else {
                s << *x++;
            }

        }
        return std::nullopt;
    }

    #define CSI0(SHORTHAND, NAME, SUFFIX) \
        class NAME { \
        public: \
            static constexpr char Suffix = SUFFIX; \
            NAME(CSISequence && seq) { \
                if (seq.numArgs() != 0) \
                    throw SequenceError{STR("Non zero arguments for CSI sequence " << PRETTY(seq) << " when converting to SHORTHAND")}; \
                if (seq.suffix() != Suffix) \
                    throw SequenceError{STR("Invalid suffix for CSI sequence " << PRETTY(seq) << " when converting to SHORTHAND (suffix" << SUFFIX << ")")}; \
            } \
        }; 

    #define CSI1(SHORTHAND, NAME, SUFFIX, VALUE_NAME, DEFAULT_VALUE) \
        class NAME { \
        public: \
            static constexpr char Suffix = SUFFIX; \
            int VALUE_NAME; \
            NAME(CSISequence && seq) { \
                if (seq.numArgs() > 1) \
                    throw SequenceError{STR("Invalid number of arguments for for CSI sequence " << PRETTY(seq) << " when converting to SHORTHAND")}; \
                if (seq.suffix() != Suffix) \
                    throw SequenceError{STR("Invalid suffix for CSI sequence " << PRETTY(seq) << " when converting to SHORTHAND (suffix" << SUFFIX << ")")}; \
                VALUE_NAME = seq.arg(0, DEFAULT_VALUE); \
            } \
        }; 

    #define CSI2(SHORTHAND, NAME, SUFFIX, VALUE_NAME1, DEFAULT_VALUE1, VALUE_NAME2, DEFAULT_VALUE2) \
        class NAME { \
        public: \
            static constexpr char Suffix = SUFFIX; \
            int VALUE_NAME1; \
            int VALUE_NAME2; \
            NAME(CSISequence && seq) { \
                if (seq.numArgs() > 2) \
                    throw SequenceError{STR("Invalid number of arguments for for CSI sequence " << PRETTY(seq) << " when converting to SHORTHAND")}; \
                if (seq.suffix() != Suffix) \
                    throw SequenceError{STR("Invalid suffix for CSI sequence " << PRETTY(seq) << " when converting to SHORTHAND (suffix" << SUFFIX << ")")}; \
                VALUE_NAME1 = seq.arg(0, DEFAULT_VALUE1); \
                VALUE_NAME2 = seq.arg(1, DEFAULT_VALUE2); \
            } \
        }; 

    #define DEC(SHORTHAND, NAME, ID) \
        class NAME { \
        public: \
            static constexpr int Id = ID; \
            bool value; \
            NAME(DECSequence seq): \
                value{seq.value} { \
                if (seq.id != Id) \
                    throw SequenceError{STR("Invalid id for DEC sequence " << PRETTY(seq) << " when converting to SHORTHAND (index " << Id << ")")}; \
            } \
        };

    #define OSC1(SHORTHAND, NAME, ID, VALUE_NAME) \
        class NAME { \
        public: \
            static constexpr int Id = ID; \
            std::string VALUE_NAME; \
            NAME(OSCSequence && seq) { \
                if (seq.id.value() != Id) \
                    throw SequenceError{STR("Invalid id for OSC sequence " << PRETTY(seq) << " when converting to SHORTHAND (index " << Id << ")")}; \
                if (seq.values.size() != 1) \
                    throw SequenceError{STR("Invalid number of arguments: " << PRETTY(seq) << " provides " << seq.values.size() << " but only 1 expected")}; \
                VALUE_NAME = std::move(seq.values[0]); \
            } \
        };

    #define OSC2(SHORTHAND, NAME, ID, VALUE_NAME1, VALUE_NAME2) \
        class NAME { \
        public: \
            static constexpr int Id = ID; \
            std::string VALUE_NAME1; \
            std::string VALUE_NAME2; \
            NAME(OSCSequence && seq) { \
                if (seq.id.value() != Id) \
                    throw SequenceError{STR("Invalid id for OSC sequence " << PRETTY(seq) << " when converting to SHORTHAND (index " << Id << ")")}; \
                if (seq.values.size() != 2) \
                    throw SequenceError{STR("Invalid number of arguments: " << PRETTY(seq) << " provides " << seq.values.size() << " but only 1 expected")}; \
                VALUE_NAME1 = std::move(seq.values[0]); \
                VALUE_NAME1 = std::move(seq.values[1]); \
            } \
        };

    #define TPP2(SHORTHAND, NAME, ID, VALUE_NAME1, VALUE_TYPE1, VALUE_NAME2, VALUE_TYPE2) \
        class NAME { \
        public: \
            static constexpr int Id = ID; \
            VALUE_TYPE1 VALUE_NAME1; \
            VALUE_TYPE2 VALUE_NAME2; \
            NAME(VALUE_TYPE1 VALUE_NAME1, VALUE_TYPE2 VALUE_NAME2): VALUE_NAME1{VALUE_NAME1}, VALUE_NAME2{VALUE_NAME2} {} \
            static std::optional<NAME> parseBody(char const * & buffer, char const * end) { \
                char const * x = buffer; \
                try { \
                    auto first{TppSequence::parseArg<VALUE_TYPE1>(x, end).value()}; \
                    TppSequence::parseSeparator(x, end).value(); \
                    auto second{TppSequence::parseArg<VALUE_TYPE2>(x, end).value()}; \
                    TppSequence::parseEnd(x, end).value(); \
                    buffer = x; \
                    return NAME{first, second}; \
                } catch (std::bad_optional_access const &) { \
                    return std::nullopt; \
                } catch (...) { \
                    buffer = x; \
                    throw; \
                } \
            } \
        }; 
        
    #include "sequences.inc.h"

    /** Union of all known sequences. 
     
        See the `sequences.inc.h` for more details about the sequences supported. The union contains both specific sequences defined therein and generic sequences for which no special type has been created, which is useful for working with syntactically valid sequences of unknown semantics. 
     */
    using Sequence = std::variant<
        #define CSI0(_, NAME, ...) NAME,
        #define CSI1(_, NAME, ...) NAME, 
        #define CSI2(_, NAME, ...) NAME, 
        //#define CSIn(_, NAME, ...) NAME,
        #define DEC(_, NAME, ...) NAME, 
        #define OSC1(_, NAME, ...) NAME, 
        #define OSC2(_, NAME, ...) NAME,
        #define TPP2(_, NAME, ...) NAME, 
        #include "sequences.inc.h"
        CSISequence,
        DECSequence,
        OSCSequence,
        TppSequence,
        std::string
    >;

    /** Parses the given buffer for a sequence. 
     
        If the buffer starts with a valid sequence, returns the sequence itself and advances the buffer to the first character after the parsed sequence. 

        If the buffer starts with what appears to be a valid sequence, but ends before the sequence terminates, the function does not change the passed buffer pointer and returns None. 

        In all other cases, the function throws an exception and advances the buffer to the offending character. 
    */
    std::optional<Sequence> ParseSequence(char const * & buffer, char const * end);

} // namespace tpp

