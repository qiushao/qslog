# qslog-design

## 1. 极低延迟日志系统的关键技术
参考 nanolog, fmtlog, bqlog 等低延迟日志系统，总结以下几点关键技术。
其中 延迟格式化，增加文件缓冲区，减少运行期间的重复计算， 这几点收益是最大的。

### 延迟格式化
我们调用日志的时候是这样的
```
uint64_t u64 = 10086;
QSLOGD("uint64 test : {}", u64);
``` 

生成的日志文件结果是这样的
```text
2025-04-09 16:09:58.156 32638 32638 D example [qslogExample.cpp:38 main] uint64 test : 10086
```

日志的模式为 `date pid tid level tag [file:line function] formatStr args`，可以看到有很多额外的信息需要进行格式化，
日志的格式化操作非常耗时，如果我们只存储二进制数据到日志文件，等需要查看日志再进行解压缩就可以节省很多耗时的格式化操作。
而且日志基本都是有规律的，我们可以提取不变的数据出来作为日志模板，只保存一份， 每一行日志只保存动态的参数，这样还可以同时进行数据的压缩。
具体的压缩方案后续讨论。

### 增加文件缓冲区
写文件是操作是一个比较耗时的操作，如果每一行日志都直接写入文件的话，性能肯定会受到非常大的影响。
因此我们先把数据写入到内存缓冲区中，当达到一定的大小，或者时间间隔之后，再写入到文件。
另外一种方案是使用 mmap,但实测收益并不大，与加缓冲区并无差别，甚至更差，而且使用 mmap 更加复杂，没必要。

### 减少运行期间的重复计算
这部分的逻辑可能得先看完后面日志压缩原理的内容之后，才容易理解。
写入日志文件的时候同时压缩文件，在压缩文件的时候需要提取不变量量 format str 只记录一次，同时分配一个 format id, 日志条目只记录 format id + 动态参数。
一般来说会使用一个 map<string, int> 的结构体来保存。每次从 map 中查询 format id, 查不到则添加到 map。这样的话，每条日志都要查询一遍 map 。BqLog 就是这么做的。
使用 perf 分析发现，map 查询占用很多时间。从 nanolog 中学习到了一种技巧。
```c++
void FormatIdManager::registerFormatId(uint16_t &formatId, std::shared_ptr<FormatEntry> formatEntry) {
    std::lock_guard<std::mutex> lock(mutex_);
    formatId = formatEntries_.size();
    formatEntries_.emplace_back(formatEntry);
    formatEntry->formatId_ = formatId;
}

    template<typename... Args>
    static void log(uint16_t &formatId, LogLevel level, const char *tag,
                    const char *file, uint16_t line, const char *function,
                    fmt::format_string<Args...> format, Args &&...args) {
        if (level < logLevel_) {
            return;
        }

        if (formatId == UINT16_MAX) {
            auto formatEntry = std::make_shared<FormatEntry>();
            ...
            FormatIdManager::registerFormatId(formatId, formatEntry);
        }
        ...
    }
#define QSLOG(level, tag, format, ...)                                                           \
    do {                                                                                         \
        static uint16_t qslogFormatId = UINT16_MAX;                                              \
        qslog::Logger::log(qslogFormatId, level, tag,                                            \
                           __FILE__, __LINE__, __FUNCTION__, FMT_STRING(format), ##__VA_ARGS__); \
    } while (0)
```

使用数组来替代 map, 在 QSLOG 宏中定义一个静态变量 `static uint16_t qslogFormatId = UINT16_MAX;`，用到这个宏来打印日志的点，都会有这么一个静态变量。
把这个静态变量的引用传给 log 函数，在里面判断 formatId 是否初始化过了，未初始化的话，就初始化一下。通过静态变量这个技巧就可以避免每条日志都查询一次 map。

### 编译期间计算
主要是应用 `constexpr` 关键字是在编译期常量计算的特性，把一些在编译期间已知的信息，能在编译期运算就在编译期运算。
比如日志参数的个数，参数类型， file, line, function 这些信息都是在编译期间就可以知道的了。
```c++
template<typename... Args>
    static void log(uint16_t &formatId, LogLevel level, const char *tag,
                    const char *file, uint16_t line, const char *function,
                    fmt::format_string<Args...> format, Args &&...args) {
        if (level < logLevel_) {
            return;
        }

        constexpr uint8_t argc = sizeof...(args);
```

上面的例子中 argc 在编译期间就能得到一个常量结果，不需要在运行时再计算一遍。

### rdtsc 替代 clock_gettime 
在日志里面需要显示打印日志的时间，一般来说都是通过 clock_gettime 或者 std::chrono::system_clock 来获取当前系统时间。
但 clock_gettime 或者 std::chrono::system_clock 是一个系统调用，现代 linux 系统上一般还会有个 vdso 加速系统调用的优化，大概十几 ns 左右。
在号称低延迟的系统上基本上都是使用 rdtsc 来获取系统时间的。 rdtsc 实际上是一个寄存器， rdtsc 只需要执行一条汇编指令即可获取当前的系统时间。 大概几 ns 左右。
虽然只是少了个十 ns 左右，但 ns 级别的日志系统，就是得一点点扣性能。

### 无锁日志队列
日志系统一般都采用生产者消费者模型。生产者即调用日志打印接口的线程，各生产者把日志放到日志队列，由一个日志处理线程（消费者）把日志进行处理输出。 是一个多生产者，单消费者的模型。
那我们需要的就是一个 *多生产者单消费者环形无锁队列*，简称 MPSC(Multiple Producers Single Consumer)，采用原子操作，CAS 等原理来避免加锁操作。BqLog 采用的是这种方案。
nanolog, fmtlog 采用另外一种更高效但资源消耗有点大的方案，算是以空间换时间了。
每个线程都分配一个日志队列，日志处理线程遍历所有日志队列进行处理。这样每个日志队列都退化为只有一个生产者一个消费者(SPSC queue)。这样的队列可以做到原子操作都不需要。

