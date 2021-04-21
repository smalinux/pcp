/*
 * Copyright (c) 2013-2018 Red Hat.
 * Copyright (c) 1995-2001,2003 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <dirent.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include "pmapi.h"
#include "libpcp.h"
#include "sha1.h"

static void myeventdump(pmValueSet *, int, int);
static int  myoverrides(int, pmOptions *);

static pmLongOptions longopts[] = {
    PMAPI_OPTIONS_HEADER("General options"),
    PMOPT_ARCHIVE,
    PMOPT_DEBUG,
    PMOPT_HOST,
    PMOPT_CONTAINER,
    PMOPT_LOCALPMDA,
    PMOPT_SPECLOCAL,
    PMOPT_NAMESPACE,
    PMOPT_UNIQNAMES,
    PMOPT_ORIGIN,
    PMOPT_TIMEZONE,
    PMOPT_HOSTZONE,
    PMOPT_VERSION,
    PMOPT_HELP,
    PMAPI_OPTIONS_HEADER("Protocol options"),
    { "batch",    1, 'b', "N", "fetch N metrics at a time for -f and -v [128]" },
    { "desc",     0, 'd', 0, "get and print metric description" },
    { "fetch",    0, 'f', 0, "fetch and print values for all instances" },
    { "fetchall", 0, 'F', 0, "fetch and print values for non-enumerable indoms" },
    { "fullindom",0, 'I', 0, "print InDom in verbose format" },
    { "labels",   0, 'l', 0, "print InDom, metric and instance labels" },
    { "pmid",     0, 'm', 0, "print PMID" },
    { "fullpmid", 0, 'M', 0, "print PMID in verbose format" },
    { "series",   0, 's', 0, "print source, metric, instance series identifiers" },
    { "oneline",  0, 't', 0, "get and display (terse) oneline text" },
    { "helptext", 0, 'T', 0, "get and display (verbose) help text" },
    PMAPI_OPTIONS_HEADER("Metrics options"),
    { "derived",  1, 'c', "FILE", "load derived metric definitions from FILE(s)" },
    { "events",   0, 'x', 0, "unpack and report on any fetched event records" },
    { "verify",   0, 'v', 0, "verify mode, be quiet and only report errors" },
    PMAPI_OPTIONS_END
};

static pmOptions opts = {
    .flags = PM_OPTFLAG_STDOUT_TZ,
    .short_options = "a:b:c:CdD:Ffh:IK:lLMmN:n:O:stTvVxzZ:?",
    .long_options = longopts,
    .short_usage = "[options] [metricname | pmid | indom]...",
    .override = myoverrides,
};

static int	p_mid;		/* Print metric IDs of leaf nodes */
static int	p_fullmid;	/* Print verbose metric IDs of leaf nodes */
static int	p_fulliid;	/* Print verbose indom IDs in descriptions */
static int	p_desc;		/* Print descriptions for metrics */
static int	p_label;	/* Print labels for InDoms,, metrics indoms and instances */
static int	p_series;	/* Print metrics series identifiers */
static int	p_oneline;	/* fetch oneline text? */
static int	p_help;		/* fetch help text? */
static int	p_value;	/* pmFetch and print value(s)? */
static int	p_force;	/* pmFetch and print value(s)? for non-enumerable indoms */

static int	need_context;	/* set if need a pmapi context */
static int	need_pmid;	/* set if need to lookup names */
static char	**namelist;
static pmID	*pmidlist;
static int	contextid;
static int	batchsize = 128;
static int	batchidx;
static int	verify;		/* Only print error messages */ // SMA: delete me
static pmID	pmid_flags;
static pmID	pmid_missed;
char** mymetrics;
static int totalmetrics = 0; // FIXME get this value from parser

/*
 * we only ever have one metric
 */
static void
mydump(pmDesc *dp, pmValueSet *vsp, char *indent)
{
    int		j;
    char	*p;

    // SMA: START: useless
    if (indent != NULL)
	printf("%s", indent);
    // SMA: END: useless
    if (vsp->numval == 0) {
	printf("No value(s) available!\n");
	return;
    }
    else if (vsp->numval < 0) {
	printf("Error: %s\n", pmErrStr(vsp->numval));
	return;
    }


	pmValue	*vp = &vsp->vlist[j];

	pmPrintValue(stdout, vsp->valfmt, dp->type, vp, 1);

}

