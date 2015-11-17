#define _GNU_SOURCE
#include <dlfcn.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

int (*orig_gettimeofday) (struct timeval *tv, struct timezone *tz);
time_t (*orig_time) (time_t *t);
int (*orig_clock_gettime) (clockid_t clk_id, struct timespec *tp);

static char fake_env_name[] = "FAKE_SECONDS_SINCE_EPOCH";
static char fake_debug_env_name[] = "FAKE_DEBUG";

void
debug_printf (char const *fmt, ...)
{
  static int fake_debug = -1;
  if (fake_debug < 0)
    {
      char const *fake_debug_env = getenv (fake_debug_env_name);
      fake_debug = (fake_debug_env && atoi (fake_debug_env));
    }
  if (fake_debug)
    {
      va_list ap;
      va_start (ap, fmt);
      vfprintf (stderr, fmt, ap);
    }
}

static time_t
get_fake_seconds ()
{
  char const *fake_time = getenv (fake_env_name);
  return (fake_time
	  ? atoi (fake_time)
	  : -1);
}

time_t
time (time_t *t)
{
  debug_printf ("fake %s\n", __FUNCTION__);
  time_t seconds = get_fake_seconds ();
  if (seconds < 0)
    return (*orig_time) (t);
  else
    {
      debug_printf ("interpose %s\n", __FUNCTION__);
      if (t)
	*t = seconds;
      return seconds;
    }
}

int
gettimeofday (struct timeval *tv, struct timezone *tz)
{
  debug_printf ("fake %s\n", __FUNCTION__);
  time_t seconds = get_fake_seconds ();
  if (seconds < 0 || !tv)
    return (*orig_gettimeofday) (tv, tz);
  else
    {
      debug_printf ("interpose %s\n", __FUNCTION__);
      int result = (*orig_gettimeofday) (tv, tz);
      if (result == 0)
	{
	  tv->tv_sec = seconds;
	  tv->tv_usec = 0;
	}
      return result;
    }
}

int
clock_gettime (clockid_t clk_id, struct timespec *tp)
{
  debug_printf ("fake %s\n", __FUNCTION__);
  time_t seconds = get_fake_seconds ();
  if (seconds < 0 || !tp || clk_id != CLOCK_REALTIME)
    return (*orig_clock_gettime) (clk_id, tp);
  else
    {
      debug_printf ("interpose %s\n", __FUNCTION__);
      tp->tv_sec = seconds;
      tp->tv_nsec = 0;
      return 0;
    }
}

void
_init(void)
{
  debug_printf ("preload %s\n", __FILE__);
  orig_gettimeofday = dlsym(RTLD_NEXT, "gettimeofday");
  orig_time = dlsym(RTLD_NEXT, "time");
  orig_clock_gettime = dlsym(RTLD_NEXT, "clock_gettime");
}
