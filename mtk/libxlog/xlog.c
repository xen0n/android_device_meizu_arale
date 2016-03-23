#include <cutils/log.h>

struct xlog_record {
	const char *tag_str;
	const char *fmt_str;
	int prio;
};

static void init(void) __attribute__ ((constructor));

void init(void)
{
}

int __xlog_buf_printf(int bufid, const struct xlog_record *rec, ...)
{
  va_list args;
  va_start(args, rec);
  LOG_PRI_VA(rec->prio, rec->tag_str, rec->fmt_str, args);
  va_end(args);

  return 0;
}

void dl_unregister_notify_function(void){return 0;}

void dl_register_notify_function(int (*load_notify_function) (const char *name,uintptr_t address,uintptr_t size ),int (*unload_notify_function) (const char *name, uintptr_t address)){return 0;}
