/* cups-pdf.c -- CUPS Backend (version 3.0.2, 2025-03-23)
   08.02.2003, Volker C. Behr
   volker@cups-pdf.de
   http://www.cups-pdf.de

   This code may be freely distributed as long as this header 
   is preserved. 

   This code is distributed under the GPL.
   (http://www.gnu.org/copyleft/gpl.html)

   ---------------------------------------------------------------------------

   Copyright (C) 2003-2025  Volker C. Behr

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   ---------------------------------------------------------------------------
  
   If you want to redistribute modified sources/binaries this header
   has to be preserved and all modifications should be clearly 
   indicated.
   In case you want to include this code into your own programs 
   I would appreciate your feedback via email.

  
   HISTORY: see ChangeLog in the parent directory of the source archive
 */

#include "cups-pdf.h"
#include "read-file.h"
#include "stub.h"
#include "logging.h"


/***************************************************************************************
 ** announce_printers()
 ****************************************************************************************
 **
 ** This function will report some initial printer/driver information
 ** back to CUPS, so that CUPS can place any cups-pdf printers into
 ** it's available printers list.
 **
 ** Accepts:	nothing
 ** Returns:	nothing
 **
 */
void
announce_printers()
{
   DIR *dir;
   struct dirent *config_ent;
   int len;
   cp_string setup;

   printf("file cups-fax:/ \"Virtual PDF/FAX Printer\" \"CUPS-FAX\" \"MFG:GTC;MDL:CUPS-FAX Printer;DES:GTC CUPS-FAX Printer;CLS:PRINTER;CMD:POSTSCRIPT;\"\n");

   if ((dir = opendir(CP_CONFIG_PATH)) != NULL)
   {
      while ((config_ent = readdir(dir)) != NULL)
      {
         len = strlen(config_ent->d_name);
         if ((strncmp(config_ent->d_name, "cups-fax-", 9) == 0) &&
             (len > 14 && strcmp(config_ent->d_name + len - 5, ".conf") == 0))
         {
            strncpy(setup, config_ent->d_name + 14, BUFSIZE > len - 14 ? len - 14 : BUFSIZE);
            setup[BUFSIZE > len - 14 ? len - 14 : BUFSIZE - 1] = '\0';
            printf("file cups-fax:/%s \"Virtual %s Printer\" \"CUPS-FAX-\" \"MFG:GTC;MDL:CUPS-FAX Printer;DES:GTC CUPS-FAX Printer;CLS:PRINTER;CMD:POSTSCRIPT;\"\n", setup, setup);
         }
      }
      closedir(dir);
   }
   return;
}

/***************************************************************************************
 ** prepareuser(passwd,dirname)
 ****************************************************************************************
 ** Establish user access to the PDF output directory 
 **
 ** Accepts:	pointer to struct passwd of user requesting print
 **		pointer to string containing path to output directory
 **
 ** Returns:	int	0 returned on success
 **			1 returned on all error conditions
 */
int
prepareuser(struct ConfigData* cfg, struct passwd *passwd, char *dirname)
{
   struct stat fstatus;

   (void) umask(0000);
   if (stat(dirname, &fstatus) || !S_ISDIR(fstatus.st_mode))
   {
      if (!strcmp(passwd->pw_name, Conf_AnonUser(cfg)))
      {
         if (create_dir(dirname, 0))
         {
            log_event(CPERROR, "failed to create anonymous output directory: %s", dirname);
            return 1;
         }
         if (chmod(dirname, (mode_t) (0777 & ~Conf_AnonUMask(cfg))))
         {
            log_event(CPERROR, "failed to set mode on anonymous output directory: %s", dirname);
            return 1;
         }
         log_event(CPDEBUG, "anonymous output directory created: %s", dirname);
      }
      else
      {
         if (create_dir(dirname, 0))
         {
            log_event(CPERROR, "failed to create user output directory: %s", dirname);
            return 1;
         }
         if (chmod(dirname, (mode_t) (0777 & ~Conf_UserUMask(cfg))))
         {
            log_event(CPERROR, "failed to set mode on user output directory: %s", dirname);
            return 1;
         }
         log_event(CPDEBUG, "user output directory created: %s", dirname);
      }
      if (chown(dirname, passwd->pw_uid, passwd->pw_gid))
      {
         log_event(CPERROR, "failed to set owner for output directory: %s", passwd->pw_name);
         return 1;
      }
      log_event(CPDEBUG, "owner set for output directory: %s", passwd->pw_name);
   }
   (void) umask(0077);
   return 0;
}

