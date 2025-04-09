# qslog-design

## 1. 极低延迟日志系统的关键技术
参考 nanolog, fmtlog, bqlog 等低延迟日志系统，总结以下几点关键技术。
其中 延迟格式化，增加文件缓冲区，减少运行期间的重复计算， 这几点收益是最大的。

### 延迟格式化
我们调用日志的时候是这样的
```
uint64_t u64 = 18446744073709551615u;
QSLOGD("uint64 test : {}", u64);
``` 

生成的日志文件结果是这样的
```text
2025-04-09 16:09:58.156 32638 32638 D example [qslogExample.cpp:38 main] uint64 test : 18446744073709551615
```

日志的模式为 `date pid tid level tag [file:line function] formatStr args`，可以看到有很多额外的信息需要进行格式化，
日志的格式化操作非常耗时，如果我们只存储二进制数据到日志文件，等需要查看日志再进行解压缩就可以节省很多耗时的格式化操作。
而且日志基本都是有规律的，我们可以提取不变的数据出来作为日志模板，只保存一份， 每一行日志只保存动态的参数，这样还可以同时进行数据的压缩。
具体的压缩方案后续讨论。

### 增加文件缓冲区
写文件是操作是一个比较耗时的操作，如果每一行日志都直接写入文件的话，性能肯定会受到非常大的影响。
因此我们先把数据写入到内存缓冲区中，当达到一定的大小，或者时间间隔之后，再写入到文件。

### 减少运行期间的重复计算

### 编译期间计算

### rdtsc 替代 system_clock 

### 无锁日志队列

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
