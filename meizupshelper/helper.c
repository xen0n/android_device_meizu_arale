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

	return 127;
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		return do_usage(argv[0]);
	}

	if (!strcasecmp(argv[1], "c")) {
		return do_calibrate();
	}

	return do_usage(argv[0]);
}
