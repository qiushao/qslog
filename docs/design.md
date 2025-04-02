# qslog-design
## compress log file struct

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

| arg0 type : 1 byte | arg0 | arg1 type : 1 byte | arg1 | ... |
| ------------------ | ---- | ------------------ | ---- | --- |

如果要进一步压缩，可以考虑整数变长编码，比如 BqLog 使用了 VLQ 编码。每行日志可以再减少两三个字节。


### 何时写入 info entry
pid info entry 在进程启动，打开日志文件的时候就写入，后续不再写入。
日志库有个 last_ts 变量， 初始值为 0 ， 写入日志时，计算 ts_diff = current_ts - last_ts， ts_diff > 65535 时，更新 last_ts, 写入一条 ts info entry 到日志文件。

### 何时写入 format entry
日志库有个 hashMap<string, uint16> formatMap，
写一条日志前，会从 formatMap 中查询 formatId, 当查询不到 formaId 时，则先写入 format entry ，再写入 log entry


## 解压日志文件
设置变量 pid,  last_ts, hashMap<uint16, string> formatMap
从头开始解析每个 entry, 
是 pid info entry 则更新 pid 变量，
是 ts info entry 则更新 last_ts 变量，
是 format entry，则更新 formatMap 数据，
是 log entry，则提取 formatId, 从 formatMap 中查询 format str, 提取 args, 调用 fmt 进行格式化，写入输出文件。