/***************************************************************************************
 ** decode_ps_hex_string(char *string)
 ****************************************************************************************
 **
 ** Decode postscript hex string in place within string
 **
 ** Accepts:	char pointer to a string containing the postscript hexadecimal string
 ** Returns:	nothing
 **
 ** NB: no validation is done here, please use is_ps_hex_string for that
 */
void
decode_ps_hex_string(char *string)
{
   char *src_ptr, *dst_ptr;
   int is_lower_digit; /* 0 - higher digit, 1 - lower digit */
   char number, digit;

   dst_ptr = string; /* we should always be behind src_ptr,
                   			   		   so it's safe to write over original string */
   number = (char) 0;
   is_lower_digit = 0;
   for (src_ptr = string + 1; *src_ptr != '>'; src_ptr++)
   { /* begin after start marker */
      if (*src_ptr == ' ' || *src_ptr == '\t')
      { /* skip whitespace */
         continue;
      }
      if (*src_ptr >= 'a')
      { /* assuming 0 < A < a */
         digit = *src_ptr - 'a' + (char) 10;
      }
      else if (*src_ptr >= 'A')
      {
         digit = *src_ptr - 'A' + (char) 10;
      }
      else
      {
         digit = *src_ptr - '0';
      }
      if (is_lower_digit)
      {
         number |= digit;
         *dst_ptr = number; /* write character */
         dst_ptr++;
         is_lower_digit = 0;
      }
      else
      { /* higher digit */
         number = digit << 4;
         is_lower_digit = 1;
      }
   }
   if (is_lower_digit)
   { /* write character with lower digit = 0,
    							   as per PostScript Language Reference */
      *dst_ptr = number;
      dst_ptr++;
      /* is_lower_digit=0; */
   }
   *dst_ptr = 0; /* finish him! */
   return;
}

/***************************************************************************************
 ** is_ps_hex_string(char *string)
 ****************************************************************************************
 **
 ** Determine if given string contains postscript hex strings
 **
 ** Accepts:	char pointer to a string containing the postscript "title"
 ** Returns:	an int indicating whether (1) or not (0) the input string
 **		contains a valid postscript "hex" string
 */
int
is_ps_hex_string(char *string)
{
   int got_end_marker = 0;
   char *ptr;

   if (string[0] != '<')
   { /* if has no start marker */
      log_event(CPDEBUG, "not a hex string, has no start marker: %s", string);
      return 0; /* not hex string, obviously */
   }
   for (ptr = string + 1; *ptr; ptr++)
   { /* begin after start marker */
      if (got_end_marker)
      { /* got end marker and still something left */
         log_event(CPDEBUG, "not a hex string, trailing characters after end marker: %s", ptr);
         return 0; /* that's bad! */
      }
      else if (*ptr == '>')
      { /* here it is! */
         got_end_marker = 1;
         log_event(CPDEBUG, "got an end marker in the hex string, expecting 0-termination: %s", ptr);
      }
      else if (!(
                 isxdigit(*ptr) ||
                 *ptr == ' ' ||
                 *ptr == '\t'
                 ))
      {
         log_event(CPDEBUG, "not a hex string, invalid character: %s", ptr);
         return 0; /* that's bad, too */
      }
   }
   return got_end_marker;
}

/***************************************************************************************
 ** alternate_replace_string(string)
 ****************************************************************************************
 **
 ** UTILITY to examine string and replace special characters with underscore
 **
 ** Accepts:	pointer to char, string to have special characters replaced
 ** Updates:	in place, the string pointed to by the input pointer
 ** Returns:	nothing
 **
 ** Allowed characters: 0-9, A-Z, a-z, '-', '+', '.', any character above 0x7f
 ** All other characters converted to underscore ('_')
 ** NB: this function alters the string pointed to by the input parameter
 */
void
alternate_replace_string(char *string)
{
   unsigned int i;

   log_event(CPDEBUG, "removing alternate special characters from title: %s", string);
   for (i = 0; i < (unsigned int) strlen(string); i++)
      if (isascii(string[i]) && /* leaving non-ascii characters intact */
          (!isalnum(string[i])) &&
          string[i] != '-' && string[i] != '+' && string[i] != '.')
         string[i] = '_';
   return;
}

