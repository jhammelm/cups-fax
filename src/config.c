#include "logging.h"

struct ConfigData *
create_config() {
   struct ConfigData* cfg = malloc(sizeof(configDefaults));
   if (!cfg) {
      return NULL;
   }
   memcpy(cfg, configDefaults, sizeof(configDefaults));
   return cfg;
}

void
free_config(struct ConfigData* cfg) {
   if (!cfg) {
      return;
   }
   free(cfg);
}

/***************************************************************************************
 ** _assign_value(security,key,value)
 ****************************************************************************************
 **
 ** Store/update process-global key/value pairs
 **
 ** Accepts:	int representing level of security to apply
 **		char pointer to string containing key of key/value pair
 **		char pointer to string containing value of key/value pair
 **
 ** Returns:	integer representing success or failure of operation
 **		1 is SUCCESS
 **		0 is FAILURE
 **
 ** Configuration values set:
 **	AnonDirName		ABSOLUTE path for anonymously created PDF files
 **	AnonUser                uid for anonymous PDF creation
 **	GhostScript		location of GhostScript binary (gs)
 **	GSCall			command line for calling GhostScript
 **	Grp      		group cups-pdf is supposed to run as
 **	GSTmp                  	location of temporary files during GhostScript operation 
 **	Log                     CUPS-FAX log directory
 **	PDFVer			PDF version to be created
 **	PostProcessing          postprocessing script that will be called after the creation of the PDF
 **	HomeConf                Config file in	home directory of user
 **	Out                     CUPS-FAX output directory
 **	Spool                  	CUPS-FAX spool directory
 **	UserPrefix		some installations require a domain prefix added to the user name
 **	RemovePrefix           	domain prefix to be removed for ${USER} expansion
 **	Cut      		remove file name extensions before appending .pdf to output
 **	Truncate                max # characters to truncate long filenames to
 **	DirPrefix		if UserPrefix, then use the prefix in the output directory's name
 **	Label                   label all jobs with a unique job-id
 **	LogType			log-mode (bitflags: errors, status, debug)
 **	LowerCase		check user names against their lower case variants
 **	TitlePref		where to look first for a title when creating the output filename
 **	DecodeHexStrings	try to decode hex strings in the title to allow internationalized titles
 **	FixNewlines		try to fix various unusal line delimiters (e.g. form feeds)
 **	AnonUMask		umask for anonymous output
 **	UserUMask		umask for user output of known users
 */
