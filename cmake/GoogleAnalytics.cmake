
set(ANALYTICS OFF CACHE BOOL "Using Google Analytics")
set(ANALYTICS_ID "" CACHE STRING "Google Analytics ID")
set(ANALYTICS_KEY "" CACHE STRING "Google Analytics key")

if (ANALYTICS)
    if (NOT ANALYTICS_ID OR NOT ANALYTICS_KEY)
        message(FATAL_ERROR "ANALYTICS set to ON, You need set ANALYTICS_ID and ANALYTICS_KEY too")
    endif ()
    list(APPEND APP_PLATFORM_OPTION
        -DANALYTICS=${ANALYTICS}
        -DANALYTICS_ID=${ANALYTICS_ID}
        -DANALYTICS_KEY=${ANALYTICS_KEY}
    )
endif ()