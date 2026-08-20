#include <pappl/pappl.h>
#include <string.h>

#ifndef CPV3
#include "config.h"
#endif

#include "read-file.h"
#include "stub.h"

#define MODEL "CUPS PDF-FAX v3 Printer"
#define DEVICE_ID "MFG:GTC;MDL:" MODEL ";CMD:PDF;"
#define DRIVER_NAME "cups3-fax"
#define PRINTER_QUEUE "CUPS3_FAX"
#define SOCKET "file:///tmp/" DRIVER_NAME ".out"
#define DRIVER_STATE "/tmp/" DRIVER_NAME ".state"

static const char * // O - Driver name or `NULL` for none
autoadd_callback(
    const char *device_info, // I - Device information string (not used)
    const char *device_uri,  // I - Device URI (not used)
    const char *device_id,   // I - IEEE-1284 device ID
    void *data)              // I - Callback data (not used)
{
   size_t num_did;            // Anzahl geparster Key-Value-Paare
   cups_option_t *did = NULL; // Array der Key-Value-Paare
   const char *cmd;           // Wert von "CMD" oder "COMMAND SET"
   const char *ret = NULL;    // Rückgabewert (Treibername)

   (void)device_info;
   (void)device_uri;
   (void)data;

   // Abfangschutzes: Wenn kein Device ID übergeben wurde, abbrechen
   if (!device_id)
      return (NULL);

   // IEEE-1284-String parsen (z.B. "MFG:GTC;MDL:Fax;CMD:PDF;")
   num_did = papplDeviceParseID(device_id, &did);

   // Befehlssatz auslesen
   if ((cmd = cupsGetOption("COMMAND SET", num_did, did)) == NULL)
      cmd = cupsGetOption("CMD", num_did, did);

   // Auf PDF-Unterstützung prüfen
   if (cmd && (strstr(cmd, "PDF") != NULL || strstr(cmd, "pdf") != NULL)) {
      ret = DRIVER_NAME; // Name deines Treibers aus dem drivers-Array
   }

   // Speicher der geparsten Optionen freigeben
   cupsFreeOptions(num_did, did);

   return (ret);
}

