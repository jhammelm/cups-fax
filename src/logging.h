#ifndef __logging_h__
#define __logging_h__

#include "config.h"

void log_event(short type, const char *message, ...);

int enable_log(struct ConfigData* cfg);

void close_log();

int create_dir(char *dirname, int nolog);

#endif