static void
report(void)
{
    int		i;
    int		sts;
    pmDesc	desc;
    pmResult	*result = NULL;
    pmResult	*xresult = NULL;
    pmValueSet	*vsp = NULL;
    int		all_count;
    int		*all_inst;
    char	**all_names;

    // SMA: START: useless
    if (batchidx == 0)
    // SMA: END: useless

    /* Lookup names.
     * Cull out names that were unsuccessfully looked up.
     * However, it is unlikely to fail because names come from a traverse PMNS.
     */
     printf("\n=============batchidx= %d\n", batchidx);

    if (need_pmid) {
        if ((sts = pmLookupName(batchidx, (const char **)namelist, pmidlist)) < 0) {
	    int j = 0;
	    for (i = 0; i < batchidx; i++) {
		if (pmidlist[i] == PM_ID_NULL) {
		    printf("%s: pmLookupName: %s\n", namelist[i], pmErrStr(sts));
		    free(namelist[i]);
		}
		else {
		    /* assert(j <= i); */
		    pmidlist[j] = pmidlist[i];
		    namelist[j] = namelist[i];
		    j++;
		}
	    }
	    batchidx = j;
	}
    }

    if (p_value || p_label || verify) {
	if (opts.context == PM_CONTEXT_ARCHIVE) {
	    if ((sts = pmSetMode(PM_MODE_FORW, &opts.origin, 0)) < 0) {
		fprintf(stderr, "%s: pmSetMode failed: %s\n", pmGetProgname(), pmErrStr(sts));
		exit(1);
	    }
	}
	if ((sts = pmFetch(batchidx, pmidlist, &result)) < 0) {
	    for (i = 0; i < batchidx; i++)
		printf("%s: pmFetch: %s\n", namelist[i], pmErrStr(sts));
	    goto done;
	}
    }

    for (i = 0; i < batchidx; i++) {

	if (p_desc || p_value || p_label || p_series || verify) {
	    if ((sts = pmLookupDesc(pmidlist[i], &desc)) < 0) {
		printf("%s: pmLookupDesc: %s\n", namelist[i], pmErrStr(sts));
		continue;
	    }
	}

   // SMA: useless if statment
	if (p_desc || p_help || p_value || p_label)
	    /* Not doing verify, output separator  */
	    putchar('\n');


	if (p_value || verify) {
      printf("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");
	    vsp = result->vset[i];
	    if (p_force) {
		if (result->vset[i]->numval == PM_ERR_PROFILE) {
		    /* indom is non-enumerable; try harder */
		    if ((all_count = pmGetInDom(desc.indom, &all_inst, &all_names)) > 0) {
			pmDelProfile(desc.indom, 0, NULL);
			pmAddProfile(desc.indom, all_count, all_inst);
			if (xresult != NULL) {
			    pmFreeResult(xresult);
			    xresult = NULL;
			}
			if (opts.context == PM_CONTEXT_ARCHIVE) {
			    if ((sts = pmSetMode(PM_MODE_FORW, &opts.origin, 0)) < 0) {
				fprintf(stderr, "%s: pmSetMode failed: %s\n", pmGetProgname(), pmErrStr(sts));
				exit(1);
			    }
			}
			if ((sts = pmFetch(1, &pmidlist[i], &xresult)) < 0) {
			    printf("%s: pmFetch: %s\n", namelist[i], pmErrStr(sts));
			    continue;
			}
			vsp = xresult->vset[0];
			/* leave the profile in the default state */
			free(all_inst);
			free(all_names);
			pmDelProfile(desc.indom, 0, NULL);
			pmAddProfile(desc.indom, 0, NULL);
		    }
		    else if (all_count == 0) {
			printf("%s: pmGetIndom: No instances?\n", namelist[i]);
			continue;
		    }
		    else {
			printf("%s: pmGetIndom: %s\n", namelist[i], pmErrStr(all_count));
			continue;
		    }
		}
	    }
	}

	if (verify) {
	    if (desc.type == PM_TYPE_NOSUPPORT)
		printf("%s: Not Supported\n", namelist[i]);
	    else if (vsp->numval < 0)
		printf("%s: %s\n", namelist[i], pmErrStr(vsp->numval));
	    else if (vsp->numval == 0)
		printf("%s: No value(s) available\n", namelist[i]);
	    continue;
	}

	/* not verify mode - detailed reporting */
	printf("%s:> ", namelist[i]);
	if (p_value)
	    mydump(&desc, vsp, NULL);
    }

    if (result != NULL) {
	pmFreeResult(result);
	result = NULL;
    }
    if (xresult != NULL) {
	pmFreeResult(xresult);
	xresult = NULL;
    }


	putchar('\n'); // SMA: end of line before exit the program.

done:
    for (i = 0; i < batchidx; i++)
	free(namelist[i]);
    batchidx = 0;
}

