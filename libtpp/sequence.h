#pragma once

#include <vector>
#include <optional>
#include <variant>

#include "helpers/helpers.h"
#include "helpers/helpers_pretty.h"

namespace tpp {

    class SequenceError : public std::runtime_error {
    public:
        SequenceError(std::string const & what): std::runtime_error(what) {}
    }; // tpp::SequenceError

    /** Generic CSI sequence. 
     
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

        static std::optional<CSISequence> Parse(char const * & buffer, char const * end);

    private:
        std::vector<std::optional<int>> args_;
        char suffix_;

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

        TppSequences encode the extra t++ features such as data transmission and terminal multiplexing features. They are sent using the Device Control String (DCS) sequence that starts with ESC P + and is terminated by the string terminator (ESC \).

        The payload of a TPP sequence consists of an integer sequence number, followed by sequence payload, which is decoded based on the sequence kind itself (see details of the respective sequences for their encodings). In general, printable characters and readable representations are preferred. For binary data, non-printable characters (or a subset of those with reduced compatibility) can be quoted with '`' followed by a hexadecimal representation of the byte. '``' is the quote character itself. 

     */
    class TppSequence {

    }; // TppSequence

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
        #include "sequences.inc.h"
        CSISequence,
        DECSequence,
        OSCSequence
    >;

    /** Parses the given buffer for a sequence. 
     
        If the buffer starts with a valid sequence, returns the sequence itself and advances the buffer to the first character after the parsed sequence. 

        If the buffer starts with what appears to be a valid sequence, but ends before the sequence terminates, the function does not change the passed buffer pointer and returns None. 

        In all other cases, the function throws an exception and advances the buffer to the offending character. 
    */
    std::optional<Sequence> ParseSequence(char const * & buffer, char const * end);

} // namespace tpp