### 避免多余的内存拷贝
一般来说把日志放到日志队列里面，我们需要先定义一个日志对象，再 push 到队列，这样的话，就会有一次内存拷贝。 nanolog 采用的方案是原地构造，相关代码如下：
```c++
    char *writePos = NanoLogInternal::RuntimeLogger::reserveAlloc(allocSize);
    auto originalWritePos = writePos;

    UncompressedEntry *ue = new(writePos) UncompressedEntry();
    writePos += sizeof(UncompressedEntry);
```

预先分配好队列上的内存 writePos， 然后 `UncompressedEntry *ue = new(writePos) UncompressedEntry();` 直接在这个内存上构造日志对象。
这样可以避免内存分配和内存拷贝。

### 内存对齐与缓存失效


### 总结
应用了以上各种技巧优化后的日志系统延迟可以达到 10ns 左右。 

## 2. 压缩

### compress log file struct

| info entry | format entry | log entry |
| ---------- | ------------ | --------- |

日志由 info entry, format entry, log entry 几种数据项构成， entry 类型总共有 3 种，2 位可以表示，所以 entry 的第一个字节的高2位表示 entry type， 定义如下：

|     0     |      1       |     2      |
| --------- | ------------ | ---------- |
| log entry | format entry | info entry |

低 4 位由各 entry 内部定义子类型。


### info entry, type = 2

| entry type : 2 bits | info type : 2 bits | data length : 4 bits | data |
| ------------------- | ------------------ | -------------------- | ---- |

info entry 可以用来表示多种全局信息，目前有 pid, 当前系统完整时间戳（ts, 精度为毫秒）, info type 定义如下：

|    0     |    1    |
| -------- | ------- |
| pid type | ts type |

pid type 是当前的进程号，在进程打开日志文件的时候就会写入一条 pid 的 info entry, 后续的日志就不用再记录进程号了。pid 以 uint32 类型来保存。
ts type 表示系统时间戳，后续 log entry 的时间戳并不记录完整的时间戳，而是记录与 ts info 时间戳的差值。毫秒精度的时间戳差值，使用 2 字节就可以记录 1 分钟的时间差，所以 ts info 可以每隔 1 分钟插入一个， log entry 中的时间戳用 2 字节来记录差值。ts 以 uint64 类型来保存。

### format entry, type = 1

| entry type : 2 bits | log level : 6 bits | format id : uint16 | format str len : uint16 | format str |
| ------------------- | ------------------ | ------------------ | ----------------------- | ---------- |

format entry 不仅用来保存格式化字符串，还包含了文件名，行号，函数名， tag 信息。
`format str = tag file:line function format`
uint16 的最大值是 65535, 一般来说，一个应用的日志的点有个几千点已经算多的了，很少会超过 65535 这么多。用 uint16 相比 uint32, 每条日志可以少 2 个字节。


### log entry, type = 0
日志条目的格式如下：

| entry type : 2 bits | argc: 6 bits | ts diff : uint16 | format id : uint16 | tid : uint32 | args ... |
| ------------------- | ------------ | ---------------- | ------------------ | ------------ | -------- |

其中 args 按如下格式存储：

| arg0 type id : 4 bits | rest : 4 bits | arg0 | arg1 type | arg1 | ... |
| --------------------- | ------------- | ---- | --------- | ---- | --- |

参数个数，参数类型，应该存储在 format entry 中， 只存一次就行，不用每个 log entry 都去记录。
arg typeId 高4位为参数类型，定义如下：
```
enum ArgTypeId {
    BOOL = 0,
    CHAR = 1,
    UINT8 = 2,
    UINT64 = 3,
    FLOAT = 4,
    DOUBLE = 5,
    STR = 6
};
```

对于 bool 类型来说，低4位用于保存值，可以节省一个字节。
为什么没有 uint16, uint32?
因为这几种类型都采用变长编码，都当 64 处理即可。
为什么没有 int8, int16, int32, int64?
int8 为正数当 uint8 处理，负数先取反，再当 uint8 处理，正负号保存在 typeid 第 5 位
正数都当 uint64 处理，负数先取反，再当 uint64 处理，正负号保存在 typeid 第 5 位
遇到类型为 uint64 按变长编码来处理。typeid 第 5 位为 1 时，取反即可。

### 何时写入 info entry
pid info entry 在进程启动，打开日志文件的时候就写入，后续不再写入。
日志库有个 last_ts 变量， 初始值为 0 ， 写入日志时，计算 ts_diff = current_ts - last_ts， ts_diff > 65535 时，更新 last_ts, 写入一条 ts info entry 到日志文件。

### 何时写入 format entry
日志库有个 hashMap<string, uint16> formatMap，
写一条日志前，会从 formatMap 中查询 formatId, 当查询不到 formaId 时，则先写入 format entry ，再写入 log entry


## 3. 解压
设置变量 pid,  last_ts, hashMap<uint16, string> formatMap
从头开始解析每个 entry,
是 pid info entry 则更新 pid 变量，
是 ts info entry 则更新 last_ts 变量，
是 format entry，则更新 formatMap 数据，
是 log entry，则提取 formatId, 从 formatMap 中查询 format str, 提取 args, 调用 fmt 进行格式化，写入输出文件。

目前使用整数变长编码 LEB128 之前，不打印字条串变量的话，压缩比能达到 9 左右，打印字符串的话，只能达到 3 左右。
所以字符串变量的压缩也要考虑。

xlog 的日志压缩比在 8 左右。


## 4. 进程崩溃日志也不丢
