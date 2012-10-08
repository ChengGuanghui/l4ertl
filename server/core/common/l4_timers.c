#include <config.h>
#include <hwtimers.h>

extern hwtimer_t l4_timer;
extern system_clock_t l4_timer_clock;

#define L4_HWTIMERS  1

hwtimer_t *hwtimers[L4_HWTIMERS] = {
  &l4_timer
};

system_clock_t *monotonic_clock = &l4_timer_clock;
system_clock_t *realtime_clock = &l4_timer_clock;

//---------------//
// getnrhwtimers //
//---------------//

int getnrhwtimers (void) {
  return L4_HWTIMERS;
}

//------------------//
// get_best_hwtimer //
//------------------//

hwtimer_t *get_best_hwtimer (void) {
  return &l4_timer;
}
