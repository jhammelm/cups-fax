#ifndef __logging_h__
#define __logging_h__

#ifndef CPV3
   #include "config.h"
#else
   #include <time.h>
   #include <errno.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <fcntl.h>
   #include <string.h>
   #include <ctype.h>
   #include <unistd.h>
   #include <stddef.h>
   #include <pwd.h>
   #include <grp.h>
   #include <dirent.h>
   #include <stdarg.h>
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <sys/wait.h>

   #include <cups/cups.h>
   #include <cups/ppd.h>
   #include <cups/backend.h>

   /* User-customizable settings - if unsure leave the default values 
   /  they are reasonable for most systems.			     */

   /* location of the configuration file */
   #define CP_CONFIG_PATH "/etc/cups"

   /* --- DO NOT EDIT BELOW THIS LINE --- */

   /* The following settings are for internal purposes only - all relevant 
   /  options listed below can be set via cups-fax.conf at runtime     */

   #define CPVERSION "v3.1.0"

   #define BUFSIZE 4096
   #define TBUFSIZE "4096"

   typedef char cp_string[BUFSIZE];

   #define ENDPOINT  "http://localhost:8080/ws-cups-fax/WS"

   #define CPERROR         1
   #define CPSTATUS        2
   #define CPDEBUG         4

#endif

int confLogType();

char* confLog();

void log_event(short type, const char *message, ...);

int enable_log();

void close_log();

int create_dir(char *dirname, int nolog);

#endif