/***************************************************************************************
 ** replace_string(string)
 ****************************************************************************************
 **
 ** UTILITY to examine string and replace special characters with underscore
 **
 ** Accepts:	pointer to char, string to have special characters replaced
 ** Updates:	in place, the string pointed to by the input pointer
 ** Returns:	nothing
 **
 ** Allowed characters: 0-9, A-Z, a-z, '-', '+', '.'
 ** All other characters converted to underscore ('_')
 ** NB: this function alters the string pointed to by the input parameter
 */
void
replace_string(char *string)
{
   unsigned int i;

   log_event(CPDEBUG, "removing special characters from title: %s", string);
   for (i = 0; i < (unsigned int) strlen(string); i++)
      if ((string[i] < '0' || string[i] > '9') &&
          (string[i] < 'A' || string[i] > 'Z') &&
          (string[i] < 'a' || string[i] > 'z') &&
          string[i] != '-' && string[i] != '+' && string[i] != '.')
         string[i] = '_';
   return;
}

/***************************************************************************************
 ** preparetitle(title)
 ****************************************************************************************
 **
 ** Using title prepared by caller, modify the title string to decode hex strings,
 **   remove leading and trailing underscores, parenthesis pairs, slashes,
 **   backslashes and "filename extensions". If required, truncate title to given
 **   length
 **
 **
 ** Accepts:	pointer to character string containing base document title
 **		(may point to an "empty" string)
 **
 ** Returns:	truth value as to whether <<title>> points to a valid title or not
 **		  0 if title is empty
 **		  non-zero if title contains a string
 **
 ** Configuration values used:
 **   DecodeHexStrings	flag to enable decoding of hex strings to allow internationalized titles
 **   Cut		flag to enable removing file name extensions
 **   Truncate		truncate long filenames to a maximum of specified characters
 */
int
preparetitle(struct ConfigData* cfg, char *title)
{
   char *cut;
   int i;

   if (title != NULL)
   {
      if (Conf_DecodeHexStrings(cfg))
      {
         log_event(CPSTATUS, "***Experimental Option: DecodeHexStrings");
         log_event(CPDEBUG, "checking for hex strings: %s", title);
         if (is_ps_hex_string(title))
            decode_ps_hex_string(title);
         log_event(CPDEBUG, "calling alternate_replace_string");
         alternate_replace_string(title);
      }
      else
      {
         replace_string(title);
      }
      i = strlen(title);
      if (i > 1)
      {
         while (title[--i] == '_');
         if (i < strlen(title) - 1)
         {
            log_event(CPDEBUG, "removing trailing _ from title: %s", title);
            title[i + 1] = '\0';
         }
         i = 0;
         while (title[i++] == '_');
         if (i > 1)
         {
            log_event(CPDEBUG, "removing leading _ from title: %s", title);
            memmove(title, title + i - 1, strlen(title) - i + 2);
         }
      }
      while (strlen(title) > 2 && title[0] == '(' && title[strlen(title) - 1] == ')')
      {
         log_event(CPDEBUG, "removing enclosing parentheses () from full title: %s", title);
         title[strlen(title) - 1] = '\0';
         memmove(title, title + 1, strlen(title));
      }
   }
   cut = strrchr(title, '/');
   if (cut != NULL)
   {
      log_event(CPDEBUG, "removing slashes from full title: %s", title);
      memmove(title, cut + 1, strlen(cut + 1) + 1);
   }
   cut = strrchr(title, '\\');
   if (cut != NULL)
   {
      log_event(CPDEBUG, "removing backslashes from full title: %s", title);
      memmove(title, cut + 1, strlen(cut + 1) + 1);
   }
   cut = strrchr(title, '.');
   if ((cut != NULL) && ((int) strlen(cut) <= Conf_Cut(cfg) + 1) && (cut != title))
   {
      log_event(CPDEBUG, "removing file name extension: %s", cut);
      cut[0] = '\0';
   }
   if (strlen(title) > Conf_Truncate(cfg))
   {
      title[Conf_Truncate(cfg)] = '\0';
      log_event(CPDEBUG, "truncating title: %s", title);
   }
   return strcmp(title, "");
}

