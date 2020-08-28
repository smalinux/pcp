#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#include "zfs_dmu_tx.h"

void
zfs_dmu_tx_refresh(zfs_dmu_tx_t *dmu_tx, regex_t *rgx_row)
{
        int len_mn, len_mv, nmatch = 3;
        regmatch_t pmatch[3];
        char *line, *mname, *mval;
	char *fname = "/proc/spl/kstat/zfs/dmu_tx";
	FILE *fp;
        size_t len = 0;

        fp = fopen(fname, "r");
	if (fp != NULL) {
		while (getline(&line, &len, fp) != -1) {
                        if (regexec(rgx_row, line, nmatch, pmatch, 0) == 0) {
                                len_mn = pmatch[1].rm_eo - pmatch[1].rm_so + 1;
                                len_mv = pmatch[2].rm_eo - pmatch[2].rm_so + 1;
                                mname = (char *) malloc((size_t) (len_mn + 1) * sizeof(char));
                                mval  = (char *) malloc((size_t) (len_mv + 1) * sizeof(char));
                                strncpy(mname, line + pmatch[1].rm_so, len_mn);
                                strncpy(mval,  line + pmatch[2].rm_so, len_mv);
                                mname[len_mn] = '\0';
                                mval[len_mv] = '\0';
				if (strcmp(mname, "dmu_tx_assigned")) dmu_tx->assigned = atoi(mval);
				else if (strcmp(mname, "dmu_tx_delay")) dmu_tx->delay = atoi(mval);
				else if (strcmp(mname, "dmu_tx_error")) dmu_tx->error = atoi(mval);
				else if (strcmp(mname, "dmu_tx_suspended")) dmu_tx->suspended = atoi(mval);
				else if (strcmp(mname, "dmu_tx_group")) dmu_tx->group = atoi(mval);
				else if (strcmp(mname, "dmu_tx_memory_reserve")) dmu_tx->memory_reserve = atoi(mval);
				else if (strcmp(mname, "dmu_tx_memory_reclaim")) dmu_tx->memory_reclaim = atoi(mval);
				else if (strcmp(mname, "dmu_tx_dirty_throttle")) dmu_tx->dirty_throttle = atoi(mval);
				else if (strcmp(mname, "dmu_tx_dirty_delay")) dmu_tx->dirty_delay = atoi(mval);
				else if (strcmp(mname, "dmu_tx_dirty_over_max")) dmu_tx->dirty_over_max = atoi(mval);
				else if (strcmp(mname, "dmu_tx_dirty_frees_delay")) dmu_tx->dirty_frees_delay = atoi(mval);
				else if (strcmp(mname, "dmu_tx_quota")) dmu_tx->quota = atoi(mval);
                        }
                        free(mname);
                        free(mval);
                }
        }
        fclose(fp);
}
