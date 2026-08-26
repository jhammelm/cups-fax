#include "logging.h"

FILE* logfp = NULL;
int logType = 0;

/***************************************************************************************
 ** log_event(type,message,...)
 ****************************************************************************************
 **
 ** UTILITY function to write cups-pdf events to custom log
 **
 ** Accepts:     short int indicating type of logging (CPERROR, CPSTATUS, CPDEBUG)
 **              pointer to char string containing log message
 **              varargs as required by the log message
 **
 ** Returns:     nothing
 **
 ** Configuration values used:
 **  LogType             log-mode (binary bitflags)
 */

void
log_event(short type, const char *message, ...)
{
   time_t secs;
   int error = errno;
   char ctype[8], *timestring;
   cp_string logbuffer;
   va_list ap;

   if (logfp != NULL && (type & logType))
   {
      (void) time(&secs);
      timestring = ctime(&secs);
      timestring[strlen(timestring) - 1] = '\0';

      if (type == CPERROR)
         snprintf(ctype, 8, "ERROR");
      else if (type == CPSTATUS)
         snprintf(ctype, 8, "STATUS");
      else
         snprintf(ctype, 8, "DEBUG");

      va_start(ap, message);
      vsnprintf(logbuffer, BUFSIZE, message, ap);
      va_end(ap);

      fprintf(logfp, "%s  [%s] %s\n", timestring, ctype, logbuffer);
      if ((logType & CPDEBUG) && (type == CPERROR) && error)
      {
         fprintf(logfp, "%s  [DEBUG] ERRNO: %d (%s)\n", timestring, error, strerror(error));
      }
      (void) fflush(logfp);
   }

   return;
}

int
enable_log(struct ConfigData* cfg)
{
   cp_string logFilename;
   struct stat fstatus;
   if (strlen(Conf_Log(cfg)))
   {
      close_log();

      if (stat(Conf_Log(cfg), &fstatus) || !S_ISDIR(fstatus.st_mode))
      {
         if (create_dir(Conf_Log(cfg), 1))
         {
            fprintf(stderr, "creating directory %s failed", Conf_Log(cfg));
            return 1;
         }
         if (chmod(Conf_Log(cfg), 0755))
         {
            fprintf(stderr, "chmod %d for directory %s failed", 0755, Conf_Log(cfg));
            return 1;
         }
      }

      snprintf(logFilename, BUFSIZE, "%s/%s%s%s", Conf_Log(cfg), "cups-fax-", getenv("PRINTER"), "_log");
      if ((logfp = fopen(logFilename, "a")) == NULL)
      {
         fprintf(stderr, "the following target file %s can not be appended", logFilename);
         return 1;
      }

      logType = Conf_LogType(cfg);
   }
   return 0;
}

void
close_log()
{
   if (logfp != NULL) {
      (void) fclose(logfp);
      logfp = NULL;
   }
}

/***************************************************************************************
 ** create_dir(dirname,nolog)
 ****************************************************************************************
 **
 ** UTILITY function to create a specified directory
 **
 ** Accepts:	char pointer to string containing target directory path
 **		int flag indicating whether (non-zero) or not (0) to log operations
 **
 ** Returns:	integer indicating success or failure of operation
 **		  0 is SUCCESS, 1 is FAILURE
 */
int
create_dir(char *dirname, int nolog)
{
   struct stat fstatus;
   char buffer[BUFSIZE], *delim;
   int i;

   while ((i = strlen(dirname)) > 1 && dirname[i - 1] == '/')
      dirname[i - 1] = '\0';
   if (stat(dirname, &fstatus) || !S_ISDIR(fstatus.st_mode))
   {
      strncpy(buffer, dirname, BUFSIZE);
      delim = strrchr(buffer, '/');
      if (delim != buffer)
         delim[0] = '\0';
      else
         delim[1] = '\0';
      if (create_dir(buffer, nolog) != 0)
         return 1;
      (void) stat(buffer, &fstatus);
      if (mkdir(dirname, fstatus.st_mode) != 0)
      {
         if (!nolog)
            log_event(CPERROR, "failed to create directory: %s", dirname);
         return 1;
      }
      else
         if (!nolog)
         log_event(CPSTATUS, "directory created: %s", dirname);
      if (chown(dirname, fstatus.st_uid, fstatus.st_gid) != 0)
         if (!nolog)
            log_event(CPDEBUG, "failed to set owner on directory: %s (non fatal)", dirname);
   }
   return 0;
}
