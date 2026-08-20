#include <stdio.h>
#include <stdlib.h>

#include "read-file.h"

char*
read_file(char* filename, int* size) {
   FILE* fp;
   char* buffer;
   *size = 0;

   // open file
   fp = fopen(filename, "rb");
   if (fp == NULL) {
      perror("Fehler beim Öffnen der Datei");
      *size = 0;
      return NULL;
   }

   // check size of file
   fseek(fp, 0, SEEK_END);
   *size = ftell(fp);
   rewind(fp);

   // reservate memory
   buffer = (char *)malloc(*size + 1);
   if (buffer == NULL) {
      perror("Speicher konnte nicht reserviert werden");
      fclose(fp);
      *size = 0;
      return NULL;
   }

   // read file
   size_t s = fread(buffer, 1, *size, fp);
   if (s != *size) {
      perror("Fehler beim Lesen der Datei");
      free(buffer);
      fclose(fp);
      *size = 0;
      return NULL;
   }
   buffer[*size] = 0;

   // close file
   fclose(fp);

   return buffer;
}