/***************************************************************************************
 ** fgets2(fbuffer,bufsize,ffpsrc)
 ****************************************************************************************
 ** Reads in at most one less than bufsize characters from stream and stores them into
 ** the buffer pointed to by fbuffer. Reading stops after an EOF, or 0x0A, 0x0C or 0x0D.
 **
 ** Accepts:	pointer to char pre-allocated buffer in which to store input data
 **		int size of pre-allocated buffer in bytes
 **		pointer to FILE handle of file to read input from
 **
 ** Returns:	pointer to char of data stored in pre-allocated buffer, or NULL on error
 **
 ** NB: like fgets() but linedelimiters are 0x0A, 0x0C, 0x0D (ASCII LF, FF, CR).
 ** NB: if FixNewlines config option not set, this devolves to fgets()
 **
 ** Configuration values used:
 **   FixNewlines	flag to enable alternate line endings (LF, FF, CR) (experimental)
 */
char *
fgets2(struct ConfigData* cfg, char *fbuffer, int fbufsize, FILE *ffpsrc)
{
   /* like fgets() but linedelimiters are 0x0A, 0x0C, 0x0D (LF, FF, CR). */
   int c, pos;
   char *result;

   if (!Conf_FixNewlines(cfg))
      return fgets(fbuffer, fbufsize, ffpsrc);

   result = NULL;
   pos = 0;

   while (pos < fbufsize)
   { /* pos in [0..fbufsize-1] */
      c = fgetc(ffpsrc); /* converts CR/LF to LF in some OSses */
      if (c == EOF) /* EOF _or_ error */
         break;
      fbuffer[pos++] = c;
      if (c == 0x0A || c == 0x0C || c == 0x0D) /* line is at an end */
         break;
   }

   if (pos > 0 && !ferror(ffpsrc))
   { /* at least one char read and no error */
      fbuffer[pos] = '\0';
      result = fbuffer;
   }

   return result;
}

/***************************************************************************************
 ** preparespoolfile(fpsrc,spoolfile,title,cmdtitle,job,passwd)
 ****************************************************************************************
 **
 ** cups hands us print data either through a named file (main() argv[6])
 ** or through stdin. This function spools that input data to a cups-pdf
 ** spool file for processing. As this function transfers data, it may
 ** extract or set the title embedded in the postscript.
 **
 ** Accepts:	FILE * of input print data
 **		char * of path to cups-pdf internal spool file
 **		char * of title to apply to output PDF
 **		char * of title provided by CUPS (argv[3])
 **		int of cups job number (argv[1] converted to int)
 **		struct passwd * of passwd entry associated with target user
 **
 ** Returns:	integer 0 on success, 1 on failure
 **
 ** Configuration values used:
 **   FixNewlines	this option will try to fix various unusal line delimiters (experimental)
 **			0: disable, 1: enable
 **   TitlePref		where to look first for a title when creating the output filename
 **			0: prefer title from %Title statement in the PS file
 **			1: prefer title passed via commandline
 **   Label		label all jobs with a unique job-id
 **			0: label untitled documents only
 **			1: label all documents with a preceeding "job_#-"
 **			2: label all documents with a tailing "-job_#"
 */
