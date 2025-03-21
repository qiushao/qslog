macro(clear_local_var)
    set(local_module "")
    set(local_source "")
    set(local_include "")
    set(local_cflags "")
    set(local_dependency "")
    set(local_export_header "")
    set(local_link_dir "")
endmacro()

macro(build_library library_type)
    add_library(${local_module} ${library_type} ${local_source})
    target_compile_definitions(${local_module} PUBLIC ${local_cflags})
    target_include_directories(${local_module} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${local_include})
    target_include_directories(${local_module} PUBLIC ${local_export_header})
    target_link_libraries(${local_module} PUBLIC ${local_dependency})
endmacro()

macro(build_prebuilt_library library_type)
    add_library(${local_module} ${library_type} IMPORTED GLOBAL)
    set_target_properties(${local_module} PROPERTIES IMPORTED_LOCATION ${local_source} INTERFACE_INCLUDE_DIRECTORIES
                                                                                       ${local_export_header})
endmacro()

macro(build_static_library)
    build_library(STATIC)
endmacro()

macro(build_shared_library)
    build_library(SHARED)
endmacro()

macro(build_prebuilt_static_library)
    build_prebuilt_library(STATIC)
endmacro()

macro(build_prebuilt_shared_library)
    build_prebuilt_library(SHARED)
endmacro()

macro(build_interface_library)
    add_library(${local_module} INTERFACE)
    target_include_directories(${local_module} INTERFACE ${local_export_header})
    target_link_directories(${local_module} INTERFACE ${local_link_dir})
    target_link_libraries(${local_module} INTERFACE ${local_dependency})
endmacro()

macro(build_executable)
    add_executable(${local_module} ${local_source})
    target_compile_definitions(${local_module} PUBLIC ${local_cflags})
    target_include_directories(${local_module} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${local_include})
    target_link_libraries(${local_module} PUBLIC ${local_dependency})
endmacro()