int
_assign_value(struct ConfigData* cfg, int security, char *key, char *value)
{
   int tmp;
   int option;

   for (option = 0; option < NUM_CONFIG_ITEMS; option++)
   {
      if (strcasecmp(key, configDefaults[option].key_name) == 0)
      {
         break;
      }
   }

   if (option == NUM_CONFIG_ITEMS)
   {
      return 0;
   }

   if (!(security & configDefaults[option].security) && !(Conf_AllowUnsafeOptions(cfg)))
   {
      log_event(CPERROR, "Unsafe option not allowed: %s", key);
      return 0;
   }

   switch (option)
   {
      case AnonDirName:
         strncpy(Conf_AnonDirName(cfg), value, BUFSIZE);
         break;

      case AnonUser:
         strncpy(Conf_AnonUser(cfg), value, BUFSIZE);
         break;

      case GhostScript:
         strncpy(Conf_GhostScript(cfg), value, BUFSIZE);
         break;

      case GSCall:
         strncpy(Conf_GSCall(cfg), value, BUFSIZE);
         break;

      case Grp:
         strncpy(Conf_Grp(cfg), value, BUFSIZE);
         break;

      case GSTmp:
         snprintf(Conf_GSTmp(cfg), BUFSIZE, "%s%s", "TMPDIR=", value);
         break;

      case Log:
         strncpy(Conf_Log(cfg), value, BUFSIZE);
         break;

      case PDFVer:
         strncpy(Conf_PDFVer(cfg), value, BUFSIZE);
         break;

      case PostProcessing:
         strncpy(Conf_PostProcessing(cfg), value, BUFSIZE);
         break;

      case HomeConf:
         strncpy(Conf_HomeConf(cfg), value, BUFSIZE);
         break;

      case Out:
         strncpy(Conf_Out(cfg), value, BUFSIZE);
         break;

      case Spool:
         strncpy(Conf_Spool(cfg), value, BUFSIZE);
         break;

      case UserPrefix:
         strncpy(Conf_UserPrefix(cfg), value, BUFSIZE);
         break;

      case RemovePrefix:
         strncpy(Conf_RemovePrefix(cfg), value, BUFSIZE);
         break;

      case Cut:
         tmp = atoi(value);
         Conf_Cut(cfg) = (tmp >= -1) ? tmp : -1;
         break;

      case Truncate:
         tmp = atoi(value);
         Conf_Truncate(cfg) = (tmp >= 8) ? tmp : 8;
         break;

      case DirPrefix:
         tmp = atoi(value);
         Conf_DirPrefix(cfg) = (tmp) ? 1 : 0;
         break;

      case Label:
         tmp = atoi(value);
         Conf_Label(cfg) = (tmp > 2) ? 2 : ((tmp < 0) ? 0 : tmp);
         break;

      case LogType:
         tmp = atoi(value);
         Conf_LogType(cfg) = (tmp > 7) ? 7 : ((tmp < 0) ? 0 : tmp);
         break;

      case LowerCase:
         tmp = atoi(value);
         Conf_LowerCase(cfg) = (tmp) ? 1 : 0;
         break;

      case TitlePref:
         tmp = atoi(value);
         Conf_TitlePref(cfg) = (tmp) ? 1 : 0;
         break;

      case DecodeHexStrings:
         tmp = atoi(value);
         Conf_DecodeHexStrings(cfg) = (tmp) ? 1 : 0;
         break;

      case FixNewlines:
         tmp = atoi(value);
         Conf_FixNewlines(cfg) = (tmp) ? 1 : 0;
         break;

      case AnonUMask:
         tmp = (int) strtol(value, NULL, 8);
         Conf_AnonUMask(cfg) = (mode_t) tmp;
         break;

      case UserUMask:
         tmp = (int) strtol(value, NULL, 8);
         Conf_UserUMask(cfg) = (mode_t) tmp;
         break;

      case Resolution:
         strncpy(Conf_Resolution(cfg), value, BUFSIZE);
         break;

      case Endpoint:
         strncpy(Conf_Endpoint(cfg), value, BUFSIZE);
         break;

      case AuthUser:
         strncpy(Conf_AuthUser(cfg), value, BUFSIZE);
         break;

      case AuthPwd:
         strncpy(Conf_AuthPwd(cfg), value, BUFSIZE);
         break;

      case FaxNumber:
         strncpy(Conf_FaxNumber(cfg), value, BUFSIZE);
         break;

      case FaxHeader:
         strncpy(Conf_FaxHeader(cfg), value, BUFSIZE);
         break;

      case SendingFaxID:
         strncpy(Conf_SendingFaxID(cfg), value, BUFSIZE);
         break;

      case EMailAddress:
         strncpy(Conf_EMailAddress(cfg), value, BUFSIZE);
         break;

      case FaxResolution:
         strncpy(Conf_FaxResolution(cfg), value, BUFSIZE);
         break;

      case FaxRendering:
         strncpy(Conf_FaxRendering(cfg), value, BUFSIZE);
         break;

      case FaxDelay:
         tmp = (int) strtol(value, NULL, 8);
         Conf_FaxDelay(cfg) = (int) tmp;
         break;

      case FaxMaxRetry:
         tmp = (int) strtol(value, NULL, 8);
         Conf_FaxMaxRetry(cfg) = (int) tmp;
         break;

      case Preview:
         strncpy(Conf_Preview(cfg), value, BUFSIZE);
         break;

      default:
         log_event(CPERROR, "Program error: option not treated: %s = %s\n", key, value);
         return 0;
   }
   return 1;
}