static void
dometric(const char *name)
{
    if (*name == '\0') {
	printf("PMNS appears to be empty!\n");
	return;
    }

    namelist[batchidx]= strdup(name);
    //printf("%s\n", name);
    if (namelist[batchidx] == NULL) {
	fprintf(stderr, "%s: namelist string malloc: %s\n", pmGetProgname(), osstrerror());
	exit(1);
    }

     //printf("\n=============batchidx= %d\n", batchidx);
    batchidx++;
    if (batchidx >= batchsize)
	report();
}

/*
 * pminfo has a few options which do not follow the defacto standards
 */
static int
myoverrides(int opt, pmOptions *opts)
{
    if (opt == 's' || opt == 't' || opt == 'T')
	return 1;	/* we've claimed these, inform pmGetOptions */
    return 0;
}

// abort wrapper
void fail() {
   abort();

   _exit(1); // Should never reach here
}

// strdup wrapper
char* xStrdup(const char* str) {
   char* data = strdup(str);
   if (!data) {
      printf("Strdup fail");
      fail(); // FIXME: this line from htop APIs
   }
   return data;
}

int xSnprintf(char* buf, size_t len, const char* fmt, ...) {
   va_list vl;
   va_start(vl, fmt);
   int n = vsnprintf(buf, len, fmt, vl);
   va_end(vl);

   if (n < 0 || (size_t)n >= len) {
      fail();
   }

   return n;
}

int parser()
{
   DIR *dir = opendir("/home/smalinux/pcplab/src/mypminfo/plugins");
   FILE *input = fopen("plugins/f1", "r");
   struct dirent *dirent;

   if (dir == NULL)  // opendir returns NULL if couldn't open directory
   {
      printf("Could not open current directory" );
      return 0;
   }

   if (input == NULL) {
      printf("Cannot open file\n");
      return 0;
   }

   if(NULL == (dir = opendir("plugins/"))) {
      fprintf(stderr, "Error : Failed %s\n", strerror(errno));
      return 1;
   }

   char buffer[1024];

   while ((dirent = readdir(dir)) != NULL) {
      /* On linux/Unix we don't want current and parent directories
      * On windows machine too, thanks Greg Hewgill
      */
     if (!strcmp (dirent->d_name, "."))
         continue;
     if (!strcmp (dirent->d_name, ".."))
         continue;

      char statePath[256];
      xSnprintf(statePath, sizeof(statePath), "plugins/%s", dirent->d_name);
      input = fopen(statePath, "r");
      if (input == NULL) {
         printf("%s\n", dirent->d_name);
         fprintf(stderr, "Err: failed to *open file:  %s\n", strerror(errno));

         return 1;
      }

      printf("====== %s\n", dirent->d_name);

      while (fgets(buffer, sizeof(buffer), input)) {
         char Metric[100];
         mymetrics = realloc(mymetrics, 5*sizeof(char *)); // FIXME there are an API gives you num of lines?!
         mymetrics[totalmetrics] = (char *)malloc(100);

         if(1 != sscanf(buffer,"metric = %100s", Metric))
            continue;

         mymetrics[totalmetrics] = xStrdup(Metric);
         printf("Metric = %s\n", Metric);
         printf("mymetrics = %s\n", mymetrics[totalmetrics]);
         totalmetrics++;
      }
      fclose(input);
   }
   return 0;
}

