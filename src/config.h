#ifndef __config_h__
#define __config_h__

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

#define SEC_CONF  1
#define SEC_PPD   2
#define SEC_LPOPT 4

/* order in the enum and the struct-array has to be identical! */

typedef enum
{
   AnonDirName, AnonUser, GhostScript, GSCall, Grp, GSTmp, Log, PDFVer, PostProcessing, HomeConf, Out, Spool, UserPrefix, RemovePrefix,    
   OutExtension, Cut, Truncate, DirPrefix, Label, LogType, LowerCase, TitlePref, DecodeHexStrings, FixNewlines, AllowUnsafeOptions, AnonUMask,     UserUMask, Resolution, Endpoint, AuthUser, AuthPwd, FaxNumber, FaxHeader, SendingFaxID, EMailAddress, FaxResolution, FaxRendering, FaxDelay,    FaxMaxRetry, Preview, NUM_CONFIG_ITEMS
} ConfigIndex;

struct ConfigData
{
   char *key_name;
   int security;

   union
   {
      cp_string sval;
      int ival;
      mode_t modval;
   } value;
};

static const struct ConfigData configDefaults[] = {
   { "AnonDirName", SEC_CONF | SEC_PPD, { "/var/spool/cups-fax/ANONYMOUS"} },
   { "AnonUser", SEC_CONF | SEC_PPD, { "nobody"} },
   { "GhostScript", SEC_CONF | SEC_PPD, { "/usr/bin/gs" } },
   { "GSCall", SEC_CONF | SEC_PPD, { "%s -q -dCompatibilityLevel=%s -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pdfwrite -sOutputFile=\"%s\" -dAutoRotatePages=/PageByPage -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode -dPDFSETTINGS=/prepress %s %s"} },
   { "Grp", SEC_CONF | SEC_PPD, { "lp" } },
   { "GSTmp", SEC_CONF | SEC_PPD, { "TMPDIR=/var/tmp" } },
   { "Log", SEC_CONF | SEC_PPD, { "/var/log/cups" } },
   { "PDFVer", SEC_CONF | SEC_PPD | SEC_LPOPT, { "1.4" } },
   { "PostProcessing", SEC_CONF | SEC_PPD | SEC_LPOPT, { "" } },
   { "HomeConf", SEC_CONF | SEC_PPD, { "/home/${USER}/.cups-fax.conf" } },
   { "Out", SEC_CONF | SEC_PPD, { "/var/spool/cups-fax/${USER}" } },
   { "Spool", SEC_CONF | SEC_PPD, { "/var/spool/cups-fax/SPOOL" } },
   { "UserPrefix", SEC_CONF | SEC_PPD, { "" } },
   { "RemovePrefix", SEC_CONF | SEC_PPD, { "" } },
   { "OutExtension", SEC_CONF | SEC_PPD | SEC_LPOPT, { "pdf" } },
   { "Cut", SEC_CONF | SEC_PPD | SEC_LPOPT, {{ 3 }} },
   { "Truncate", SEC_CONF | SEC_PPD | SEC_LPOPT, {{ 64 }} },
   { "DirPrefix", SEC_CONF | SEC_PPD, {{ 0}} },
   { "Label", SEC_CONF | SEC_PPD | SEC_LPOPT, {{ 0 }} },
   { "LogType", SEC_CONF | SEC_PPD, {{ 7 }} },
   { "LowerCase", SEC_CONF | SEC_PPD, {{ 1 }} },
   { "TitlePref", SEC_CONF | SEC_PPD | SEC_LPOPT, {{ 0 }} },
   { "DecodeHexStrings", SEC_CONF | SEC_PPD, {{ 0 }} },
   { "FixNewlines", SEC_CONF | SEC_PPD, {{ 0 }} },
   { "AllowUnsafeOptions", SEC_CONF | SEC_PPD, {{ 0 }} },
   { "AnonUmask", SEC_CONF | SEC_PPD, {{ 0000 }} },
   { "UserUMask", SEC_CONF | SEC_PPD | SEC_LPOPT, {{ 0077 }} },
   { "Resolution", SEC_CONF | SEC_PPD | SEC_LPOPT, { "" } },
   { "Endpoint", SEC_CONF | SEC_PPD | SEC_LPOPT, { ENDPOINT } },
   { "AuthUser", SEC_CONF | SEC_PPD | SEC_LPOPT, { "0" } },
   { "AuthPwd", SEC_CONF | SEC_PPD | SEC_LPOPT, { "" } },
   { "FaxNumber", SEC_CONF | SEC_PPD | SEC_LPOPT, { "" } },
   { "FaxHeader", SEC_CONF | SEC_PPD | SEC_LPOPT, { "" } },
   { "SendingFaxID", SEC_CONF | SEC_PPD | SEC_LPOPT, { "" } },
   { "EMailAddress", SEC_CONF | SEC_PPD | SEC_LPOPT, { "" } },
   { "FaxResolution", SEC_CONF | SEC_PPD | SEC_LPOPT, { "norm" } },
   { "FaxRendering", SEC_CONF | SEC_PPD | SEC_LPOPT, { "grey-dithered" } },
   { "FaxDelay", SEC_CONF | SEC_PPD | SEC_LPOPT, {{ 0 }} },
   { "FaxMaxRetry", SEC_CONF | SEC_PPD | SEC_LPOPT, {{ 0 }} },
   { "Preview", SEC_CONF | SEC_PPD | SEC_LPOPT, { "no" } },
};