int
preparespoolfile(struct ConfigData* cfg, FILE *fpsrc, char *spoolfile,
                 char *title, char *cmdtitle,
                 int job, struct passwd *passwd)
{
   cp_string buffer;
   int rec_depth, is_title = 0;
   FILE *fpdest;

   if (fpsrc == NULL)
   {
      log_event(CPERROR, "failed to open source stream");
      return 1;
   }
   log_event(CPDEBUG, "source stream ready");
   fpdest = fopen(spoolfile, "w");
   if (fpdest == NULL)
   {
      log_event(CPERROR, "failed to open spoolfile: %s", spoolfile);
      (void) fclose(fpsrc);
      return 1;
   }
   log_event(CPDEBUG, "destination stream ready: %s", spoolfile);
   if (chown(spoolfile, passwd->pw_uid, -1))
   {
      log_event(CPERROR, "failed to set owner for spoolfile: %s", spoolfile);
      return 1;
   }
   log_event(CPDEBUG, "owner set for spoolfile: %s", spoolfile);
   rec_depth = 0;
   if (Conf_FixNewlines(cfg))
      log_event(CPSTATUS, "***Experimental Option: FixNewlines");
   else
      log_event(CPDEBUG, "using traditional fgets");
   while (fgets2(cfg, buffer, BUFSIZE, fpsrc) != NULL)
   {
      if (!strncmp(buffer, "%!", 2) && strncmp(buffer, "%!PS-AdobeFont", 14))
      {
         log_event(CPDEBUG, "found beginning of postscript code: %s", buffer);
         break;
      }
   }
   log_event(CPDEBUG, "now extracting postscript code");
   (void) fputs(buffer, fpdest);
   while (fgets2(cfg, buffer, BUFSIZE, fpsrc) != NULL)
   {
      (void) fputs(buffer, fpdest);
      if (!is_title && !rec_depth)
         if (sscanf(buffer, "%%%%Title: %"TBUFSIZE"c", title) == 1)
         {
            log_event(CPDEBUG, "found title in ps code: %s", title);
            is_title = 1;
         }
      if (!strncmp(buffer, "%!", 2))
      {
         log_event(CPDEBUG, "found embedded (e)ps code: %s", buffer);
         rec_depth++;
      }
      else if (!strncmp(buffer, "%%EOF", 5))
      {
         if (!rec_depth)
         {
            log_event(CPDEBUG, "found end of postscript code: %s", buffer);
            break;
         }
         else
         {
            log_event(CPDEBUG, "found end of embedded (e)ps code: %s", buffer);
            rec_depth--;
         }
      }
   }
   (void) fclose(fpdest);
   (void) fclose(fpsrc);
   log_event(CPDEBUG, "all data written to spoolfile: %s", spoolfile);

   if (cmdtitle == NULL || !strcmp(cmdtitle, "(stdin)"))
      buffer[0] = '\0';
   else
      strncpy(buffer, cmdtitle, BUFSIZE);
   if (title == NULL || !strcmp(title, "((stdin))"))
      title[0] = '\0';

   if (Conf_TitlePref(cfg))
   {
      log_event(CPDEBUG, "trying to use commandline title: %s", buffer);
      if (!preparetitle(cfg, buffer))
      {
         log_event(CPDEBUG, "empty commandline title, using PS title: %s", title);
         if (!preparetitle(cfg, title))
            log_event(CPDEBUG, "empty PS title");
      }
      else
         snprintf(title, BUFSIZE, "%s", buffer);
   }
   else
   {
      log_event(CPDEBUG, "trying to use PS title: %s", title);
      if (!preparetitle(cfg, title))
      {
         log_event(CPDEBUG, "empty PS title, using commandline title: %s", buffer);
         if (!preparetitle(cfg, buffer))
            log_event(CPDEBUG, "empty commandline title");
         else
            snprintf(title, BUFSIZE, "%s", buffer);
      }
   }

   if (!strcmp(title, ""))
   {
      if (Conf_Label(cfg) == 2)
         snprintf(title, BUFSIZE, "untitled_document-job_%i", job);
      else
         snprintf(title, BUFSIZE, "job_%i-untitled_document", job);
      log_event(CPDEBUG, "no title found - using default value: %s", title);
   }
   else
   {
      if (Conf_Label(cfg))
      {
         strcpy(buffer, title);
         if (Conf_Label(cfg) == 2)
            snprintf(title, BUFSIZE, "%s-job_%i", buffer, job);
         else
            snprintf(title, BUFSIZE, "job_%i-%s", job, buffer);
      }
      log_event(CPDEBUG, "title successfully retrieved: %s", title);
   }
   return 0;
}

