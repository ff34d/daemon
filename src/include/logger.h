#ifndef LOGGER_H
#define LOGGER_H

typedef enum { LOG_INFO, LOG_WARN, LOG_ERROR, _LOG_COUNTER } log_level_t;

void log_message(log_level_t level, const char *name, const char *str);

#endif
