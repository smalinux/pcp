/*
htop - CPUMeter.c
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "config.h" // IWYU pragma: keep

#include "CPUMeter.h"

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

static void PluginMeter_init(Meter* this) {
   for(int i = 0; i < 1; i++) {
      char caption[100];
      xSnprintf(caption, sizeof(caption), "%s: ", mymetrics[i]);
      //fprintf(stderr, "From PluginMeter_updateValues ********************************\n");
      Meter_setCaption(this, caption);
   }
   fprintf(stderr, "From PluginMeter_updateValues qqqqqqqqqqqqqqqqqqqqqqqq\n");
}

static void PluginMeter_updateValues(Meter* this) {
   fprintf(stderr, "From PluginMeter_updateValues ********************************\n");

}

static void PluginMeter_display(ATTR_UNUSED const Object* cast, RichString* out) {
   char buffer[50];

   //xSnprintf(buffer, sizeof(buffer), "%s ", "Value");
   //RichString_writeAscii(out, CRT_colors[METER_VALUE_NOTICE] , buffer);

   xSnprintf(buffer, sizeof(buffer), "%d ", pcp->ncpu);
   RichString_writeAscii(out, CRT_colors[METER_VALUE_NOTICE] , buffer);

}


const MeterClass PluginMeter_class = {
   .super = {
      .extends = Class(Meter),
      .delete = Meter_delete,
      .display = PluginMeter_display
   },
   .updateValues = PluginMeter_updateValues,
   .defaultMode = TEXT_METERMODE,
   //.maxItems = Plugin_METER_ITEMCOUNT,
   //.maxItems = 0,
   //.total = 100.0,
   //.attributes = PluginMeter_attributes,
   .name = "Plgii",
   .uiName = "Plgin",
   .caption = "Plgkk",
   .init = PluginMeter_init,
};