/***************************************************************************************
 ** read_config_file(filename)
 ****************************************************************************************
 **
 ** Load and merge named configuration file into process configuration
 **
 ** Accepts:	valid (not NULL) pointer to string representing path to config file
 ** Performs:	updates global configuration values from values found in named config file	
 ** Returns:	int, is 0, if configuration could be read
 */
int
read_config_file(struct ConfigData* cfg, char *filename)
{
   FILE *fp = NULL;
   struct stat fstatus;
   cp_string buffer, key, value;

   if ((strlen(filename) > 1) && (!lstat(filename, &fstatus)) &&
       (S_ISREG(fstatus.st_mode) || S_ISLNK(fstatus.st_mode)))
   {
      fp = fopen(filename, "r");
   }
   if (fp == NULL)
   {
      log_event(CPERROR, "Cannot open config: %s", filename);
      return 1;
   }

   int i = 0;
   while (fgets(buffer, BUFSIZE, fp) != NULL)
   {
      key[0] = '\0';
      value[0] = '\0';
      if (sscanf(buffer, "%s %[^\n]", key, value))
      {
         if (!strlen(key) || !strncmp(key, "#", 1))
            continue;
         _assign_value(cfg, SEC_CONF, key, value);
         ++i;
      }
   }

   if (i)
   {
      log_event(CPDEBUG, "Read config: %s", filename);
   }

   (void) fclose(fp);
   return 0;
}

/***************************************************************************************
 ** read_config_ppd()
 ****************************************************************************************
 **
 ** Load and merge configuration from PPD file into process configuration
 **
 ** Accepts:	nothing
 ** Performs:	updates global configuration values from PPD file named by envvar "PPD"
 ** Returns:	nothing
 **
 ** NB: uses functions from libcups to access ppd file
 ** NB: CUPS deprecated the PPD API, starting in CUPS 1.6, and recommends
 **	using the (as yet to be documented) "Job Ticket API" instead.
 */
void
read_config_ppd(struct ConfigData* cfg)
{
   ppd_option_t *option;
   ppd_file_t *ppd_file;
   char * ppd_name;

   ppd_name = getenv("PPD");
   if (ppd_name == NULL)
   {
      log_event(CPERROR, "Could not retrieve PPD name");
      return;
   }
   ppd_file = ppdOpenFile(ppd_name);
   if (ppd_file == NULL)
   {
      log_event(CPERROR, "Could not open PPD file: %s", ppd_name);
      return;
   }

   log_event(CPDEBUG, "Opened PPD file: %s", ppd_name);

   ppdMarkDefaults(ppd_file);

   option = ppdFirstOption(ppd_file);
   while (option != NULL)
   {
      _assign_value(cfg, SEC_PPD, option->keyword, option->defchoice);
      option = ppdNextOption(ppd_file);
   }
   ppdClose(ppd_file);
}

/***************************************************************************************
 ** read_config_options(lpoptions)
 ****************************************************************************************
 **
 ** Load and merge configuration from print job options into process configuration
 **
 ** Accepts:	char pointer to job options string
 ** Performs:	updates global configuration values from given job options string
 ** Returns:	nothing
 **
 ** NB: uses function from libcups to access options string
 */
void
read_config_options(struct ConfigData* cfg, const char *lpoptions)
{
   int i;
   cups_option_t *options;
   cups_option_t *option;
   int num_options = cupsParseOptions(lpoptions, 0, &options);

   for (i = 0, option = options; i < num_options; i++, option++)
   {

      /* replace all _ by " " in value */
      int j;
      for (j = 0; option->value[j] != '\0'; j++)
      {
         if (option->value[j] == '_')
         {
            option->value[j] = ' ';
         }
      }
      _assign_value(cfg, SEC_LPOPT, option->name, option->value);
   }
}

