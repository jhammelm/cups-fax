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

#include "logging.h"

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

enum configOptions
{
   AnonDirName, AnonUser, GhostScript, GSCall, Grp, GSTmp, Log, PDFVer, PostProcessing, HomeConf, Out, Spool, UserPrefix, RemovePrefix, OutExtension, Cut, Truncate, DirPrefix, Label, LogType, LowerCase, TitlePref, DecodeHexStrings, FixNewlines, AllowUnsafeOptions, AnonUMask, UserUMask, Resolution, Endpoint, AuthUser, AuthPwd, FaxNumber, FaxHeader, SendingFaxID, EMailAddress, FaxResolution, FaxRendering, FaxDelay, FaxMaxRetry, Preview, END_OF_OPTIONS
};

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

static struct ConfigData configData[] = {
   { "AnonDirName", SEC_CONF | SEC_PPD,{ "/var/spool/cups-fax/ANONYMOUS"}},
   { "AnonUser", SEC_CONF | SEC_PPD,{ "nobody"}},
   { "GhostScript", SEC_CONF | SEC_PPD,{ "/usr/bin/gs"}},
   { "GSCall", SEC_CONF | SEC_PPD,{ "%s -q -dCompatibilityLevel=%s -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pdfwrite -sOutputFile=\"%s\" -dAutoRotatePages=/PageByPage -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode -dPDFSETTINGS=/prepress %s %s"}},
   { "Grp", SEC_CONF | SEC_PPD,{ "lp"}},
   { "GSTmp", SEC_CONF | SEC_PPD,{ "TMPDIR=/var/tmp"}},
   { "Log", SEC_CONF | SEC_PPD,{ "/var/log/cups"}},
   { "PDFVer", SEC_CONF | SEC_PPD | SEC_LPOPT,{ "1.4"}},
   { "PostProcessing", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ""}},
   { "HomeConf", SEC_CONF | SEC_PPD,{ "/home/${USER}/.cups-fax.conf"}},
   { "Out", SEC_CONF | SEC_PPD,{ "/var/spool/cups-fax/${USER}"}},
   { "Spool", SEC_CONF | SEC_PPD,{ "/var/spool/cups-fax/SPOOL"}},
   { "UserPrefix", SEC_CONF | SEC_PPD,{ ""}},
   { "RemovePrefix", SEC_CONF | SEC_PPD,{ ""}},
   { "OutExtension", SEC_CONF | SEC_PPD | SEC_LPOPT,{ "pdf"}},
   { "Cut", SEC_CONF | SEC_PPD | SEC_LPOPT,{
         { 3}}},
   { "Truncate", SEC_CONF | SEC_PPD | SEC_LPOPT,{
         { 64}}},
   { "DirPrefix", SEC_CONF | SEC_PPD,{
         { 0}}},
   { "Label", SEC_CONF | SEC_PPD | SEC_LPOPT,{
         { 0}}},
   { "LogType", SEC_CONF | SEC_PPD,{
         { 7}}},
   { "LowerCase", SEC_CONF | SEC_PPD,{
         { 1}}},
   { "TitlePref", SEC_CONF | SEC_PPD | SEC_LPOPT,{
         { 0}}},
   { "DecodeHexStrings", SEC_CONF | SEC_PPD,{
         { 0}}},
   { "FixNewlines", SEC_CONF | SEC_PPD,{
         { 0}}},
   { "AllowUnsafeOptions", SEC_CONF | SEC_PPD,{
         { 0}}},
   { "AnonUmask", SEC_CONF | SEC_PPD,{
         { 0000}}},
   { "UserUMask", SEC_CONF | SEC_PPD | SEC_LPOPT,{
         { 0077}}},
   { "Resolution", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ""}},
   { "Endpoint", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ENDPOINT}},
   { "AuthUser", SEC_CONF | SEC_PPD | SEC_LPOPT,{ "0"}},
   { "AuthPwd", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ""}},
   { "FaxNumber", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ""}},
   { "FaxHeader", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ""}},
   { "SendingFaxID", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ""}},
   { "EMailAddress", SEC_CONF | SEC_PPD | SEC_LPOPT,{ ""}},
   { "FaxResolution", SEC_CONF | SEC_PPD | SEC_LPOPT,{ "norm"}},
   { "FaxRendering", SEC_CONF | SEC_PPD | SEC_LPOPT,{ "grey-dithered"}},
   { "FaxDelay", SEC_CONF | SEC_PPD | SEC_LPOPT,{
         { 0}}},
   { "FaxMaxRetry", SEC_CONF | SEC_PPD | SEC_LPOPT,{
         { 0}}},
   { "Preview", SEC_CONF | SEC_PPD | SEC_LPOPT,{ "no"}},
};

