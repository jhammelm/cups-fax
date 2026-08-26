#include <libgen.h>

#include "webservice/soapH.h"           // include the generated declarations
#include "webservice/PortBinding.nsmap" // include the generated namespace table

#include "config.h"
#include "stub.h"
#include "read-file.h"
#include "logging.h"

char*
strrem(char* filename)
{
   size_t l = strlen(filename);
   char* buf = calloc(l + 1, sizeof (char));
   if (buf == NULL)
      return NULL;
   size_t c = 0;
   size_t k = 0;
   for (size_t i = 0; i < l; ++i)
   {
      if (filename[i] == '.')
         c = i;
   }
   for (size_t i = 0; i < l && i < c && i < 60; ++i)
   {
      if ((filename[i] >= 'A' && filename[i] <= 'Z') || (filename[i] >= 'a' && filename[i] <= 'z') || (filename[i] >= '0' && filename[i] <= '9'))
      {
         buf[k++] = filename[i];
      }
   }
   for (size_t i = c; i < l; ++i)
   {
      if ((filename[i] >= 'A' && filename[i] <= 'Z') || (filename[i] >= 'a' && filename[i] <= 'z') || (filename[i] >= '0' && filename[i] <= '9') || filename[i] == '.')
      {
         buf[k++] = filename[i];
      }
   }
   buf[k] = '\0';
   return buf;
}

char*
strdig(char* number)
{
   size_t l = strlen(number);
   char* buf = calloc(l + 1, sizeof (char));
   if (buf == NULL)
      return NULL;
   size_t k = 0;
   for (size_t i = 0; i < l && k < 32; ++i)
   {
      if (number[i] >= '0' && number[i] <= '9')
      {
         buf[k++] = number[i];
      }
   }
   buf[k] = '\0';
   return buf;
}

