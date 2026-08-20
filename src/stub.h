
#ifndef __stub_h__
#define __stub_h__

#include <stdbool.h>

char* strrem(char* filename);

char* strdig(char* number);

int stub(char* endpoint,
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
         char* errortext);

int call_stub(char* pdffile, char* errortext);

#endif