// Callback: send job/PDF to web service
static bool my_pdf_direct_callback(pappl_job_t *job,
                                   pappl_pr_options_t *options,
                                   pappl_device_t *device) {
   const char *value = NULL;

   // 1. Job-ID auslesen
   int job_id = papplJobGetID(job);

   // 2. Benutzernamen auslesen
   const char *username = papplJobGetUsername(job);

   // Falls der Username nicht ermittelt werden konnte (z.B. anonymer IPP-Druck)
   if (!username)
      username = "unknown";

   const char *docname = papplJobGetName(job);
   if (!docname)
      docname = "unknown";

   const char *filename = papplJobGetFilename(job);

   // 3. In den PAPPL-Log schreiben
   papplLogJob(job, PAPPL_LOGLEVEL_INFO,
               "Execute print job '%s' of user '%s' (Job-ID: %d)", docname,
               username, job_id);

   papplJobSetImpressions(job, 1);

   // 1. Papierformat auslesen
   papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Media size: %s (%dx%d mm)",
               options->media.size_name, options->media.size_width / 100,
               options->media.size_length / 100);

   // 2. Anzahl der Kopien
   papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Count copies: %d", options->copies);

   // 3. Erste Seite
   papplLogJob(job, PAPPL_LOGLEVEL_INFO, "First page: %d", options->first_page);

   // 4. Letzte Seite
   papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Last page: %d", options->last_page);

   // 5. Druckauflösung
   papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Resolution: %dx%d dpi",
               options->printer_resolution[0], options->printer_resolution[1]);

   // 6. Orientierung (Portrait/Landscape)
   if (options->orientation_requested == IPP_ORIENT_LANDSCAPE) {
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Orientation: landscape");
   }

   // Parameter abfragen...
   // maxretry
   ipp_attribute_t *attr = papplJobGetAttribute(job, "maxretry");
   unsigned int maxretry = 0;
   if (attr) {
      maxretry = ippGetInteger(attr, 0);
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Number of redials (maxretry): %d", maxretry);
   }

   // delay
   attr = papplJobGetAttribute(job, "delay");
   unsigned int delay = 0;
   if (attr) {
      delay = ippGetInteger(attr, 0);
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Delay, in hours (delay): %d", delay);
   }

   // Defaults: sq, tq, gq
   unsigned int sq = 18;
   unsigned int tq = 35;
   unsigned int gq = 14;

   // type
   const char *type = NULL;
   attr = papplJobGetAttribute(job, "type");
   if (attr) {
      type = ippGetString(attr, 0, NULL);
      if (strcmp(type, "text") == 0) {
         sq = 18;
         tq = 35;
         gq = 14;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Type of templates (type): %s", type);
      } else if (strcmp(type, "image") == 0) {
         sq = 10;
         tq = 180;
         gq = 16;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Type of templates (type): %s", type);
      } else if (strcmp(type, "text+image") == 0) {
         sq = 45;
         tq = 100;
         gq = 18;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Type of templates (type): %s", type);
      }
   }

   // sq
   attr = papplJobGetAttribute(job, "sq");
   if (attr) {
      sq = ippGetInteger(attr, 0);
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Sharpness (sq): %d", sq);
   }

   // tq
   attr = papplJobGetAttribute(job, "tq");
   if (attr) {
      tq = ippGetInteger(attr, 0);
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Contrast/Threshold (tq): %d", tq);
   }

   // gq
   attr = papplJobGetAttribute(job, "gq");
   if (attr) {
      gq = ippGetInteger(attr, 0);
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Brightness/Gamma (gq): %d", gq);
   }

   // faxresolution
   const char *faxresolution = "norm";
   attr = papplJobGetAttribute(job, "faxresolution");
   if (attr) {
      faxresolution = ippGetString(attr, 0, NULL);
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "FAX resolution (faxresolution): %s", faxresolution);
   }
   bool highResolution = (strcmp(faxresolution, "high") == 0);

   // faxrendering
   const char *faxrendering = "floyd-steinberg-sharp-threshold";
   attr = papplJobGetAttribute(job, "faxrendering");
   if (attr) {
      faxrendering = ippGetString(attr, 0, NULL);
      papplLogJob(job, PAPPL_LOGLEVEL_INFO, "FAX rendering (faxrendering): %s", faxrendering);
   }

   // rotate
   bool rotate = true;
   attr = papplJobGetAttribute(job, "rotate");
   rotate = attr ? ippGetBoolean(attr, 0) : true;
   papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Rotate? (rotate): %s",
               rotate ? "yes" : "no");

   // preview
   bool preview = false;
   attr = papplJobGetAttribute(job, "preview");
   preview = attr ? ippGetBoolean(attr, 0) : false;
   papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Preview (preview): %s",
               preview ? "yes" : "no");

   // faxheader
   const char *faxheader = NULL;
   attr = papplJobGetAttribute(job, "faxheader");
   if (attr) {
      value = ippGetString(attr, 0, NULL);
      if (strcmp(value, "none") != 0) {
         faxheader = value;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "FAX header (faxheader): %s", faxheader);
      }
   }

   // sendingfaxid
   const char *sendingfaxid = NULL;
   attr = papplJobGetAttribute(job, "sendingfaxid");
   if (attr) {
      value = ippGetString(attr, 0, NULL);
      if (strcmp(value, "none") != 0) {
         sendingfaxid = value;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "FAX sender ID (sendingfaxid): %s", sendingfaxid);
      }
   }

   // emailaddress
   const char *emailaddress = NULL;
   attr = papplJobGetAttribute(job, "emailaddress");
   if (attr) {
      value = ippGetString(attr, 0, NULL);
      if (strcmp(value, "none") != 0) {
         emailaddress = value;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Email address (emailaddress): %s", emailaddress);
      }
   }

   // authuser
   const char *authuser = NULL;
   attr = papplJobGetAttribute(job, "authuser");
   if (attr) {
      value = ippGetString(attr, 0, NULL);
      if (strcmp(value, "none") != 0) {
         authuser = value;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO,
                     "Authentication: username (authuser): %s", authuser);
      }
   }

   // authpwd
   const char *authpwd = NULL;
   attr = papplJobGetAttribute(job, "authpwd");
   if (attr) {
      value = ippGetString(attr, 0, NULL);
      if (strcmp(value, "none") != 0) {
         authpwd = value;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO,
                     "Authentication: password, length (authpwd): %d", (int)strlen(authpwd));
      }
   }

   // endpoint
   const char *endpoint = NULL;
   attr = papplJobGetAttribute(job, "endpoint");
   if (attr) {
      value = ippGetString(attr, 0, NULL);
      if (strcmp(value, "none") != 0) {
         endpoint = value;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "Endpoint of stub (endpoint): %s", endpoint);
      }
   }

   // faxnumber
   const char *faxnumber = NULL;
   attr = papplJobGetAttribute(job, "faxnumber");
   if (attr) {
      value = ippGetString(attr, 0, NULL);
      if (strcmp(value, "none") != 0) {
         faxnumber = value;
         papplLogJob(job, PAPPL_LOGLEVEL_INFO, "FAX number (faxnumber): %s", faxnumber);
      }
   }

   // 7. Sende (erstelltes) PDF zu ws-cups-fax
   char errortext[256];
   errortext[0] = '\0';
   int ec = stub((char *)endpoint, (char *)authuser, (char *)authpwd,
                 (char *)faxnumber, (char *)faxheader, (char *)sendingfaxid,
                 (char *)emailaddress, (char *)filename, &highResolution,
                 (char *)faxrendering, &delay, &maxretry, &preview, &rotate,
                 &sq, &tq, &gq, errortext);
   if (ec != 0) {
      papplLogJob(job, PAPPL_LOGLEVEL_ERROR,
                  "Got error at call of stub: ec=%d: %s", ec, errortext);
   }
   papplJobSetImpressionsCompleted(job, 1);

   return (true);
}