/***************************************************************************************
 ** dump_configuration(src)
 ****************************************************************************************
 ** DEBUGGING function to write selected configuration values to custom log
 **
 ** Accepts:	nothing
 ** Returns:	nothing
 **
 ** Configuration values used:
 **	AnonDirName		ABSOLUTE path for anonymously created PDF files
 **	AnonUser		uid for anonymous PDF creation
 **	GhostScript		location of GhostScript binary (gs)
 **	GSCall			command line for calling GhostScript
 **	Grp			group cups-pdf is supposed to run as
 **	GSTmp			location of temporary files during GhostScript operation 
 **	Log			CUPS-FAX log directory
 **	PDFVer			PDF version to be created
 **	PostProcessing		postprocessing script that will be called after the creation of the PDF
 **	Out			CUPS-FAX output directory
 **	Spool			CUPS-FAX spool directory
 **	UserPrefix		some installations require a domain prefix added to the user name
 **	RemovePrefix		domain prefix to be removed for ${USER} expansion
 **	Cut			remove file name extensions before appending .pdf to output
 **	Truncate		max # characters to truncate long filenames to
 **	DirPrefix		if UserPrefix, then use the prefix in the output directory's name
 **	Label			label all jobs with a unique job-id
 **	LogType			log-mode (bitflags: errors, status, debug)
 **	LowerCase		check user names against their lower case variants
 **	TitlePref		where to look first for a title when creating the output filename
 **	DecodeHexStrings	try to decode hex strings in the title to allow internationalized titles
 **	FixNewlines		try to fix various unusal line delimiters (e.g. form feeds)
 **	AnonUMask		umask for anonymous output
 **	UserUMask		umask for user output of known users
 **     Preview                 switch to send FAX template via email
 */
void
dump_configuration(struct ConfigData* cfg, char *user)
{
   if (Conf_LogType(cfg) & CPDEBUG)
   {
      log_event(CPDEBUG, "*** Final Configuration for user %s ***", user);
      log_event(CPDEBUG, "AnonDirName        = \"%s\"", Conf_AnonDirName(cfg));
      log_event(CPDEBUG, "AnonUser           = \"%s\"", Conf_AnonUser(cfg));
      log_event(CPDEBUG, "GhostScript        = \"%s\"", Conf_GhostScript(cfg));
      log_event(CPDEBUG, "GSCall             = \"%s\"", Conf_GSCall(cfg));
      log_event(CPDEBUG, "Grp                = \"%s\"", Conf_Grp(cfg));
      log_event(CPDEBUG, "GSTmp              = \"%s\"", Conf_GSTmp(cfg));
      log_event(CPDEBUG, "Log                = \"%s\"", Conf_Log(cfg));
      log_event(CPDEBUG, "PDFVer             = \"%s\"", Conf_PDFVer(cfg));
      log_event(CPDEBUG, "PostProcessing     = \"%s\"", Conf_PostProcessing(cfg));
      log_event(CPDEBUG, "HomeConf           = \"%s\"", Conf_HomeConf(cfg));
      log_event(CPDEBUG, "Out                = \"%s\"", Conf_Out(cfg));
      log_event(CPDEBUG, "Spool              = \"%s\"", Conf_Spool(cfg));
      log_event(CPDEBUG, "UserPrefix         = \"%s\"", Conf_UserPrefix(cfg));
      log_event(CPDEBUG, "RemovePrefix       = \"%s\"", Conf_RemovePrefix(cfg));
      log_event(CPDEBUG, "OutExtension       = \"%s\"", Conf_OutExtension(cfg));
      log_event(CPDEBUG, "Cut                = %d", Conf_Cut(cfg));
      log_event(CPDEBUG, "Truncate           = %d", Conf_Truncate(cfg));
      log_event(CPDEBUG, "DirPrefix          = %d", Conf_DirPrefix(cfg));
      log_event(CPDEBUG, "Label              = %d", Conf_Label(cfg));
      log_event(CPDEBUG, "LogType            = %d", Conf_LogType(cfg));
      log_event(CPDEBUG, "LowerCase          = %d", Conf_LowerCase(cfg));
      log_event(CPDEBUG, "TitlePref          = %d", Conf_TitlePref(cfg));
      log_event(CPDEBUG, "DecodeHexStrings   = %d", Conf_DecodeHexStrings(cfg));
      log_event(CPDEBUG, "FixNewlines        = %d", Conf_FixNewlines(cfg));
      log_event(CPDEBUG, "AllowUnsafeOptions = %d", Conf_AllowUnsafeOptions(cfg));
      log_event(CPDEBUG, "AnonUMask          = %04o", Conf_AnonUMask(cfg));
      log_event(CPDEBUG, "UserUMask          = %04o", Conf_UserUMask(cfg));
      log_event(CPDEBUG, "Resolution         = \"%s\"", Conf_Resolution(cfg));
      log_event(CPDEBUG, "Endpoint           = \"%s\"", Conf_Endpoint(cfg));
      log_event(CPDEBUG, "AuthUser           = \"%s\"", Conf_AuthUser(cfg));
      log_event(CPDEBUG, "AuthPwd            = \"%s\"", Conf_AuthPwd(cfg));
      log_event(CPDEBUG, "FaxNumber          = \"%s\"", Conf_FaxNumber(cfg));
      log_event(CPDEBUG, "FaxHeader          = \"%s\"", Conf_FaxHeader(cfg));
      log_event(CPDEBUG, "SendingFaxID       = \"%s\"", Conf_SendingFaxID(cfg));
      log_event(CPDEBUG, "EMailAddress       = \"%s\"", Conf_EMailAddress(cfg));
      log_event(CPDEBUG, "FaxResolution      = \"%s\"", Conf_FaxResolution(cfg));
      log_event(CPDEBUG, "FaxRendering       = \"%s\"", Conf_FaxRendering(cfg));
      log_event(CPDEBUG, "FaxDelay           = %d", Conf_FaxDelay(cfg));
      log_event(CPDEBUG, "FaxMaxRetry        = %d", Conf_FaxMaxRetry(cfg));
      log_event(CPDEBUG, "Preview            = \"%s\"", Conf_Preview(cfg));
      log_event(CPDEBUG, "*** End of Configuration ***");
   }
   return;
}