int
stub(char* endpoint,
     char* authuser,
     char* authpwd,
     char* faxnumber,
     char* faxheader,
     char* sendingfaxid,
     char* emailaddress,
     char* pdfFilename,
     bool* highResolution,
     char* faxRendering,
     unsigned int* delay,
     unsigned int* maxretry,
     bool* preview,
     bool* rotate,
     unsigned int* sq,
     unsigned int* tq,
     unsigned int* gq,
     char* errortext)
{
   errno = 0;
   int ec = -1;
   int size = 0;
   char* pdfBinary = read_file(pdfFilename, &size);
   char* buf0 = NULL;
   char* buf1 = NULL;

   if (pdfBinary != NULL)
   {
      struct soap* soap = soap_new();

      int i = 3;
      struct ns1__Entries* request = soap_new_ns1__Entries(soap, i);

      request->__sizeitem = i;
      request->item = (struct ns1__Entry*) soap_new_ns1__Entry(soap, request->__sizeitem);

      i = -1;

      request->item[++i].key = "command";
      request->item[i].value = "login";

      request->item[++i].key = "authuser";
      request->item[i].value = authuser;

      request->item[++i].key = "authpwd";
      request->item[i].value = authpwd;

      struct __ns1__execResponse* response = NULL;
      if (ec == -1)
      {
         response = soap_new___ns1__execResponse(soap, 1);
      }

      char* et = "";
      strncpy(errortext, et, 256);
      if (response != NULL)
      {
         // call stub...
         if (soap_call___ns1__exec(soap, endpoint, "urn:ns1#exec", request, response) == SOAP_OK)
         {
            for (int i = 0; i < response->ns1__entries->__sizeitem; ++i)
            {
               if (strcmp(response->ns1__entries->item[i].key, "errorcode") == 0)
               {
                  ec = *(response->ns1__entries->item[i].intValue);
               }
               else if (strcmp(response->ns1__entries->item[i].key, "errortext") == 0)
               {
                  et = response->ns1__entries->item[i].value;
                  strncpy(errortext, et, 256);
               }
               else if (strcmp(response->ns1__entries->item[i].key, "password") == 0)
               {
                  authpwd = response->ns1__entries->item[i].value;
                  size_t l = strlen(authpwd) + 1;
                  buf1 = calloc(l, sizeof (char));
                  strncpy(buf1, authpwd, l);
                  authpwd = buf1;
                  buf1 = NULL;
               }
            }
            if (ec == 0)
            {
               errno = 0;
               char buf2[256];
               sprintf(buf2, "call_stub: got response for command 'login': errorcode=%d", ec);
               log_event(CPDEBUG, buf2);
               strncpy(errortext, buf2, 256);
            }
            else
            {
               errno = soap->errnum;
               char buf2[256];
               sprintf(buf2, "call_stub: got response: errorcode=%d: errortext=%s: command 'login' failed", ec, et);
               log_event(CPERROR, buf2);
               strncpy(errortext, buf2, 256);
               authpwd = NULL;
            }
         }
         else
         {
            errno = soap->errnum;
            char buf2[256];
            char buf3[256];
            sprintf(buf3, "call_stub: call failed: %s", soap_sprint_fault(soap, buf2, 256));
            log_event(CPERROR, buf3);
            strncpy(errortext, buf3, 256);
            authpwd = NULL;
         }
      }

      if (ec == 0 && authpwd != NULL)
      {
         soap_destroy(soap); // delete managed objects
         soap_end(soap); // delete managed data and temporaries
         soap_free(soap); // finalize and delete the context
         soap = soap_new();

         ec = -1;
         i = 11;
         request = soap_new_ns1__Entries(soap, i);

         request->__sizeitem = i;
         request->item = (struct ns1__Entry*) soap_new_ns1__Entry(soap, request->__sizeitem);

         i = -1;

         request->item[++i].key = "command";
         request->item[i].value = "newjob";

         request->item[++i].key = "authuser";
         request->item[i].value = authuser;

         request->item[++i].key = "authpwd";
         request->item[i].value = authpwd;

         request->item[++i].key = "authmode";
         request->item[i].value = "pw";

         if (preview != NULL)
         {
            request->item[++i].key = "preview";
            request->item[i].value = (*preview ? "yes" : "no");
         }
         if (faxnumber != NULL)
         {
            buf0 = strdig(faxnumber);
            if (buf0 != NULL && strlen(buf0) > 0)
            {
               request->item[++i].key = "FaxNumber";
               request->item[i].values = soap_new_ns1__ArrayOfString(soap, 1);
               request->item[i].values->item = soap_malloc(soap, 1 * sizeof (ns1__string));
               request->item[i].values->item[0] = (char*) buf0;
               request->item[i].values->__sizeitem = 1;
            }
            else if (preview == NULL || !(*preview))
            {
               ec = 100;
               strcpy(errortext, "no faxnumber set");
               log_event(CPERROR, errortext);
            }
            else
            {
               request->item[++i].key = "FaxNumber";
               request->item[i].values = soap_new_ns1__ArrayOfString(soap, 1);
               request->item[i].values->item = soap_malloc(soap, 1 * sizeof (ns1__string));
               request->item[i].values->item[0] = (char*) "";
               request->item[i].values->__sizeitem = 1;
            }
         }
         else
         {
            ec = 100;
            strcpy(errortext, "no faxnumber set");
            log_event(CPERROR, errortext);
         }
         if (pdfFilename != NULL)
         {
            buf1 = strrem(basename(pdfFilename));
            if (buf1 != NULL && strlen(buf1) > 0)
            {
               request->item[++i].key = "FaxPage";
               request->item[i].files = soap_new_ns1__File(soap, 1);
               request->item[i].files[0].name = buf1;
               request->item[i].files[0].data.__ptr = (unsigned char*) soap_malloc(soap, size);
               memcpy(request->item[i].files[0].data.__ptr, pdfBinary, size);
               request->item[i].files[0].data.__size = size;
               request->item[i].__sizefiles = 1;
            }
            else
            {
               ec = 101;
               strcpy(errortext, "no filename set");
               log_event(CPERROR, errortext);
            }
         }
         else
         {
            ec = 101;
            strcpy(errortext, "no filename set");
            log_event(CPERROR, errortext);
         }
         if (faxheader != NULL)
         {
            request->item[++i].key = "FaxPageHeader";
            request->item[i].value = faxheader;
         }
         if (sendingfaxid != NULL)
         {
            request->item[++i].key = "SendingFaxID";
            request->item[i].value = sendingfaxid;
         }
         if (emailaddress != NULL)
         {
            request->item[++i].key = "CustomerEMail";
            request->item[i].value = emailaddress;
         }
         if (highResolution != NULL)
         {
            request->item[++i].key = "FaxPageResolution";
            if (*highResolution)
               request->item[i].value = "HIGH";
            else
               request->item[i].value = "NORM";
         }
         if (faxRendering != NULL)
         {
            request->item[++i].key = "FaxRendering";
            request->item[i].value = faxRendering;
         }
         if (delay != NULL)
         {
            request->item[++i].key = "Delay";
            request->item[i].intValue = soap_new_int(soap, 1);
            *(request->item[i].intValue) = *delay;
         }
         if (maxretry != NULL)
         {
            request->item[++i].key = "MaxRetry";
            request->item[i].intValue = soap_new_int(soap, 1);
            *(request->item[i].intValue) = *maxretry;
         }

         response = NULL;
         if (ec == -1)
         {
            response = soap_new___ns1__execResponse(soap, 1);
         }

         et = "";
         strncpy(errortext, et, 256);
         if (response != NULL)
         {
            // call stub...
            if (soap_call___ns1__exec(soap, endpoint, "urn:ns1#exec", request, response) == SOAP_OK)
            {
               for (int i = 0; i < response->ns1__entries->__sizeitem; ++i)
               {
                  if (strcmp(response->ns1__entries->item[i].key, "errorcode") == 0)
                  {
                     ec = *(response->ns1__entries->item[i].intValue);
                  }
                  else if (strcmp(response->ns1__entries->item[i].key, "errortext") == 0)
                  {
                     et = response->ns1__entries->item[i].value;
                     strncpy(errortext, et, 256);
                  }
               }
               if (ec == 0)
               {
                  errno = 0;
                  char buf2[256];
                  sprintf(buf2, "call_stub: got response: errorcode=%d: sent FAX file %s of size=%d", ec, buf1, size);
                  log_event(CPDEBUG, buf2);
                  strncpy(errortext, buf2, 256);
               }
               else
               {
                  errno = soap->errnum;
                  char buf2[256];
                  sprintf(buf2, "call_stub: got response: errorcode=%d: errortext=%s: sending of FAX file %s of size=%d failed", ec, et, buf1, size);
                  log_event(CPERROR, buf2);
                  strncpy(errortext, buf2, 256);
               }
            }
            else
            {
               errno = soap->errnum;
               char buf2[256];
               char buf3[256];
               sprintf(buf3, "call_stub: call failed: %s", soap_sprint_fault(soap, buf2, 256));
               log_event(CPERROR, buf3);
               strncpy(errortext, buf3, 256);
            }
         }
      }
      if (buf0 != NULL)
         free(buf0);
      if (buf1 != NULL)
         free(buf1);
      if (authpwd != NULL)
         free(authpwd);
      soap_destroy(soap); // delete managed objects
      soap_end(soap); // delete managed data and temporaries
      soap_free(soap); // finalize and delete the context
      if (pdfBinary != NULL)
         free(pdfBinary);
   }
   return ec;
}