/***************************************************************************************
 ** main()
 ****************************************************************************************
 **
 ** mainline of cups-pdf
 ** 
 ** 1) check if running as privileged user, abort if not
 **
 ** 2) gross argument check:
 **    a) no arguments: announce printer and exit
 **    b) 5 or 6 args:  process print request
 **       NB: read print data from argv[6] (if given) or stdin (if absent)
 **    c) anything else: abort with error message
 **
 ** 3) process print request, abort on failure (see #2/b above):
 **    a) initialize configuration
 **    b) determine PDF output directory path
 **    c) determine user's group involvement (list of GIDs)
 **    d) establish user access to the PDF output directory
 **    e) generate input spoolfile filename
 **    f) create/prepare the input spoolfile
 **    g) generate output PDF file filename
 **    h) build custom ghostscript commandline
 **    i) ensure that the output file does not exist before ghostscript execution
 **    j) set ghostscript TMPDIR envvar
 **    k) fork a child to run ghostscript
 **    l) parent waits for ghostscript child process to terminate
 **    m) parent deallocates all previously-allocated resources
 **    n) parent terminates with 0 status code
 **
 ** 4) child process runs ghostscript (see #3/k above):
 **    a) set process uid, gid, groups to user's uid, gid, groups (see #3/c)
 **    b) set umask to 0077
 **    c) use system() to invoke ghostscript (#3/g, #3/h, #3/j)
 **    d) chmod the output file
 **    e) invoke optional output postprocessing
 **    f) child terminates with 0 status code
 **
 ** Accepts:	int of count of arguments in argument list
 **		char ** of argument list
 **			[0] holds name of this binary
 **			[1] holds the job ID
 **			[2] holds the user printing the job
 **			[3] holds the job name/title
 **			[4] holds the number of copies to print
 **			[5] holds the options that were provided when the job was submitted
 **			[6] holds the path of the file to print (optional, first program only)
 **
 ** Returns:	integer status of filter
 **	  	0	process completed (success, usage error)
 **	  	5	operational error (failed init, config or processing)
 **
 **
 ** Directly uses configuration values:
 **   UserPrefix	common prefix to user name
 **			  used to fix user name for getpwnam() call (required under some OS)
 **   LowerCase		flag used to force user id into lower case
 **			  TRUE when lower case required
 **			  FALSE when mixed/upper case permitted
 **   AnonUser		default user id for "anonymous" PDF printing
 **   AnonDirName	directory to place "anonymous" PDFs in
 **   AnonUMask		default "anonymous" umask for created file(s)
 **   UserUMask		umask for user output of known users
 **   Spool		CUPS-FAX spool directory
 **   OutExtension	filename suffix for output file (undocumented, defaults to "pdf")
 **   GSCall		command line for calling GhostScript
 **   GhostScript	path to GhostScript binary
 **   PDFVer		PDF version to be created
 **   GSTmp		path to tempfile directory during GhostScript operation
 **   PostProcessing	path to postprocessing script called after the creation of the PDF
 */
