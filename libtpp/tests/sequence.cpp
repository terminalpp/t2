#include "helpers/helpers_tests.h"
#include "libtpp/sequence.h"

using namespace tpp;

TEST(CSISequence, ParseIncomplete) {
    char const * buffer = "";
    char const * x = buffer;
    auto r = CSISequence::Parse(x, buffer);
    EXPECT(!r.has_value());
    EXPECT(x == buffer);

    buffer = "\033";
    x = buffer;
    r = CSISequence::Parse(x, buffer);
    EXPECT(!r.has_value());
    EXPECT(x == buffer);

    buffer = "\033[";
    x = buffer;
    r = CSISequence::Parse(x, buffer);
    EXPECT(!r.has_value());
    EXPECT(x == buffer);

    buffer = "\033[56";
    x = buffer;
    r = CSISequence::Parse(x, buffer);
    EXPECT(!r.has_value());
    EXPECT(x == buffer);

    buffer = "\033[56;";
    x = buffer;
    r = CSISequence::Parse(x, buffer);
    EXPECT(!r.has_value());
    EXPECT(x == buffer);

    buffer = "\033[56;;8";
    x = buffer;
    r = CSISequence::Parse(x, buffer);
    EXPECT(!r.has_value());
    EXPECT(x == buffer);
}

TEST(CSISequence, Invalid) {
    std::string buffer("h");
    char const * x = buffer.c_str();
    EXPECT_THROWS(SequenceError, CSISequence::Parse(x, buffer.c_str() + 1));
}

TEST(CSISequence, ValidNoArgs) {
    std::string buffer{"\033[a"};
    char const * x = buffer.c_str();
    auto r = CSISequence::Parse(x, x + buffer.size());
    EXPECT(r.has_value());
    EXPECT(STR(r.value()), buffer);
    EXPECT(r->numArgs(), (size_t)0);
    EXPECT(x == buffer.c_str() + buffer.size());
    buffer = "\033[atest";
    x = buffer.c_str();
    r = CSISequence::Parse(x, x + buffer.size());
    EXPECT(r.has_value());
    EXPECT(STR(r.value()), "\033[a");
    EXPECT(r->numArgs(), (size_t)0);
    EXPECT(x == buffer.c_str() + buffer.size() - 4); // test
}

TEST(CSISequence, CSI0Sequences) {
    #define CSI0(_, NAME, SUFFIX) { \
        std::string buffer{STR("\033[" << SUFFIX)}; \
        char const * x = buffer.c_str(); \
        auto r = ParseSequence(x, x + buffer.size()); \
        EXPECT(r.has_value()); \
        EXPECT(x == buffer.c_str() + buffer.size()); \
        EXPECT(std::holds_alternative<NAME>(r.value())); \
    }
    #include "libtpp/sequences.inc.h"
} 

TEST(CSISequence, CSI1Sequences) {
    #define CSI1(_, NAME, SUFFIX, ARG_NAME, DEFAULT_VALUE) { \
        std::string buffer{STR("\033[" << SUFFIX)}; \
        char const * x = buffer.c_str(); \
        auto r = ParseSequence(x, x + buffer.size()); \
        EXPECT(r.has_value()); \
        EXPECT(x == buffer.c_str() + buffer.size()); \
        EXPECT(std::holds_alternative<NAME>(r.value())); \
        auto seq = std::get<NAME>(r.value()); \
        EXPECT(seq.ARG_NAME == DEFAULT_VALUE); \
    }
    #include "libtpp/sequences.inc.h"
}

TEST(DECSequence, DECSequencesHi) {
    #define DEC(_, NAME, ID) { \
        std::string buffer{STR("\033[?" << ID << "h")}; \
        char const * x = buffer.c_str(); \
        auto r = ParseSequence(x, x + buffer.size()); \
        CHECK(r.has_value()); \
        EXPECT(x == buffer.c_str() + buffer.size()); \
        EXPECT(std::holds_alternative<NAME>(r.value())); \
        auto seq = std::get<NAME>(r.value()); \
        EXPECT(seq.value == true); \
    }
    #include "libtpp/sequences.inc.h"
}

TEST(DECSequence, DECSequencesLo) {
    #define DEC(_, NAME, ID) { \
        std::string buffer{STR("\033[?" << ID << "l")}; \
        char const * x = buffer.c_str(); \
        auto r = ParseSequence(x, x + buffer.size()); \
        CHECK(r.has_value()); \
        EXPECT(x == buffer.c_str() + buffer.size()); \
        EXPECT(std::holds_alternative<NAME>(r.value())); \
        auto seq = std::get<NAME>(r.value()); \
        EXPECT(seq.value == false); \
    }
    #include "libtpp/sequences.inc.h"
}
