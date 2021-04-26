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

static void PluginMeter_updateValues(Meter* this) {
   fprintf(stderr, "From PluginMeter_updateValues ********************************\n");
}


const MeterClass PluginMeter_class = {
   .super = {
      .extends = Class(Meter),
      .delete = Meter_delete,
      //.display = PluginMeter_display
   },
   .updateValues = PluginMeter_updateValues,
   .defaultMode = BAR_METERMODE,
   //.maxItems = Plugin_METER_ITEMCOUNT,
   //.total = 100.0,
   //.attributes = PluginMeter_attributes,
   .name = "Plg",
   .uiName = "Plg",
   .caption = "Plg",
   //.init = PluginMeter_init
};

