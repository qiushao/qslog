clear_local_var()
set(local_module libqslog)
set(local_source
    api/Logger.cpp
    api/BaseSink.cpp
    api/StdoutSink.cpp
    api/CompressFileSink.cpp
    api/LogEntry.cpp
    api/common.cpp
    FileUtils.cpp
    OSUtils.cpp)
set(local_export_header api/include)
set(local_dependency fmt::fmt)
build_static_library()
