#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

typedef struct plugin_ {
   char* name;
   char* metric;
   char* color;
} Plugin;

typedef struct plugins_data_ {
   Plugin data;
   struct plugins_data_* next;
} Plugins_data;

typedef struct Plugins_meterData_ {
   bool error;
   struct plugins_data_ * plugins;
} Plugins_meterData;

void fail() {
   abort();

   _exit(1); // Should never reach here
}

// FIXME: htop APIs
char* xStrdup(const char* str) {
   char* data = strdup(str);
   if (!data) {
      printf("Strdup fail");
      fail(); // FIXME: this line from htop APIs
   }
   return data;
}

static inline bool String_startsWith(const char* s, const char* match) {
   return strncmp(s, match, strlen(match)) == 0;
}

Plugins_meterData* getPluginsData(void) {
   Plugins_meterData * pdata = calloc(1, sizeof(Plugins_meterData));

   FILE *input = fopen("Plugin", "r");

   if (input == NULL) {
      printf("Cannot open file\n");
      return 0;
   }

   char buffer[1024];
   Plugins_data ** data_ref = &pdata->plugins;
   Plugins_data* ldata = calloc(1, sizeof(Plugins_data));
   Plugin* data = &ldata->data;

   while (fgets(buffer, sizeof(buffer), input)) {
      char meter[29];
      char meterName[29];
      char meterMetric[29];
      char meterColor[29];

      switch(buffer[0]) {
         case '[':
            sscanf(buffer,"[%28s]", meter);
            //printf("* %s\n", meter);
            // FIXME: trim ']' last char.
            break;
         case 'n':
            if(1 != sscanf(buffer,"name = %28s", meterName))
               continue;
            data->name = xStrdup(meterName);
            //printf("* %s\n", data->name);
            break;
         case 'm':
            if(1 != sscanf(buffer,"metric = %28s", meterMetric))
               continue;
            data->metric = xStrdup(meterMetric);
            //printf("* %s\n", data->metric);
            break;
         case 'c':
            if(1 != sscanf(buffer,"color = %28s", meterColor))
               continue;
            data->color = xStrdup(meterColor);
            //printf("* %s\n", data->color);
            break;
         default:
            break;

      } // END switch


      //printf("*1 %s\n", data->name);
      //printf("*2 %s\n", data->metric);
      //printf("*3 %s\n", data->color);
   }
   *data_ref = ldata;
   data_ref = &ldata->next;

   fclose(input);
   return pdata;
}

// ----------------------------------- main
int main(void) {
   Plugins_meterData* pdata = getPluginsData();
   printf("Nice?\n");

   if(!pdata)
      printf("No pdata!\n");
   else {
      Plugins_data* ldata = pdata->plugins;
      if(!ldata)
         printf("No ldata!\n");
      else {
         // access all items ))
         while(ldata) {
            Plugin* data = &ldata->data;
            if(!data)
               printf("No data!\n");
            else {
               printf("%s, done:%s,%s\n", data->name, data->metric, data->color);
            }
            ldata = ldata->next;
         }

      }

   }
   // --------------------- start read multi files dynamically.

   return 0;
}
