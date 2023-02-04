/***************************************************************************
* Copyright (c) {% now 'utc', '%Y' %}, {{cookiecutter.full_name}}                                  
*                                                                          
* Distributed under the terms of the {{cookiecutter.open_source_license}}.                 
*                                                                          
* The full license is in the file LICENSE, distributed with this software. 
****************************************************************************/

#ifndef ALS_XEUS_CLING_CONFIG_HPP
#define ALS_XEUS_CLING_CONFIG_HPP

// Include paths.
// Copy below the include path of your cling installation.
#define ALS_CLING_INCLUDE_PATH "/opt/cling/include"
// Copy below the include path of the clang version the was installed with cling.
// Do not write the include path your normal clang version!
#define ALS_CLANG_INCLUDE_PATH "/opt/cling/lib/clang/9.0.1/include"

// Project version
#define ALS_XEUS_CLING_VERSION_MAJOR 0
#define ALS_XEUS_CLING_VERSION_MINOR 1
#define ALS_XEUS_CLING_VERSION_PATCH 0

// Composing the version string from major, minor and patch
#define ALS_XEUS_CLING_CONCATENATE(A, B) ALS_XEUS_CLING_CONCATENATE_IMPL(A, B)
#define ALS_XEUS_CLING_CONCATENATE_IMPL(A, B) A##B
#define ALS_XEUS_CLING_STRINGIFY(a) ALS_XEUS_CLING_STRINGIFY_IMPL(a)
#define ALS_XEUS_CLING_STRINGIFY_IMPL(a) #a

#define ALS_XEUS_CLING_VERSION ALS_XEUS_CLING_STRINGIFY(ALS_XEUS_CLING_CONCATENATE(ALS_XEUS_CLING_VERSION_MAJOR,   \
                 ALS_XEUS_CLING_CONCATENATE(.,ALS_XEUS_CLING_CONCATENATE(ALS_XEUS_CLING_VERSION_MINOR,   \
                                  ALS_XEUS_CLING_CONCATENATE(.,ALS_XEUS_CLING_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef ALS_XEUS_CLING_EXPORTS
        #define ALS_XEUS_CLING_API __declspec(dllexport)
    #else
        #define ALS_XEUS_CLING_API __declspec(dllimport)
    #endif
#else
    #define ALS_XEUS_CLING_API
#endif

#endif