clear_local_var()
set(local_module libqslog)
set(local_source
    Logger.cpp
    BaseSink.cpp
    StdoutSink.cpp
    FileSink.cpp
    CompressFileSink.cpp
    LogEntry.cpp
    common.cpp
    FormatIdManager.cpp
    FileUtils.cpp
    OSUtils.cpp)

set(local_export_header include)
set(local_dependency fmt::fmt)
build_static_library()

add_subdirectory(decompress)
