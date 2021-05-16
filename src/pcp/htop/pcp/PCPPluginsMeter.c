#include "config.h" // IWYU pragma: keep

#include "PCPPluginsMeter.h"

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


static const int PluginMeter_attributes[] = {
   CPU_NICE,
   CPU_NORMAL,
   CPU_SYSTEM,
   CPU_IRQ,
   CPU_SOFTIRQ,
   CPU_STEAL,
   CPU_GUEST,
   CPU_IOWAIT
};

static void PluginMeter_init(Meter* this) {
   int index = this->param;
   for(int i = 0; i < totalplugins; i++) {
      char caption[100];
      xSnprintf(caption, sizeof(caption), "%s: ", plugins->caption[index]);
      Meter_setCaption(this, caption);
   }
}

static void PluginMeter_updateValues(Meter* this) {
   unsigned int index = this->param;
   pmAtomValue value;
   float result = 0;
   int percent = 0;
   this->curItems = 4;
   Metric_values(index+PCP_METRIC_COUNT, &value, 1, pcp->descs[index+PCP_METRIC_COUNT].type);


   switch(pcp->descs[index].type) {
      case 0:
         result = ((plugins->bar_max[index] - value.l) * 100) / value.l;
         percent = (int)result;
         break;
      case 1:
         result = ((plugins->bar_max[index] - value.ul) * 100) / value.ul;
         percent = (int)result;
         break;
      case 2:
         result = ((plugins->bar_max[index] - value.ll) * 100) / value.ll;
         percent = (int)result;
         break;
      case 3:
         result = ((plugins->bar_max[index] - value.ull) * 100) / value.ull;
         percent = (int)result;
         break;
      case 4:
         result = ((plugins->bar_max[index] - value.f) * 100) / value.f;
         percent = (int)result;
         break;
      case 5:
         result = ((plugins->bar_max[index] - value.d) * 100) / value.d;
         percent = (int)result;
         break;
      default: // err case
         this->values[0] = 25; // blue
         this->values[1] = 25; // green
         this->values[2] = 25; // red
         this->values[3] = 25; // yellow
         break;
   }


   this->values[0] = percent;
}

static void PluginMeter_display(ATTR_UNUSED const Object* cast, RichString* out) {
   const Meter* this = (const Meter*)cast;
   char buffer[200];
   pmAtomValue value;

   int index = this->param + PCP_METRIC_COUNT;
   char *color = plugins->color[this->param];
   //mydump(index);

   Metric_values(index, &value, 1, pcp->descs[index].type);

   // FIXME: cleanup! move this block to back-end & pcp/Platform.c
   switch(pcp->descs[index].type) {
      case 0:
         xSnprintf(buffer, sizeof(buffer), "%d ", value.l);
         break;
      case 1:
         xSnprintf(buffer, sizeof(buffer), "%u ", value.ul);
         break;
      case 2:
         xSnprintf(buffer, sizeof(buffer), "%lu ", value.ll);
         break;
      case 3:
         xSnprintf(buffer, sizeof(buffer), "%llu ", value.ull);
         break;
      case 4:
         xSnprintf(buffer, sizeof(buffer), "%f ", value.f);
         break;
      case 5:
         xSnprintf(buffer, sizeof(buffer), "%f ", value.d);
         break;
      case 6:
         xSnprintf(buffer, sizeof(buffer), "%s ", value.cp);
         break;
      default:
         xSnprintf(buffer, sizeof(buffer), "Err: pmDesc.type not implemented");
         break;
   }

   if(String_eq(color, "blue"))
      RichString_writeAscii(out, CRT_colors[PCP_BLUE] , buffer);
   else if(String_eq(color, "red"))
      RichString_writeAscii(out, CRT_colors[PCP_RED] , buffer);
   else if(String_eq(color, "yellow"))
      RichString_writeAscii(out, CRT_colors[PCP_YELLOW] , buffer);
   else if(String_eq(color, "green"))
      RichString_writeAscii(out, CRT_colors[PCP_GREEN] , buffer);
   else if(String_eq(color, "cyan"))
      RichString_writeAscii(out, CRT_colors[PCP_CYAN] , buffer);
   else if(String_eq(color, "shadow"))
      RichString_writeAscii(out, CRT_colors[PCP_SHADOW] , buffer);
   else
      RichString_writeAscii(out, CRT_colors[PCP_GREEN] , buffer);
}


const MeterClass PCPPluginsMeter_class = {
   .super = {
      .extends = Class(Meter),
      .delete = Meter_delete,
      .display = PluginMeter_display
   },
   .updateValues = PluginMeter_updateValues,
   .defaultMode = TEXT_METERMODE,
   .maxItems = PCP_METRIC_COUNT,
   .total = 100.0,
   .attributes = PluginMeter_attributes,
   .name = "PCPPlugin",
   .uiName = "PCPPlgin",
   .caption = "PCPPlugin",
   .init = PluginMeter_init,
};