int
call_stub(struct ConfigData* cfg, char* pdffile, char* errortext)
{
   errno = 0;
   int ec = -1;
   if (pdffile != NULL)
   {
      bool* highResolution = malloc(sizeof (bool));
      *(highResolution) = strlen(Conf_FaxResolution(cfg)) > 0 ? strcasecmp(Conf_FaxResolution(cfg), "high") == 0 : false;

      unsigned int* delay = malloc(sizeof (unsigned int));
      *(delay) = Conf_FaxDelay(cfg) > 0 ? Conf_FaxDelay(cfg) : 0;

      unsigned int* maxretry = malloc(sizeof (unsigned int));
      *(maxretry) = Conf_FaxMaxRetry(cfg) > 0 ? Conf_FaxMaxRetry(cfg) : 0;

      bool* preview = malloc(sizeof (bool));
      *(preview) = strlen(Conf_Preview(cfg)) > 0 ? strcasecmp(Conf_Preview(cfg), "yes") == 0 : false;

      ec = stub(strlen(Conf_Endpoint(cfg)) > 0 ? Conf_Endpoint(cfg) : ENDPOINT,
                strlen(Conf_AuthUser(cfg)) > 0 ? Conf_AuthUser(cfg) : "0",
                strlen(Conf_AuthPwd(cfg)) > 0 ? Conf_AuthPwd(cfg) : "",
                strlen(Conf_FaxNumber(cfg)) > 0 ? Conf_FaxNumber(cfg) : "",
                strlen(Conf_FaxHeader(cfg)) > 0 ? Conf_FaxHeader(cfg) : "",
                strlen(Conf_SendingFaxID(cfg)) > 0 ? Conf_SendingFaxID(cfg) : "",
                strlen(Conf_EMailAddress(cfg)) > 0 ? Conf_EMailAddress(cfg) : "",
                pdffile,
                highResolution,
                strlen(Conf_FaxRendering(cfg)) > 0 ? Conf_FaxRendering(cfg) : "",
                delay,
                maxretry,
                preview,
                NULL,
                NULL,
                NULL,
                NULL,
                errortext);
      if (ec == 0)
      {
#ifndef CPTEST
         if (unlink(pdffile))
            log_event(CPERROR, "failed to unlink PDF file: %s", pdffile);
         else
            log_event(CPDEBUG, "unlinked PDF file: %s", pdffile);
#endif
      }

      free(preview);
      free(maxretry);
      free(delay);
      free(highResolution);
   }
   return ec;
}