// Dummy-Callbacks für Rasterisierung definieren
static bool rstart_job_cb(pappl_job_t *job, pappl_pr_options_t *options,
                          pappl_device_t *device) {
   return true;
}

static bool rend_job_cb(pappl_job_t *job, pappl_pr_options_t *options,
                        pappl_device_t *device) {
   return true;
}

static bool rstart_page_cb(pappl_job_t *job, pappl_pr_options_t *options,
                           pappl_device_t *device, unsigned int page) {
   return true;
}

static bool rend_page_cb(pappl_job_t *job, pappl_pr_options_t *options,
                         pappl_device_t *device, unsigned int page) {
   return true;
}

static bool rwrite_cb(pappl_job_t *job, pappl_pr_options_t *options,
                      pappl_device_t *device, unsigned int y,
                      const unsigned char *line) {
   return true;
}

bool safe_system_load_state(pappl_system_t *system, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return false;

    char buffer[4096];
    char *new_content = malloc(1);
    new_content[0] = '\0';
    size_t total_size = 1;
    char *printer_pos;
    char *state_pos;

    while (fgets(buffer, sizeof(buffer), fp)) {
        // Suchen Sie nach dem Drucker-Tag (z.B. <printer )
        if ((printer_pos = strstr(buffer, "<Printer")) && (state_pos = strstr(buffer, "state=\""))) {
            char temp_line[4096];
            
            // Füge ein temporär gültiges String-Attribut ein
            snprintf(temp_line, sizeof(temp_line), "%s", state_pos + 10);
            strcpy(state_pos, temp_line);
        }

        total_size += strlen(buffer);
        new_content = realloc(new_content, total_size);
        strcat(new_content, buffer);
    }
    fclose(fp);

    // Temporäre Datei für den PAPPL-Parser schreiben
    char temp_filename[256];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);
    
    fp = fopen(temp_filename, "w");
    if (fp) {
        fputs(new_content, fp);
        fclose(fp);
    }
    free(new_content);

    // Mit der reparierten temporären Datei laden
    bool success = papplSystemLoadState(system, temp_filename);
    
    // Temporäre Datei löschen
    remove(temp_filename);
    
    return success;
}

static bool save_cb(pappl_system_t *system, void *data) {
   return papplSystemSaveState(system, DRIVER_STATE);
}