int
main(int argc, char *argv[])
{
   char *user, *dirname, *spoolfile, *outfile, *gscall, *ppcall;
   cp_string title;
   int size;
   mode_t mode;
   struct passwd *passwd;
   gid_t *groups;
   int ngroups;
   pid_t pid;
   errno = 0;

   if (setuid(0))
   {
      (void) fputs("CUPS-FAX cannot be called without root privileges!\n", stderr);
      return 0;
   }

   if (argc == 1)
   {
      announce_printers();
      return 0;
   }
   if (argc < 6 || argc > 7)
   {
      (void) fputs("Usage: cups-fax job-id user title copies options [file]\n", stderr);
      return 0;
   }

   struct ConfigData* cfg = init(argv);
   if (!cfg)
      return 5;

   log_event(CPDEBUG, "initialization (part 1) done");

   size = strlen(Conf_UserPrefix(cfg)) + strlen(argv[2]) + 1;
   user = calloc(size, sizeof (char));
   if (user == NULL)
   {
      (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
      return 5;
   }
   snprintf(user, size, "%s%s", Conf_UserPrefix(cfg), argv[2]);
   passwd = getpwnam(user);
   if (passwd == NULL && Conf_LowerCase(cfg))
   {
      log_event(CPDEBUG, "unknown user: %s", user);
      for (size = 0; size < (int) strlen(argv[2]); size++)
         argv[2][size] = tolower(argv[2][size]);
      log_event(CPDEBUG, "trying lower case user name: %s", argv[2]);
      size = strlen(Conf_UserPrefix(cfg)) + strlen(argv[2]) + 1;
      snprintf(user, size, "%s%s", Conf_UserPrefix(cfg), argv[2]);
      passwd = getpwnam(user);
   }
   if (passwd == NULL)
   {
      if (strlen(Conf_AnonUser(cfg)))
      {
         passwd = getpwnam(Conf_AnonUser(cfg));
         if (passwd == NULL)
         {
            log_event(CPERROR, "username for anonymous access unknown: %s", Conf_AnonUser(cfg));
            free(user);
            close_log();
            return 5;
         }
         log_event(CPDEBUG, "unknown user: %s", user);
         size = strlen(Conf_AnonDirName(cfg)) + 4;
         dirname = calloc(size, sizeof (char));
         if (dirname == NULL)
         {
            (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
            free(user);
            close_log();
            return 5;
         }
         snprintf(dirname, size, "%s", Conf_AnonDirName(cfg));
         while (strlen(dirname) && ((dirname[strlen(dirname) - 1] == '\n') ||
                                    (dirname[strlen(dirname) - 1] == '\r')))
            dirname[strlen(dirname) - 1] = '\0';
         log_event(CPDEBUG, "output directory name generated: %s", dirname);
      }
      else
      {
         log_event(CPSTATUS, "anonymous access denied: %s", user);
         free(user);
         close_log();
         return 0;
      }
      mode = (mode_t) (0666 & ~Conf_AnonUMask(cfg));
   }
   else
   {
      log_event(CPDEBUG, "user identified: %s", passwd->pw_name);

      if ((dirname = preparedirname(cfg, Conf_Out(cfg), passwd, argv[2])) == NULL)
      {
         (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
         free(user);
         close_log();
         return 5;
      }
      while (strlen(dirname) && ((dirname[strlen(dirname) - 1] == '\n') ||
                                 (dirname[strlen(dirname) - 1] == '\r')))
      {
         dirname[strlen(dirname) - 1] = '\0';
      }
      log_event(CPDEBUG, "output directory name generated: %s", dirname);
      mode = (mode_t) (0666 & ~Conf_UserUMask(cfg));
   }
   ngroups = 32;
   groups = calloc(ngroups, sizeof (gid_t));
   if (groups == NULL)
   {
      (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
      free(user);
      close_log();
      return 5;
   }
   size = getgrouplist(user, passwd->pw_gid, groups, &ngroups);
   if (size == -1)
   {
      free(groups);
      groups = calloc(ngroups, sizeof (gid_t));
      size = getgrouplist(user, passwd->pw_gid, groups, &ngroups);
   }
   if (size < 0)
   {
      log_event(CPERROR, "getgrouplist failed");
      free(user);
      free(groups);
      close_log();
      return 5;
   }
   free(user);
   if (prepareuser(cfg, passwd, dirname))
   {
      free(groups);
      free(dirname);
      close_log();
      return 5;
   }
   log_event(CPDEBUG, "user information prepared");

   if (init2(cfg, argv, passwd))
   {
      free(groups);
      free(dirname);
      close_log();
      return 5;
   }

   log_event(CPDEBUG, "initialization finished: %s", CPVERSION);

   size = strlen(Conf_Spool(cfg)) + 24;
   spoolfile = calloc(size, sizeof (char));
   if (spoolfile == NULL)
   {
      (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
      free(groups);
      free(dirname);
      close_log();
      return 5;
   }
   snprintf(spoolfile, size, "%s/cups2fax-%i", Conf_Spool(cfg), (int) getpid());
   log_event(CPDEBUG, "spoolfile name created: %s", spoolfile);

   if (argc == 6)
   {
      if (preparespoolfile(cfg, stdin, spoolfile, title, argv[3], atoi(argv[1]), passwd))
      {
         free(groups);
         free(dirname);
         free(spoolfile);
         close_log();
         return 5;
      }
      log_event(CPDEBUG, "input data read from stdin");
   }
   else
   {
      if (preparespoolfile(cfg, fopen(argv[6], "r"), spoolfile, title, argv[3], atoi(argv[1]), passwd))
      {
         free(groups);
         free(dirname);
         free(spoolfile);
         close_log();
         return 5;
      }
      log_event(CPDEBUG, "input data read from file: %s", argv[6]);
   }

   size = strlen(dirname) + strlen(title) + strlen(Conf_OutExtension(cfg)) + 3;
   outfile = calloc(size, sizeof (char));
   if (outfile == NULL)
   {
      (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
#ifndef CPTEST
      if (unlink(spoolfile))
         log_event(CPERROR, "failed to unlink spoolfile during clean-up: %s", spoolfile);
#endif
      free(groups);
      free(dirname);
      free(spoolfile);
      close_log();
      return 5;
   }
   if (strlen(Conf_OutExtension(cfg)))
      snprintf(outfile, size, "%s/%s.%s", dirname, title, Conf_OutExtension(cfg));
   else
      snprintf(outfile, size, "%s/%s", dirname, title);
   log_event(CPDEBUG, "output filename created: %s", outfile);

   size = strlen(Conf_GSCall(cfg)) + strlen(Conf_GhostScript(cfg)) + strlen(Conf_PDFVer(cfg)) + strlen(outfile) + strlen(spoolfile) + strlen(Conf_Resolution(cfg)) + 3 + 7;
   gscall = calloc(size, sizeof (char));
   if (gscall == NULL)
   {
      (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
#ifndef CPTEST
      if (unlink(spoolfile))
         log_event(CPERROR, "failed to unlink spoolfile during clean-up: %s", spoolfile);
#endif
      free(groups);
      free(dirname);
      free(spoolfile);
      free(outfile);
      close_log();
      return 5;
   }
   char* res = calloc(strlen(Conf_Resolution(cfg)) + 3, sizeof (char));
   if (res == NULL)
   {
      (void) fputs("CUPS-FAX: failed to allocate memory\n", stderr);
#ifndef CPTEST
      if (unlink(spoolfile))
         log_event(CPERROR, "failed to unlink spoolfile during clean-up: %s", spoolfile);
#endif
      free(groups);
      free(dirname);
      free(spoolfile);
      free(outfile);
      free(gscall);
      close_log();
      return 5;
   }
   if (strlen(Conf_Resolution(cfg)) > 0)
      snprintf(res, strlen(Conf_Resolution(cfg)) + 3, "-r%s", Conf_Resolution(cfg));
   snprintf(gscall, size, Conf_GSCall(cfg), Conf_GhostScript(cfg), Conf_PDFVer(cfg), outfile, res, spoolfile);
   log_event(CPDEBUG, "ghostscript commandline built: %s", gscall);

   (void) unlink(outfile);
   log_event(CPDEBUG, "(old) output file unlinked: %s", outfile);

   if (putenv(Conf_GSTmp(cfg)))
   {
      log_event(CPERROR, "insufficient space in environment to set TMPDIR: %s", Conf_GSTmp(cfg));
#ifndef CPTEST
      if (unlink(spoolfile))
         log_event(CPERROR, "failed to unlink spoolfile during clean-up: %s", spoolfile);
#endif
      free(groups);
      free(dirname);
      free(spoolfile);
      free(outfile);
      free(gscall);
      free(res);
      close_log();
      return 5;
   }
   log_event(CPDEBUG, "TMPDIR set for GhostScript: %s", getenv("TMPDIR"));

   pid = fork();

   if (!pid)
   {
      log_event(CPDEBUG, "entering child process");

      if (setgid(passwd->pw_gid))
         log_event(CPERROR, "failed to set GID for current user");
      else
         log_event(CPDEBUG, "GID set for current user");
      if (setgroups(ngroups, groups))
         log_event(CPERROR, "failed to set supplementary groups for current user");
      else
         log_event(CPDEBUG, "supplementary groups set for current user");
      if (setuid(passwd->pw_uid))
         log_event(CPERROR, "failed to set UID for current user: %s", passwd->pw_name);
      else
         log_event(CPDEBUG, "UID set for current user: %s", passwd->pw_name);

      (void) umask(0077);
      size = system(gscall);
      log_event(CPDEBUG, "ghostscript has finished: %d", size);
      if (chmod(outfile, mode))
         log_event(CPERROR, "failed to set file mode for PDF file: %s (non fatal)", outfile);
      else
         log_event(CPDEBUG, "file mode set for user output: %s", outfile);

      if (strlen(Conf_PostProcessing(cfg)))
      {
         size = strlen(Conf_PostProcessing(cfg)) + strlen(outfile) + strlen(passwd->pw_name) + strlen(argv[2]) + 4;
         ppcall = calloc(size, sizeof (char));
         if (ppcall == NULL)
            log_event(CPERROR, "failed to allocate memory for postprocessing (non fatal)");
         else
         {
            snprintf(ppcall, size, "%s %s %s %s", Conf_PostProcessing(cfg), outfile, passwd->pw_name, argv[2]);
            log_event(CPDEBUG, "postprocessing commandline built: %s", ppcall);
            size = system(ppcall);
            snprintf(title, BUFSIZE, "%d", size);
            log_event(CPDEBUG, "postprocessing has finished: %s", title);
            free(ppcall);
         }
      }
      else
         log_event(CPDEBUG, "no postprocessing");

      return 0;
   }
   log_event(CPDEBUG, "waiting for child to exit");
   (void) waitpid(pid, NULL, 0);
#ifndef CPTEST
   if (unlink(spoolfile))
      log_event(CPERROR, "failed to unlink spoolfile: %s (non fatal)", spoolfile);
   else
      log_event(CPDEBUG, "spoolfile unlinked: %s", spoolfile);
#endif
   char errortext[256];
   errortext[0] = '\0';
   int ec = call_stub(cfg, outfile, errortext);
   if (ec != 0) {
      log_event(CPERROR, "call of stub failed: error %d: %s", ec, errortext);
   }

   free(groups);
   free(dirname);
   free(spoolfile);
   free(outfile);
   free(gscall);
   free(res);

   log_event(CPDEBUG, "all memory has been freed");

   log_event(CPSTATUS, "PDF creation successfully finished for %s", passwd->pw_name);

   close_log();
   return ec;
}
