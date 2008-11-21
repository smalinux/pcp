/*
 * Copyright (c) 1995-2001 Silicon Graphics, Inc.  All Rights Reserved.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <pcp/pmapi.h>
#include <pcp/impl.h>

int
main(int argc, char **argv)
{
    int		c;
    int		sts;
    char	*p;
    int		errflag = 0;
    int		type = 0;
    char	*host;
    int		mode = PM_MODE_INTERP;		/* mode for archives */
    char 	*configfile = NULL;
    char	*start = NULL;
    char	*finish = NULL;
    char	*align = NULL;
    char	*offset = NULL;
    char 	*logfile = NULL;
    pmLogLabel	label;				/* get hostname for archives */
    int		zflag = 0;			/* for -z */
    char 	*tz = NULL;			/* for -Z timezone */
    int		tzh;				/* initial timezone handle */
    char	local[MAXHOSTNAMELEN];
    char	*pmnsfile = PM_NS_DEFAULT;
#ifdef REAL_APP
    char	*control_port = NULL;
#endif
    int		samples = -1;
    struct stat	statbuf;
    char	*endnum;
    extern char	*optarg;
    extern int	optind;
    extern int	pmDebug;
    struct timeval delta = { 1, 0 };
    struct timeval startTime;
    struct timeval endTime;
    struct timeval appStart;
    struct timeval appEnd;
    struct timeval appOffset;

    /* trim cmd name of leading directory components */
    pmProgname = argv[0];
    for (p = pmProgname; *p; p++) {
	if (*p == '/')
	    pmProgname = p+1;
    }

    while ((c = getopt(argc, argv, "a:A:c:D:h:l:Ln:O:p:s:S:t:T:U:zZ:?")) != EOF) {
	switch (c) {

	case 'a':	/* archive name */
	    if (type != 0) {
#ifdef BUILD_STANDALONE
		fprintf(stderr, "%s: at most one of -a, -h and -L allowed\n", pmProgname);
#else
		fprintf(stderr, "%s: at most one of -a and -h allowed\n", pmProgname);
#endif
		errflag++;
	    }
	    type = PM_CONTEXT_ARCHIVE;
	    host = optarg;
	    break;

	case 'A':	/* time alignment */
	    align = optarg;
	    break;

	case 'c':	/* configfile */
	    if (configfile != NULL) {
		fprintf(stderr, "%s: at most one -c option allowed\n", pmProgname);
		errflag++;
	    }
	    configfile = optarg;
	    break;	

	case 'D':	/* debug flag */
	    sts = __pmParseDebug(optarg);
	    if (sts < 0) {
		fprintf(stderr, "%s: unrecognized debug flag specification (%s)\n",
		    pmProgname, optarg);
		errflag++;
	    }
	    else
		pmDebug |= sts;
	    break;

	case 'h':	/* contact PMCD on this hostname */
	    if (type != 0) {
#ifdef BUILD_STANDALONE
		fprintf(stderr, "%s: at most one of -a, -h and -L allowed\n", pmProgname);
#else
		fprintf(stderr, "%s: at most one of -a and -h allowed\n", pmProgname);
#endif
		errflag++;
	    }
	    host = optarg;
	    type = PM_CONTEXT_HOST;
	    break;

#ifdef BUILD_STANDALONE
	case 'L':	/* LOCAL, no PMCD */
	    if (type != 0) {
		fprintf(stderr, "%s: at most one of -a, -h, -L and -U allowed\n", pmProgname);
		errflag++;
	    }
	    host = NULL;
	    type = PM_CONTEXT_LOCAL;
	    putenv("PMDA_LOCAL_PROC=");		/* if proc PMDA needed */
	    putenv("PMDA_LOCAL_SAMPLE=");	/* if sampledso PMDA needed */
	    break;
#endif

	case 'l':	/* logfile */
	    logfile = optarg;
	    break;

	case 'n':	/* alternative name space file */
	    pmnsfile = optarg;
	    break;

	case 'p':	/* port for slave of existing pmtime master */
	    if ((sts = stat(optarg, &statbuf)) < 0) {
		fprintf(stderr, "%s: Error: can not access time control port \"%s\": %s\n",
		    pmProgname, optarg, pmErrStr(-errno));
		errflag++;
	    }
	    else if ((statbuf.st_mode & S_IFSOCK) != S_IFSOCK) {
		fprintf(stderr, "%s: Error: time control port \"%s\" is not a socket\n",
		    pmProgname, optarg);
		errflag++;
	    }
#ifdef REAL_APP
	    else
		/*
		 * in a real application this would be used later ... here it
		 * simply produces a variable "control_port" was set but never
		 * used compilation warning
		 */
		control_port = optarg;
#endif
	    break;

	case 'O':	/* sample offset time */
	    offset = optarg;
	    break;

	case 's':	/* sample count */
	    samples = (int)strtol(optarg, &endnum, 10);
	    if (*endnum != '\0' || samples < 0) {
		fprintf(stderr, "%s: -s requires numeric argument\n", pmProgname);
		errflag++;
	    }
	    break;

	case 'S':	/* start time */
	    start = optarg;
	    break;

	case 't':	/* change update interval */
	    if (pmParseInterval(optarg, &delta, &p) < 0) {
		fprintf(stderr, "%s: illegal -t argument\n", pmProgname);
		fputs(p, stderr);
		free(p);
		errflag++;
	    }
	    break;

	case 'T':	/* terminate time */
	    finish = optarg;
	    break;

	case 'U':	/* uninterpolated archive log */
	    if (type != 0) {
#ifdef BUILD_STANDALONE
		fprintf(stderr, "%s: at most one of -a, -h, -L and -U allowed\n", pmProgname);
#else
		fprintf(stderr, "%s: at most one of -a, -h and -U allowed\n", pmProgname);
#endif
		errflag++;
	    }
	    type = PM_CONTEXT_ARCHIVE;
	    mode = PM_MODE_FORW;
	    host = optarg;
	    break;

	case 'z':	/* timezone from host */
	    if (tz != NULL) {
		fprintf(stderr, "%s: at most one of -Z and/or -z allowed\n", pmProgname);
		errflag++;
	    }
	    zflag++;
	    break;

	case 'Z':	/* $TZ timezone */
	    if (zflag) {
		fprintf(stderr, "%s: at most one of -Z and/or -z allowed\n", pmProgname);
		errflag++;
	    }
	    tz = optarg;
	    break;

	case '?':
	default:
	    errflag++;
	    break;
	}
    }

    if (zflag && type == 0) {
	fprintf(stderr, "%s: -z requires an explicit -a, -h or -U option\n", pmProgname);
	errflag++;
    }

    if (errflag) {
	fprintf(stderr,
"Usage: %s [options] [metrics ...]\n\
\n\
Options:\n\
  -a archive     metrics source is a PCP log archive\n\
  -A align       align sample times on natural boundaries\n\
  -c configfile  file to load configuration from\n\
  -h host        metrics source is PMCD on host\n\
  -l logfile     redirect diagnostics and trace output\n"
#ifdef BUILD_STANDALONE
"  -L             use local context instead of PMCD\n"
#endif
"  -n pmnsfile    use an alternative PMNS\n\
  -O offset      initial offset into the time window\n\
  -p port        port name for connection to existing time control\n\
  -s samples     terminate after this many samples\n\
  -S starttime   start of the time window\n\
  -t interval    sample interval [default 1.0 seconds]\n\
  -T endtime     end of the time window\n\
  -z             set reporting timezone to local time of metrics source\n\
  -Z timezone    set reporting timezone\n",
                pmProgname);
        exit(1);
    }

    if (logfile != NULL) {
	__pmOpenLog(pmProgname, logfile, stderr, &sts);
	if (sts < 0) {
	    fprintf(stderr, "%s: Could not open logfile \"%s\"\n", pmProgname, logfile);
	}
    }

    if (pmnsfile != PM_NS_DEFAULT && (sts = pmLoadNameSpace(pmnsfile)) < 0) {
	printf("%s: Cannot load namespace from \"%s\": %s\n", pmProgname, 
	       pmnsfile, pmErrStr(sts));
	exit(1);
    }

    if (type == 0) {
	type = PM_CONTEXT_HOST;
	(void)gethostname(local, MAXHOSTNAMELEN);
	local[MAXHOSTNAMELEN-1] = '\0';
	host = local;
    }
    if ((sts = pmNewContext(type, host)) < 0) {
	if (type == PM_CONTEXT_HOST)
	    fprintf(stderr, "%s: Cannot connect to PMCD on host \"%s\": %s\n",
		pmProgname, host, pmErrStr(sts));
	else
	    fprintf(stderr, "%s: Cannot open archive \"%s\": %s\n",
		pmProgname, host, pmErrStr(sts));
	exit(1);
    }

    if (type == PM_CONTEXT_ARCHIVE) {
	if ((sts = pmGetArchiveLabel(&label)) < 0) {
	    fprintf(stderr, "%s: Cannot get archive label record: %s\n",
		pmProgname, pmErrStr(sts));
	    exit(1);
	}
	if (mode != PM_MODE_INTERP) {
	    if ((sts = pmSetMode(mode, &label.ll_start, 0)) < 0) {
		fprintf(stderr, "%s: pmSetMode: %s\n", pmProgname, pmErrStr(sts));
		exit(1);
	    }
	}
  	startTime = label.ll_start;
	if ((sts = pmGetArchiveEnd(&endTime)) < 0) {
	    endTime.tv_sec = INT_MAX;
	    endTime.tv_usec = 0;
	    fflush(stdout);
	    fprintf(stderr, "%s: Cannot locate end of archive: %s\n",
		pmProgname, pmErrStr(sts));
	    fprintf(stderr, "\nWARNING: This archive is sufficiently damaged that it may not be possible to\n");
	    fprintf(stderr, "         produce complete information.  Continuing and hoping for the best.\n\n");
	    fflush(stderr);
	}
    }
    else {
	gettimeofday(&startTime, NULL);
	endTime.tv_sec = INT_MAX;
    }

    if (zflag) {
	if ((tzh = pmNewContextZone()) < 0) {
	    fprintf(stderr, "%s: Cannot set context timezone: %s\n",
		pmProgname, pmErrStr(tzh));
	    exit(1);
	}
	if (type == PM_CONTEXT_ARCHIVE)
	    printf("Note: timezone set to local timezone of host \"%s\" from archive\n\n",
		label.ll_hostname);
	else
	    printf("Note: timezone set to local timezone of host \"%s\"\n\n", host);
    }
    else if (tz != NULL) {
	if ((tzh = pmNewZone(tz)) < 0) {
	    fprintf(stderr, "%s: Cannot set timezone to \"%s\": %s\n",
		pmProgname, tz, pmErrStr(tzh));
	    exit(1);
	}
	printf("Note: timezone set to \"TZ=%s\"\n\n", tz);
    }

    /* non-flag args are argv[optind] ... argv[argc-1] */
    while (optind < argc) {
	printf("extra argument[%d]: %s\n", optind, argv[optind]);
	optind++;
    }

    if (align != NULL && type != PM_CONTEXT_ARCHIVE) {
	fprintf(stderr, "%s: -A option only supported for PCP archives, alignment request ignored\n",
		pmProgname);
	align = NULL;
    }

    sts = pmParseTimeWindow(start, finish, align, offset, &startTime,
			    &endTime, &appStart, &appEnd, &appOffset,
			    &p);

    if (sts < 0) {
	fprintf(stderr, "%s: illegal time window specification\n%s", pmProgname, p);
	exit(1);
    }

    while (samples == -1 || samples-- > 0) {
	/* put real stuff here */
	break;
    }

    return 0;
}
