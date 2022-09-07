#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include "epidemic_sim.h"

void printMap(City *city);

void printMapDoctorPosition(City *city);

void printMapFiremanPosition(City *city);

void printMapContaminationLevel(City *city);

void printCitizens(City  *city);

void printCitizensContaminationLevel(City  *city);

int verif_flags(City *city, int value);

void set_flags(City *city, int value);

int verif_gui_flag(City *city, int value);

void set_gui_flag(City *city);

int verif_press_flag(City *city, int value);

void set_press_flag(City *city);

Citizen init_citizen(City *city, int number, int role);

Position random_position();

Position doctor_position(City *city);

Position fireman_position(City *city);

int check_hospital(City *city);

int check_firehouse(City *city);

int test_population_count(City *city);

void test_doctor_move(City *city);

int test_doctor_count(City *city);

#endif