/***************************************************************************************
 ** preparedirname(src,passwd,uname)
 ****************************************************************************************
 **
 ** generate user-specific output directory from prototype
 **
 ** Accepts:	pointer to struct passwd for selected target user
 **		pointer to char string naming target user
 **
 ** Returns:	pointer to char string containing (computed) path of output directory
 **
 ** Configuration:
 **    RemovePrefix	common username prefix to be removed
 **    Out		CUPS-FAX output directory
 */
char *
preparedirname(struct ConfigData* cfg, char *src, struct passwd *passwd, char *uname)
{
   int size;
   char bufin[BUFSIZE], bufout[BUFSIZE], *needle, *cptr;

   needle = strstr(uname, Conf_RemovePrefix(cfg));
   if ((int) strlen(uname)>(size = strlen(Conf_RemovePrefix(cfg))))
      uname = uname + size;

   strncpy(bufin, src, BUFSIZE);
   do
   {
      needle = strstr(bufin, "${HOME}");
      if (needle == NULL)
         break;
      needle[0] = '\0';
      cptr = needle + 7;
      snprintf(bufout, BUFSIZE, "%s%s%s", bufin, passwd->pw_dir, cptr);
      strncpy(bufin, bufout, BUFSIZE);
   }
   while (needle != NULL);
   do
   {
      needle = strstr(bufin, "${USER}");
      if (needle == NULL)
         break;
      needle[0] = '\0';
      cptr = needle + 7;
      if (!Conf_DirPrefix(cfg))
         snprintf(bufout, BUFSIZE, "%s%s%s", bufin, uname, cptr);
      else
         snprintf(bufout, BUFSIZE, "%s%s%s", bufin, passwd->pw_name, cptr);
      strncpy(bufin, bufout, BUFSIZE);
   }
   while (needle != NULL);
   size = strlen(bufin) + 1;
   cptr = calloc(size, sizeof (char));
   if (cptr == NULL)
      return NULL;
   snprintf(cptr, size, "%s", bufin);
   return cptr;
}