#define Conf_AnonDirName          configData[AnonDirName].value.sval
#define Conf_AnonUser             configData[AnonUser].value.sval
#define Conf_GhostScript          configData[GhostScript].value.sval
#define Conf_GSCall               configData[GSCall].value.sval
#define Conf_Grp                  configData[Grp].value.sval
#define Conf_GSTmp                configData[GSTmp].value.sval
#define Conf_Log                  configData[Log].value.sval
#define Conf_PDFVer               configData[PDFVer].value.sval
#define Conf_PostProcessing       configData[PostProcessing].value.sval
#define Conf_HomeConf             configData[HomeConf].value.sval
#define Conf_Out                  configData[Out].value.sval
#define Conf_Spool                configData[Spool].value.sval
#define Conf_UserPrefix           configData[UserPrefix].value.sval
#define Conf_RemovePrefix         configData[RemovePrefix].value.sval
#define Conf_OutExtension         configData[OutExtension].value.sval
#define Conf_Cut                  configData[Cut].value.ival
#define Conf_Truncate             configData[Truncate].value.ival
#define Conf_DirPrefix            configData[DirPrefix].value.ival
#define Conf_Label                configData[Label].value.ival
#define Conf_LogType              configData[LogType].value.ival
#define Conf_LowerCase            configData[LowerCase].value.ival
#define Conf_TitlePref            configData[TitlePref].value.ival
#define Conf_DecodeHexStrings     configData[DecodeHexStrings].value.ival
#define Conf_FixNewlines          configData[FixNewlines].value.ival
#define Conf_AllowUnsafeOptions   configData[AllowUnsafeOptions].value.ival
#define Conf_AnonUMask            configData[AnonUMask].value.modval
#define Conf_UserUMask            configData[UserUMask].value.modval
#define Conf_Resolution           configData[Resolution].value.sval
#define Conf_Endpoint             configData[Endpoint].value.sval
#define Conf_AuthUser             configData[AuthUser].value.sval
#define Conf_AuthPwd              configData[AuthPwd].value.sval
#define Conf_FaxNumber            configData[FaxNumber].value.sval
#define Conf_FaxHeader            configData[FaxHeader].value.sval
#define Conf_SendingFaxID         configData[SendingFaxID].value.sval
#define Conf_EMailAddress         configData[EMailAddress].value.sval
#define Conf_FaxResolution        configData[FaxResolution].value.sval
#define Conf_FaxRendering         configData[FaxRendering].value.sval
#define Conf_FaxDelay             configData[FaxDelay].value.ival
#define Conf_FaxMaxRetry          configData[FaxMaxRetry].value.ival
#define Conf_Preview              configData[Preview].value.sval


int _assign_value(int security, char *key, char *value);

int read_config_file(char *filename);

void read_config_ppd();

void read_config_options(const char *lpoptions);

void dump_configuration(char *user);

char* preparedirname(char *src, struct passwd *passwd, char *uname);

int init(char *argv[]);

int init2(char *argv[], struct passwd *passwd);

#endif