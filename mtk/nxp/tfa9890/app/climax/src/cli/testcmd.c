#include "cmdline.h"

struct gengetopt_args_info gCmdLine; /* Global visible command line args */
extern int optind;
int main(int argc, char *argv[])
{
int i;
i=cmdline_parser (argc, argv, &gCmdLine);

printf("%d\n",optind);

printf("verbose=%d\n", gCmdLine.verbose_given);
while (argc--)
 printf("%s\n", argv[argc]);

printf("%s\n", argv[optind]);

}
