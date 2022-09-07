#ifndef EPIDEMIC_SIM_H
#define EPIDEMIC_SIM_H

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define LAND 0
#define HOSPITAL 1
#define FIREHOUSE 2
#define HOUSE 3

#define CITIZEN 4
#define DOCTOR 5
#define FIREMAN 6
#define REPORTER 7

#define SICK 8
#define HEALTHY 9
#define DEAD 10
#define CALCINATED 11

#define PRINT 0
#define INTERFACE 1

#define ROUNDS 100

#define NEWS_BUFFER 100

FILE *fpEvolution;

typedef struct position {
    int x;
    int y;
} Position;

typedef struct citizen {
    int role;
    int status;
    Position position;
    double contaminationLevel;
    int sickDays;
    int numberKits;
    int number;
} Citizen;

typedef struct case_ {
    int type;
    int capacityMax;
    double contaminationLevel;
    int citizensOn[37];
    int populationCount;
    int populationMax;
    int doctorCount;
    int firemanCount;
} Case;

typedef struct city {
    Case map[7][7];
    Citizen citizens[37];
    int flag[37];
    int gui_flag;
    int press_flag;
    int numberRounds;
    char news[4][NEWS_BUFFER]; /* 5 type of news: death count, contaminated count, mean contamination level and reporter contamination level */
} City;

void exit_handler();

void increment_round();

void manage_timer();

void manage_round();

void init_city(int fd);

void manage_end_of_the_round();

int start_processes();

void init_party();

#endif