/***************************************************************************************
 ** init()
 ****************************************************************************************
 **
 ** Load printer-global and printer-instance configuration values,
 ** establish execution group and spool directory
 ** establish log directory if required
 **
 ** Accepts:	a pointer to a list of pointer-to-char containing the commandline arguments
 ** Returns:	int	0 returned on success
 **			1 returned on all error conditions
 **
 ** Directly uses configuration values:
 **   Grp	group cups-pdf is supposed to run as
 **   Log	CUPS-FAX log directory
 **   Spool	CUPS-FAX spool directory
 **
 ** NB: uses function from libcups to retrieve backend device URI
 */
struct ConfigData*
init(char *argv[])
{
   struct ConfigData* cfg = create_config();

   cp_string filename;
   const char *uri = cupsBackendDeviceURI(argv);
#ifdef CPLOG
   if (enable_log(cfg))
   {
      return NULL;
   }
#endif
   if ((uri != NULL) && (strncmp(uri, "cups-fax:/", 11) == 0) && strlen(uri) > 11)
   {
      uri = uri + 11;
      sprintf(filename, "%s/cups-fax-%s.conf", CP_CONFIG_PATH, uri);
   }
   else
   {
      sprintf(filename, "%s/cups-fax.conf", CP_CONFIG_PATH);
   }

   read_config_file(cfg, filename);

   read_config_ppd(cfg);

   return cfg;
}

int
init2(struct ConfigData* cfg, char *argv[], struct passwd *passwd)
{
   struct stat fstatus;
   struct group *group;
   cp_string homedir;
   int grpstat;

   char *p = strncpy(homedir, preparedirname(cfg, Conf_HomeConf(cfg), passwd, argv[2]), BUFSIZE);
   size_t l = p - homedir;
   while ((l = strlen(homedir)) && ((homedir[l - 1] == '\n') ||
                                    (homedir[l - 1] == '\r')))
   {
      homedir[l - 1] = '\0';
   }
   if (l)
   {
      read_config_file(cfg, homedir);
   }

   read_config_options(cfg, argv[5]);

   (void) umask(0077);

   group = getgrnam(Conf_Grp(cfg));
   grpstat = setgid(group->gr_gid);
#ifdef CPLOG
   if (enable_log(cfg))
   {
      return 1;
   }
#endif
   dump_configuration(cfg, argv[2]);

   if (!group)
   {
      log_event(CPERROR, "Grp not found: %s", Conf_Grp(cfg));
      return 1;
   }
   else if (grpstat)
   {
      log_event(CPERROR, "failed to set new gid: %s", Conf_Grp(cfg));
      return 1;
   }
   else
      log_event(CPDEBUG, "set new gid: %s", Conf_Grp(cfg));

   (void) umask(0022);

   if (stat(Conf_Spool(cfg), &fstatus) || !S_ISDIR(fstatus.st_mode))
   {
      if (create_dir(Conf_Spool(cfg), 0))
      {
         log_event(CPERROR, "failed to create spool directory: %s", Conf_Spool(cfg));
         return 1;
      }
      if (chmod(Conf_Spool(cfg), 0751))
      {
         log_event(CPERROR, "failed to set mode on spool directory: %s", Conf_Spool(cfg));
         return 1;
      }
      if (chown(Conf_Spool(cfg), -1, group->gr_gid))
         log_event(CPERROR, "failed to set group id %s on spool directory: %s (non fatal)", Conf_Grp(cfg), Conf_Spool(cfg));
      log_event(CPSTATUS, "spool directory created: %s", Conf_Spool(cfg));
   }

   (void) umask(0077);
   return 0;
}
