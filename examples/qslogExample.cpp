#define QSLOG_TAG "example"

#include "qslog/CompressFileSink.h"
#include "qslog/FileSink.h"
#include "qslog/Logger.h"
#include "qslog/StdoutSink.h"

int main() {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    auto fileSink = std::make_shared<qslog::FileSink>("FileSink", "fileSink.log", true);
    auto binSink = std::make_shared<qslog::CompressFileSink>("CompressFileSink", "binSink.log", true);
    qslog::Logger::addSink(consoleSink);
    qslog::Logger::addSink(fileSink);
    qslog::Logger::addSink(binSink);
    bool b = true;
    char c = 'c';
    uint8_t u8 = 255;
    uint16_t u16 = 65535;
    uint32_t u32 = 4294967295u;
    uint64_t u64 = 18446744073709551615u;
    int8_t i8 = INT8_MIN;
    int16_t i16 = INT16_MIN;
    int32_t i32 = INT32_MIN;
    int64_t i64 = INT64_MIN;
    float f = 3.1415926f;
    double d = 3.1415926;
    std::string str = "hello std str";
    const char *p = "hello c str";
    char cArray[]{"hello c array\0"};
    int *pInt = nullptr;
    qslog::BaseSink *sink = consoleSink.get();

    QSLOGI("bool test : {}", b);
    QSLOGW("char test : {}", c);
    QSLOGE("uint8 test : {}", u8);
    QSLOGF("uint16 test : {}", u16);
    QSLOGD("uint32 test : {}", u32);
    QSLOGF("uint64 test : {}", u64);
    QSLOGI("int8 test : {}", i8);
    QSLOGW("int16 test : {}", i16);
    QSLOGI("int32 test : {}", i32);
    QSLOGF("int64 test : {}", i64);
    QSLOGI("float test : {}", f);
    QSLOGW("double test : {}", d);
    QSLOGI("string test : {}", str);
    QSLOGF("c string test : {}", p);
    QSLOGI("c array test : {}", cArray);
    QSLOGI("nullptr test : {}", pInt);
    QSLOGI("normal ptr test : {}", sink);
    //QSLOGI("share ptr test : {}", fileSink.get());
    qslog::Logger::sync();
    return 0;
}