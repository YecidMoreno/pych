#pragma once

/*
    journalctl -t hh  -f --output=cat
    cmake .. -DHH_LOG_LEVEL=LOG_INFO 
    cmake .. -DHH_LOG_LEVEL=LOG_DEBUG 
*/

#include <syslog.h>
#include <string.h>

// Extrae solo el nombre del archivo, no la ruta
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

// Nivel de log global (defínelo vía -DHH_LOG_LEVEL=LOG_INFO en tu build)
#ifndef HH_LOG_LEVEL
#define HH_LOG_LEVEL LOG_INFO
#endif

// Interno: macro de control
#define HH_LOG_ENABLED(level) ((level) <= HH_LOG_LEVEL)

// LOG_ERR
#if HH_LOG_LEVEL >= LOG_ERR
#define hh_loge(fmt, ...) syslog(LOG_ERR, "[%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#else
#define hh_loge(fmt, ...) do {} while(0)
#endif

// LOG_WARNING
#if HH_LOG_LEVEL >= LOG_WARNING
#define hh_logw(fmt, ...) syslog(LOG_WARNING, "[%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#else
#define hh_logw(fmt, ...) do {} while(0)
#endif

// LOG_NOTICE
#if HH_LOG_LEVEL >= LOG_NOTICE
#define hh_logn(fmt, ...) syslog(LOG_NOTICE, fmt, ##__VA_ARGS__)
#else
#define hh_logn(fmt, ...) do {} while(0)
#endif

// LOG_INFO
#if HH_LOG_LEVEL >= LOG_INFO
#define hh_logi(fmt, ...) syslog(LOG_INFO,fmt, ##__VA_ARGS__)
#else
#define hh_logi(fmt, ...) do {} while(0)
#endif

// LOG_DEBUG
#if HH_LOG_LEVEL >= LOG_DEBUG
#define hh_logv(fmt, ...) syslog(LOG_DEBUG, "[%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#else
#define hh_logv(fmt, ...) do {} while(0)
#endif

// Inicializa syslog con etiqueta personalizada
#define hh_log_init(tag) openlog(tag, LOG_PID | LOG_CONS, LOG_USER)

// Cierra syslog si es necesario
#define hh_log_close() closelog()
