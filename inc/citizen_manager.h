#ifndef CITIZEN_MANAGER_H
#define CITIZEN_MANAGER_H

#include "epidemic_sim.h"
#include "utils.h"

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <mqueue.h>

extern sig_atomic_t run;

void citizen_manager();

void *citizen_thread(void *args);

void update_city(Position oldPosition, Citizen *citizen);

void contamination_spread(Citizen *citizen);

void burn_dead_citizens(Position position);

void heal_citizens(Position position);

void update_citizen_contamination_level(Citizen *citizen, int percentage);

void update_citizen_status(Citizen *citizen);

void update_case_contamination_level(Citizen *citizen);

Position movePosition(Position position);

Position random_move(Position position);

Position top_left_move(Position position);

Position top_right_move(Position position);

Position bottom_left_move(Position position);

Position bottom_right_move(Position position);

Position top_move(Position position);

Position bottom_move(Position position);

Position left_move(Position position);

Position right_move(Position position);

void citizen_round(Citizen *citizen);

void reporter_round(Citizen *citizen);

void fireman_round(Citizen *citizen);

int fireman_can_move(Citizen *citizen);

void doctor_round(Citizen *citizen);

void cure_someone(Citizen *citizen);

void cure(Citizen *citizen);

int doctor_can_move(Citizen *doctor);

int doctor_have_to_cure_himself(Citizen *doctor);

int doctor_can_cure(Citizen *doctor);

int citizen_on_hospital(Citizen *citizen);

int citizen_on_firehouse(Citizen *citizen);

int had_move(Position before, Position current);

#endif