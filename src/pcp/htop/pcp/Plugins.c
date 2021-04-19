/*
htop - CPUMeter.c
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "config.h" // IWYU pragma: keep

#include "pcp/Plugins.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "CRT.h"
#include "Object.h"
#include "Platform.h"
#include "ProcessList.h"
#include "RichString.h"
#include "Settings.h"
#include "XUtils.h"

/* SMA:
 * - this func should be static
 * - i think this func should be in pcp/Platform.c
 * - /etc/pcp/htop/
 */
int PCPPlugin_computePluginCount(void) {

    DIR* FD = opendir("./plugins/");
    struct dirent* in_file;
    char    buffer[BUFSIZ];
    int     count = 0;


    /* Scanning the in directory */
    if (NULL == FD)
    {
        fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));

        return 1;
    }
    while ( (in_file = readdir(FD)) != NULL)
    {
        /* On linux/Unix we don't want current and parent directories
         * On windows machine too, thanks Greg Hewgill
         */
        fprintf(stderr, "%s\n", in_file->d_name);

        /* Open directory entry file for common operation */
        /* TODO : change permissions to meet your need! */
        //entry_file = fopen(in_file->d_name, "r");

        /* Doing some struf with entry_file : */
        /* For example use fgets */
        /*while (fgets(buffer, BUFSIZ, entry_file) != NULL)
        {
        } */

        /* When you finish with the file, close it */
        /*fclose(entry_file);*/
    }

    /* Don't forget to close common file before leaving */

    return 0;
}

/*
const MeterClass PCPPluginsMeter_class = {
};
*/
