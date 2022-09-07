#ifndef PRESS_AGENCY_H
#define PRESS_AGENCY_H

# include <fcntl.h>
# include <mqueue.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "epidemic_sim.h"
#include "utils.h"

extern sig_atomic_t run;

void press_agency_manager();
void *press_agency_thread(void *args);

#endif