// Callback zur Treiber-Zuweisung
static bool driver_callback(pappl_system_t *system, const char *driver_name,
                            const char *device_uri, const char *device_id,
                            pappl_pr_driver_data_t *driver_data,
                            ipp_t **driver_attrs, void *data) {
   // 1. Speicher sichern & leeren
   memset(driver_data, 0, sizeof(pappl_pr_driver_data_t));

   // 2. Standard-Treiberdaten befüllen
   driver_data->ppm = 10;
   driver_data->format = "application/pdf";
   papplCopyString(driver_data->make_and_model, MODEL,
                   sizeof(driver_data->make_and_model));

   // Jetzt weisen wir die exakten Member aus deinem Header zu:
   driver_data->rendjob_cb = rend_job_cb;
   driver_data->rendpage_cb = rend_page_cb;
   driver_data->rstartjob_cb = rstart_job_cb;
   driver_data->rstartpage_cb = rstart_page_cb;
   driver_data->rwriteline_cb = rwrite_cb;
   driver_data->status_cb = NULL;
   driver_data->printfile_cb = my_pdf_direct_callback;

   // JETZT RICHTIG: Welche Unter-Formate/Farbräume beherrscht der Treiber
   // intern? Hier nutzen wir deine exakten Grep-Treffer (z.B. Monochrom und
   // sRGB 8-Bit)
   driver_data->raster_types =
       PAPPL_PWG_RASTER_TYPE_BLACK_1 | PAPPL_PWG_RASTER_TYPE_BLACK_8 |
       PAPPL_PWG_RASTER_TYPE_SGRAY_8 | PAPPL_PWG_RASTER_TYPE_SRGB_8;

   driver_data->color_supported =
       PAPPL_COLOR_MODE_MONOCHROME | PAPPL_COLOR_MODE_COLOR;
   driver_data->color_default = PAPPL_COLOR_MODE_MONOCHROME;

   driver_data->num_resolution = 3;

   driver_data->x_resolution[0] = 204;
   driver_data->y_resolution[0] = 196;

   driver_data->x_resolution[1] = 300;
   driver_data->y_resolution[1] = 300;

   driver_data->x_resolution[2] = 600;
   driver_data->y_resolution[2] = 600;

   driver_data->x_default = 204;
   driver_data->y_default = 196;

   driver_data->orient_default = IPP_ORIENT_NONE;

   driver_data->quality_default = IPP_QUALITY_NORMAL;

   pwg_media_t *pwg = pwgMediaForPWG("iso_a4_210x297mm");
   if (pwg) {
      driver_data->num_media = 2;
      driver_data->media[0] = "iso_a4_210x297mm";
      driver_data->media[1] = "na_letter_8.5x11in";

      // Unterstützte Schächte (Sources)
      driver_data->num_source = 1;
      driver_data->source[0] = "main";

      // Unterstützte Papier-Typen (WICHTIG: Hat gefehlt!)
      driver_data->num_type = 1;
      driver_data->type[0] = "stationery";

      // ZUERST media_default vollständig befüllen:
      papplCopyString(driver_data->media_default.size_name, "iso_a4_210x297mm",
                      sizeof(driver_data->media_default.size_name));
      driver_data->media_default.size_width = pwg->width;
      driver_data->media_default.size_length = pwg->length;
      driver_data->media_default.bottom_margin = 423;
      driver_data->media_default.left_margin = 423;
      driver_data->media_default.right_margin = 423;
      driver_data->media_default.top_margin = 423;
      papplCopyString(driver_data->media_default.source, "main",
                      sizeof(driver_data->media_default.source));
      papplCopyString(driver_data->media_default.type, "stationery",
                      sizeof(driver_data->media_default.type));

      // ERST DANACH media_ready kopieren!
      driver_data->media_ready[0] = driver_data->media_default;
   }

   driver_data->sides_supported = PAPPL_SIDES_ONE_SIDED;
   driver_data->sides_default = PAPPL_SIDES_ONE_SIDED;

   driver_data->printfile_cb = my_pdf_direct_callback;

   // Erlaubt file:// als Device-URI
   driver_data->has_supplies =
       false; // Optional, für File-Output nicht relevant

   driver_data->num_vendor = 17;

   driver_data->vendor[0] = "maxretry";
   driver_data->vendor[1] = "delay";
   driver_data->vendor[2] = "faxresolution";
   driver_data->vendor[3] = "faxrendering";
   driver_data->vendor[4] = "type";
   driver_data->vendor[5] = "sq";
   driver_data->vendor[6] = "tq";
   driver_data->vendor[7] = "gq";
   driver_data->vendor[8] = "rotate";
   driver_data->vendor[9] = "preview";
   driver_data->vendor[10] = "faxnumber";
   driver_data->vendor[11] = "faxheader";
   driver_data->vendor[12] = "sendingfaxid";
   driver_data->vendor[13] = "emailaddress";
   driver_data->vendor[14] = "endpoint";
   driver_data->vendor[15] = "authuser";
   driver_data->vendor[16] = "authpwd";

   // 3. Custom Attributes für den Treiber
   if (driver_attrs) {
      *driver_attrs = ippNew();

      // MaxRetry 4
      ippAddRange(*driver_attrs, IPP_TAG_PRINTER, "maxretry-supported", 0, 99);
      ippAddInteger(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_INTEGER,
                    "maxretry-default", 4);

      // Delay 0
      ippAddRange(*driver_attrs, IPP_TAG_PRINTER, "delay-supported", 0, 10);
      ippAddInteger(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_INTEGER,
                    "delay-default", 0);

      // FaxPageResolution (NORM | HIGH)
      static const char *const faxresolution_values[] = {"norm", "high"};
      ippAddStrings(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_KEYWORD,
                    "faxresolution-supported",
                    2,
                    NULL,
                    faxresolution_values);
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_KEYWORD,
                   "faxresolution-default", NULL, "norm");

      // FaxRendering
      static const char *const faxrendering_values[] = {
          "black-white",
          "ghostscript",
          "grey-dithered",
          "floyd-steinberg-dithered",
          "floyd-steinberg-sharpened",
          "floyd-steinberg-threshold",
          "floyd-steinberg-sharp-threshold"};
      ippAddStrings(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_KEYWORD,
                    "faxrendering-supported",
                    7,
                    NULL,
                    faxrendering_values);
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_KEYWORD,
                   "faxrendering-default", NULL,
                   "floyd-steinberg-sharp-threshold");

      // type
      static const char *const type_values[] = {"text", "image", "text+image"};
      ippAddStrings(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_KEYWORD,
                    "type-supported",
                    3,
                    NULL,
                    type_values);
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_KEYWORD,
                   "type-default",
                   NULL,
                   "text");

      // sq,tq,gq
      ippAddRange(*driver_attrs, IPP_TAG_PRINTER, "sq-supported", 10, 45);
      ippAddInteger(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_INTEGER,
                    "sq-default", 18);
      ippAddRange(*driver_attrs, IPP_TAG_PRINTER, "tq-supported", 20, 200);
      ippAddInteger(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_INTEGER,
                    "tq-default", 35);
      ippAddRange(*driver_attrs, IPP_TAG_PRINTER, "gq-supported", 10, 20);
      ippAddInteger(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_INTEGER,
                    "gq-default", 14);

      // rotate  (no | yes)
      ippAddBoolean(*driver_attrs, IPP_TAG_PRINTER, "rotate-supported", true);
      ippAddBoolean(*driver_attrs, IPP_TAG_PRINTER, "rotate-default", true);

      // Preview (no | yes)
      ippAddBoolean(*driver_attrs, IPP_TAG_PRINTER, "preview-supported", true);
      ippAddBoolean(*driver_attrs, IPP_TAG_PRINTER, "preview-default", false);

      // Definition einer Faxnummer-Option (Textfeld) u. Standardwert
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "faxnumber-supported", NULL, "any");
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "faxnumber-default", NULL, "none");

      // faxheader
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "faxheader-supported", NULL, "any");
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "faxheader-default", NULL, "none");

      // sendingfaxid
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "sendingfaxid-supported", NULL, "any");
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "sendingfaxid-default", NULL, "none");

      // emailaddress
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "emailaddress-supported", NULL, "any");
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "emailaddress-default", NULL, "none");

      // endpoint
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "endpoint-supported", NULL, "any");
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "endpoint-default", NULL, "none");

      // authuser
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "authuser-supported", NULL, "any");
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "authuser-default", NULL, "none");

      // authpwd
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "authpwd-supported", NULL, "any");
      ippAddString(*driver_attrs, IPP_TAG_PRINTER, IPP_TAG_TEXT, "authpwd-default", NULL, "none");
   }

   return true;
}