int
main(int argc, char **argv)
{
    int		a;
    int		sts;
    int		exitsts = 0;
    char	*source;


   parser();

   // Start: Parser




   // END: Parser


   // argv == 'f'
   p_value = 1;
   need_context = 1;
   need_pmid = 1;

   // START: useless validation
    if (opts.errors || (opts.flags & PM_OPTFLAG_EXIT)) {
      exitsts = !(opts.flags & PM_OPTFLAG_EXIT);
      pmUsageMessage(&opts);
      exit(exitsts);
    }
   // END: useless validation

    if (opts.context)
	need_context = 1;

    // START: useless
    if (opts.context == PM_CONTEXT_ARCHIVE)
	/*
	 * for archives, one metric per batch and start at beginning of
	 * archive for each batch so metric will be found if it is in
	 * the archive
	 */
	batchsize = 1;
    // END: useless

    if (verify)      // SMA: verify == 0
      p_value = 0;
    //printf("####### verify: %d\n", verify);


    if ((namelist = (char **)malloc(batchsize * sizeof(char *))) == NULL) {
	fprintf(stderr, "%s: namelist malloc: %s\n", pmGetProgname(), osstrerror());
	exit(1);
    }

    if ((pmidlist = (pmID *)malloc(batchsize * sizeof(pmID))) == NULL) {
	fprintf(stderr, "%s: pmidlist malloc: %s\n", pmGetProgname(), osstrerror());
	exit(1);
    }

    // START: useless
    if (!opts.nsflag) // SMA:    !opts.nsflag == 1
	need_context = 1; /* for distributed PMNS as no PMNS file given */
    //printf("####### nsflag: %d\n", !opts.nsflag);
    // END: useless

    // SMA: Okay,, but ...
    if (need_context) {
      printf("*****************************************\n");
	if (opts.context == PM_CONTEXT_ARCHIVE)
	    source = opts.archives[0];
	else if (opts.context == PM_CONTEXT_HOST)
	    source = opts.hosts[0];
	else if (opts.context == PM_CONTEXT_LOCAL)
	    source = NULL;
	else {
	    opts.context = PM_CONTEXT_HOST;
	    source = "local:";
	}
	if ((sts = pmNewContext(opts.context, source)) < 0) {
	    if (opts.context == PM_CONTEXT_HOST)
		fprintf(stderr, "%s: Cannot connect to PMCD on host \"%s\": %s\n",
			pmGetProgname(), source, pmErrStr(sts));
	    else if (opts.context == PM_CONTEXT_LOCAL)
		fprintf(stderr, "%s: Cannot make standalone connection on localhost: %s\n",
			pmGetProgname(), pmErrStr(sts));
	    else
		fprintf(stderr, "%s: Cannot open archive \"%s\": %s\n",
			pmGetProgname(), source, pmErrStr(sts));
	    exit(1);
	}
	contextid = sts;

   // SMA: useless
	if (opts.context == PM_CONTEXT_ARCHIVE) {
      printf("*****************************************\n");
	    if (opts.nsflag) {
		/*
		 * loaded -n (or -N) namespace from the command line,
		 * so cull metrics not in the archive
		 */
		pmTrimNameSpace();
	    }
	    /* complete TZ and time window option (origin) setup */
	    if (pmGetContextOptions(contextid, &opts)) {
		pmflush();
		exit(1);
	    }
	}
    }


    if (opts.optind >= argc) { // SMA: useless. i will use argc ..
       printf("\npmTraversePMNS\n"); // just $pminfo | head
      printf("77777777777777777777777777777777777777777777777777\n");
    	sts = pmTraversePMNS("", dometric);
	if (sts < 0) {
	    fprintf(stderr, "Error: %s\n", pmErrStr(sts));
		exitsts = 1;
	}
    }
    else { // SMA: <3
      for (a = opts.optind; a < totalmetrics; a++) { // FIXME: get num of metrics dynamically
         printf("argv[%d] = %s\n", a, mymetrics[a]);
         sts = pmTraversePMNS(mymetrics[a], dometric);
         // SMA: validation
         if (sts < 0) {
            fprintf(stderr, "Error: %s: %s\n", argv[a], pmErrStr(sts));
            exitsts = 1;
         }
      }
    }

   report();

   exit(exitsts);
}
