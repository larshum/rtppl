#include <time.h>

#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#ifdef __MACH__
#include <mach/mach_time.h>
#include <mach/mach.h>
#include <mach/clock.h>
#endif

value clock_get_time_stub() {
  CAMLparam0();
  CAMLlocal1(out);

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);

  out = caml_alloc(2, 0);
  Store_field(out, 0, Val_int(ts.tv_sec));
  Store_field(out, 1, Val_int(ts.tv_nsec));
  CAMLreturn(out);
}

// Code to support MacOS, taken from 'https://github.com/ChisholmKyle/PosixMachTiming'
#ifdef __MACH__
inline void timespec_monodiff_rml(struct timespec *ts_out, const struct timespec *ts_in) {
  ts_out->tv_sec = ts_in->tv_sec - ts_out->tv_sec;
  ts_out->tv_nsec = ts_in->tv_nsec - ts_out->tv_nsec;
  if (ts_out->tv_sec < 0) {
    ts_out->tv_sec = 0;
    ts_out->tv_nsec = 0;
  } else if (ts_out->tv_nsec < 0) {
    if (ts_out->tv_sec == 0) {
      ts_out->tv_sec = 0;
      ts_out->tv_nsec = 0;
    } else {
      ts_out->tv_sec = ts_out->tv_sec - 1;
      ts_out->tv_nsec = ts_out->tv_nsec + (long)1e9;
    }
  }
}
#endif

inline int clock_nanosleep_abstime(const struct timespec *req) {
#ifdef __MACH__
  struct timespec ts_delta;
  int retval = clock_gettime(CLOCK_MONOTONIC, &ts_delta);
  if (retval == 0) {
    timespec_monodiff_rml(&ts_delta, req);
    retval = nanosleep(&ts_delta, NULL);
  }
  return retval;
#else
  return clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, req, NULL);
#endif
}

void clock_nanosleep_stub(value t) {
  CAMLparam1(t);

  struct timespec ts;
  ts.tv_sec = Int_val(Field(t, 0));
  ts.tv_nsec = Int_val(Field(t, 1));
  clock_nanosleep_abstime(&ts);

  CAMLreturn0;
}
