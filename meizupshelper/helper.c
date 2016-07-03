#include <stdio.h>
#include <string.h>

#include <meizupshelper.h>


static int do_calibrate(void)
{
	return meizu_psh_run_calibration();
}


static int do_usage(const char *progname)
{
	fprintf(stderr, "usage: %s <c>\n\n", progname);
	fprintf(stderr, "       c -- run proximity sensor calibration\n");
	fprintf(stderr, "       r -- trigger proximity sensor reset\n");
	fprintf(stderr, "            param1: delay between enabling and disabling, in ms\n");

	return 127;
}


int main(int argc, char *argv[])
{
	if (argc < 2) {
		return do_usage(argv[0]);
	}

	if (!strcasecmp(argv[1], "c") && argc == 2) {
		return do_calibrate();
	}

	if (!strcasecmp(argv[1], "r") && argc == 3) {
		int msecs = atoi(argv[2]);
		meizu_psh_trigger_reset(msecs);
		return 0;
	}

	return do_usage(argv[0]);
}