// PAPPL_DRV_TYPE_DYNAMIC für Treiber, die via Callback (driver_callback)
// konfiguriert werden
static pappl_pr_driver_t drivers[] = { {DRIVER_NAME, MODEL, DEVICE_ID, NULL} };

static bool device_cb(const char *device_info, // z.B. "HP OfficeJet Pro"
                      const char *device_uri,  // z.B. "socket://192.168.1.50"
                      const char *device_id,   // z.B. "MFG:HP;MDL:OfficeJet..."
                      void *data)              // Eigene Daten
{
   return (true);
}

pappl_system_t *                        // O - New system object
system_callback(int num_options,        // I - Number of options
                cups_option_t *options, // I - Options
                void *data)             // I - Callback data
{
   pappl_system_t *system;         // System object
   const char *val,                // Current option value
       *hostname,                  // Hostname, if any
       *logfile,                   // Log file, if any
       *system_name;               // System name, if any
   pappl_loglevel_t loglevel;      // Log level
   int port = 0;                   // Port number, if any
   pappl_soptions_t soptions =
       PAPPL_SOPTIONS_MULTI_QUEUE | PAPPL_SOPTIONS_WEB_INTERFACE |
       PAPPL_SOPTIONS_WEB_LOG | PAPPL_SOPTIONS_WEB_NETWORK |
       PAPPL_SOPTIONS_WEB_SECURITY | PAPPL_SOPTIONS_WEB_TLS; // System options
   static pappl_contact_t contact = // Contact information
       {"Jürgen Hammelmann", "j.hammelmann@gmx.de", ""};
   static pappl_version_t versions[1] = // Software versions
       {{.name = MODEL,
         .sversion = "3.0",
         .version = {3, 0, 0, 0}}};

   // Verify that the right callback data was sent to us...
   if (!data || strcmp((char *)data, DRIVER_NAME)) {
      fprintf(stderr, "%s: Bad callback data %p.\n", DRIVER_NAME, data);
      return (NULL);
   }

   // Parse options...
   if ((val = cupsGetOption("log-level", num_options, options)) != NULL) {
      if (!strcmp(val, "fatal"))
         loglevel = PAPPL_LOGLEVEL_FATAL;
      else if (!strcmp(val, "error"))
         loglevel = PAPPL_LOGLEVEL_ERROR;
      else if (!strcmp(val, "warn"))
         loglevel = PAPPL_LOGLEVEL_WARN;
      else if (!strcmp(val, "info"))
         loglevel = PAPPL_LOGLEVEL_INFO;
      else if (!strcmp(val, "debug"))
         loglevel = PAPPL_LOGLEVEL_DEBUG;
      else {
         fprintf(stderr, "%s: Bad log-level value '%s'.\n", DRIVER_NAME, val);
         return (NULL);
      }
   } else
      loglevel = PAPPL_LOGLEVEL_UNSPEC;

   logfile = cupsGetOption("log-file", num_options, options);
   hostname = cupsGetOption("server-hostname", num_options, options);
   system_name = cupsGetOption("system-name", num_options, options);

   if ((val = cupsGetOption("server-port", num_options, options)) != NULL) {
      if (!isdigit(*val & 255)) {
         fprintf(stderr, "%s: Bad server-port value '%s'.\n", DRIVER_NAME, val);
         return (NULL);
      } else
         port = atoi(val);
   }

   // Create the system object...
   if ((system = papplSystemCreate(
            soptions,
            system_name ? system_name : DRIVER_NAME,
            port,
            "_print,_universal",
            cupsGetOption("spool-directory", num_options, options),
            logfile ? logfile : "-", loglevel,
            cupsGetOption("auth-service", num_options, options),
            /* tls_only */ false)) == NULL)
      return (NULL);

   papplSystemAddListeners(system, NULL);
   papplSystemSetHostName(system, hostname);

   papplSystemSetPrinterDrivers(
       system, (int)(sizeof(drivers) / sizeof(drivers[0])), drivers,
       autoadd_callback, /*create_cb*/ NULL, driver_callback, (char *) DRIVER_NAME);

   // papplSystemSetFooterHTML(system, FOOTER_HTML);
   papplSystemSetSaveCallback(system, save_cb, NULL);
   papplSystemSetVersions(system, (int)(sizeof(versions) / sizeof(versions[0])), versions);

   if (!safe_system_load_state(system, DRIVER_STATE)) {
      papplSystemSetContact(system, &contact);
      papplSystemSetDNSSDName(system, system_name ? system_name : DRIVER_NAME);
      papplSystemSetGeoLocation(system, "geo:48.77078,9.18310");
      papplSystemSetLocation(system, "Development Unit");
      papplSystemSetOrganization(system, "GTC TeleCommunication GmbH");

      pappl_printer_t *printer =
          papplPrinterCreate(system,
                             0,           // printer_id (0 = auto)
                             PRINTER_QUEUE, // Name der Queue
                             DRIVER_NAME, // Treiber Name
                             DEVICE_ID, // Device ID
                             SOCKET     // Device URI
          );

      if (printer) {
         papplPrinterSetDNSSDName(printer, PRINTER_QUEUE);
         papplPrinterSetPrintGroup(printer, "lp");
         papplPrinterSetContact(printer, &contact);

         // Standard-Optionen definieren
         int num_options = 0;
         cups_option_t *options = NULL;

         num_options = cupsAddOption("maxretry", "4", num_options, &options);
         num_options = cupsAddOption("delay", "0", num_options, &options);
         num_options = cupsAddOption("faxresolution", "norm", num_options, &options);
         num_options = cupsAddOption("faxrendering", "floyd-steinberg-sharp-threshold", num_options, &options);
         num_options = cupsAddOption("type", "text", num_options, &options);
         num_options = cupsAddOption("sq", "18", num_options, &options);
         num_options = cupsAddOption("tq", "35", num_options, &options);
         num_options = cupsAddOption("gq", "14", num_options, &options);
         num_options = cupsAddOption("rotate", "1", num_options, &options); // "1" statt "yes" für IPP Boolean
         num_options = cupsAddOption("preview", "0", num_options, &options); // "0" statt "no" für IPP Boolean
         num_options = cupsAddOption("faxnumber", "none", num_options, &options);
         num_options = cupsAddOption("faxheader", "none", num_options, &options);
         num_options = cupsAddOption("sendingfaxid", "none", num_options, &options);
         num_options = cupsAddOption("emailaddress", "none", num_options, &options);
         num_options = cupsAddOption("endpoint", "none", num_options, &options);
         num_options = cupsAddOption("authuser", "none", num_options, &options);
         num_options = cupsAddOption("authpwd", "none", num_options, &options);

         // WICHTIG: Hier holen wir die von PAPPL generierten driver_data ab,
         // um die Defaults darauf anzuwenden!
         pappl_pr_driver_data_t driver_data;
         papplPrinterGetDriverData(printer, &driver_data);

         // Optionen an die Queue binden
         papplPrinterSetDriverDefaults(printer, &driver_data, num_options,
                                       options);

         // Speicher der erzeugten Optionen wieder sauber freigeben
         cupsFreeOptions(num_options, options);
      }

      papplSystemSaveState(system, DRIVER_STATE);
   }

   papplDeviceList(PAPPL_DEVTYPE_ALL, device_cb, NULL, NULL, NULL);

   return system;
}

// 3. Das Hauptprogramm
int main(int argc, char *argv[]) {
   return papplMainloop(argc, argv,
                        "1.4", // Version deiner App
                        NULL,  // Footer-Text für das Web-UI
                        sizeof(drivers) / sizeof(drivers[0]), // Anzahl der Treiber
                        drivers,                              // Treiber-Liste
                        autoadd_callback,   // Autoadd-Callback (optional)
                        driver_callback,    // Treiber-Callback
                        NULL,               // Custom Sub-Command Name
                        NULL,               // Custom Sub-Command Callback
                        system_callback,    // System-Callback
                        NULL,               // Usage-Callback
                        (void *)DRIVER_NAME // Benutzerdaten
   );
}
