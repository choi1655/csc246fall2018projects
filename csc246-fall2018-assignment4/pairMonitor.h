#ifndef __PAIR_MONITOR_H__
#define __PAIR_MONITOR_H__

#include <stdbool.h>

/** Maximum length of a thread's name. */
#define NAME_MAX 20

/** Interface for the monitor.  It pairs up threads when they enter,
    then forces them to leave in the same pairings.  If I was
    programming in C++, this would all be wrapped up in a class.
    Since we're using C, it's just a collection of functions, with an
    initialization function to init the state of the whole monitor and
    a destroy function to free its resources. */

/** Initialize the monitor.  The capacity parameter tells how many
    pairs can be in the room at the same time. */
void initPairMonitor( int stock );

/** Destroy the monitor, freeing any resources it uses. */
void destroyPairMonitor();

/** Called by a thread when it wants to enter.  It will wait until
    there's enough capacity for it to enter and until it has a partner
    to enter with. At the end, we may have to force a thread to bail
    out of this function with no partner.  (so we can terminate the
    program).  In that case, the function returns false. */
bool enter( const char *name );

/** Called by a thread when it is ready to leave.  If it's partner has already
    called leave(), they can both leave right away.  If not, it will have to
    wait until its partner calls leave. */
void leave( const char *name );

/** Called when we're ready to terminate all the threads.  This will wake up any
    that might be stuck in enter(), waiting for capacity or for a partner. */
void terminate();

#endif
