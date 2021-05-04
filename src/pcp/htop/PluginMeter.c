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
   int cpu = this->param;
   for(int i = 0; i < 2; i++) {
      char caption[100];
      xSnprintf(caption, sizeof(caption), "%s: ", mymetrics[cpu]);
      //fprintf(stderr, "From PluginMeter_updateValues ********************************\n");
      Meter_setCaption(this, caption);
   }
   //fprintf(stderr, "From PluginMeter_updateValues qqqqqqqqqqqqqqqqqqqqqqqq\n");
}

static void PluginMeter_updateValues(Meter* this) {
   //fprintf(stderr, "From PluginMeter_updateValues ********************************\n");

}

static void PluginMeter_display(ATTR_UNUSED const Object* cast, RichString* out) {
   char buffer[50];
   pmAtomValue value;

   const Meter* this = (const Meter*)cast;
   //xSnprintf(buffer, sizeof(buffer), "%s ", "Value");
   //RichString_writeAscii(out, CRT_colors[METER_VALUE_NOTICE] , buffer);
   mydump(this->param + 115);

   Metric_values(117, &value, 1, pcp->descs[117].type);
   fprintf(stderr, "pcp->descs[117].type %d\n", pcp->descs[117].type);
   xSnprintf(buffer, sizeof(buffer), "%s ", value.cp);
   RichString_writeAscii(out, CRT_colors[METER_VALUE_NOTICE] , buffer);
   //fprintf(stderr, "this->param %d\n", this->param);

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
   .uiName = "PCPPlgin",
   .caption = "Plgkk",
   .init = PluginMeter_init,
};