#define Conf_AnonDirName(cfg)          (&(cfg)[AnonDirName].value.sval[0])
#define Conf_AnonUser(cfg)             (&(cfg)[AnonUser].value.sval[0])
#define Conf_GhostScript(cfg)          (&(cfg)[GhostScript].value.sval[0])
#define Conf_GSCall(cfg)               (&(cfg)[GSCall].value.sval[0])
#define Conf_Grp(cfg)                  (&(cfg)[Grp].value.sval[0])
#define Conf_GSTmp(cfg)                (&(cfg)[GSTmp].value.sval[0])
#define Conf_Log(cfg)                  (&(cfg)[Log].value.sval[0])
#define Conf_PDFVer(cfg)               (&(cfg)[PDFVer].value.sval[0])
#define Conf_PostProcessing(cfg)       (&(cfg)[PostProcessing].value.sval[0])
#define Conf_HomeConf(cfg)             (&(cfg)[HomeConf].value.sval[0])
#define Conf_Out(cfg)                  (&(cfg)[Out].value.sval[0])
#define Conf_Spool(cfg)                (&(cfg)[Spool].value.sval[0])
#define Conf_UserPrefix(cfg)           (&(cfg)[UserPrefix].value.sval[0])
#define Conf_RemovePrefix(cfg)         (&(cfg)[RemovePrefix].value.sval[0])
#define Conf_OutExtension(cfg)         (&(cfg)[OutExtension].value.sval[0])
#define Conf_Cut(cfg)                  (cfg)[Cut].value.ival
#define Conf_Truncate(cfg)             (cfg)[Truncate].value.ival
#define Conf_DirPrefix(cfg)            (cfg)[DirPrefix].value.ival
#define Conf_Label(cfg)                (cfg)[Label].value.ival
#define Conf_LogType(cfg)              (cfg)[LogType].value.ival
#define Conf_LowerCase(cfg)            (cfg)[LowerCase].value.ival
#define Conf_TitlePref(cfg)            (cfg)[TitlePref].value.ival
#define Conf_DecodeHexStrings(cfg)     (cfg)[DecodeHexStrings].value.ival
#define Conf_FixNewlines(cfg)          (cfg)[FixNewlines].value.ival
#define Conf_AllowUnsafeOptions(cfg)   (cfg)[AllowUnsafeOptions].value.ival
#define Conf_AnonUMask(cfg)            (cfg)[AnonUMask].value.modval
#define Conf_UserUMask(cfg)            (cfg)[UserUMask].value.modval
#define Conf_Resolution(cfg)           (&(cfg)[Resolution].value.sval[0])
#define Conf_Endpoint(cfg)             (&(cfg)[Endpoint].value.sval[0])
#define Conf_AuthUser(cfg)             (&(cfg)[AuthUser].value.sval[0])
#define Conf_AuthPwd(cfg)              (&(cfg)[AuthPwd].value.sval[0])
#define Conf_FaxNumber(cfg)            (&(cfg)[FaxNumber].value.sval[0])
#define Conf_FaxHeader(cfg)            (&(cfg)[FaxHeader].value.sval[0])
#define Conf_SendingFaxID(cfg)         (&(cfg)[SendingFaxID].value.sval[0])
#define Conf_EMailAddress(cfg)         (&(cfg)[EMailAddress].value.sval[0])
#define Conf_FaxResolution(cfg)        (&(cfg)[FaxResolution].value.sval[0])
#define Conf_FaxRendering(cfg)         (&(cfg)[FaxRendering].value.sval[0])
#define Conf_FaxDelay(cfg)             (cfg)[FaxDelay].value.ival
#define Conf_FaxMaxRetry(cfg)          (cfg)[FaxMaxRetry].value.ival
#define Conf_Preview(cfg)              (&(cfg)[Preview].value.sval[0])


struct ConfigData* create_config();

void free_config(struct ConfigData* cfg);

int _assign_value(struct ConfigData* cfg, int security, char *key, char *value);

int read_config_file(struct ConfigData* cfg, char *filename);

void read_config_ppd(struct ConfigData* cfg);

void read_config_options(struct ConfigData* cfg, const char *lpoptions);

void dump_configuration(struct ConfigData* cfg, char *user);

char* preparedirname(struct ConfigData* cfg, char *src, struct passwd *passwd, char *uname);

struct ConfigData* init(char *argv[]);

int init2(struct ConfigData* cfg, char *argv[], struct passwd *passwd);

#endif
