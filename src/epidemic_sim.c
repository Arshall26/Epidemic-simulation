#include "epidemic_sim.h"
#include "citizen_manager.h"
#include "press_agency_manager.h"
#include "utils.h"
#include "graphic_interface.h"
#include <signal.h>

City *city;
sem_t *sem;
sig_atomic_t run = 1; /* Used to exit properly when CTRL-C occurs */

void exit_handler() {
    run = 0;
}

void increment_round() {
    city->numberRounds++;
}

void manage_timer() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &manage_round;
    sigaction(SIGALRM, &action, NULL);
    alarm(1);
    while(city->numberRounds != ROUNDS && run){}
    alarm(0);
}

void manage_round() {
    set_flags(city, 0);
    sem_post(sem);
    while(verif_flags(city, 1) != 1) {} /* Wait for Citizens to update */
    set_press_flag(city);
    while(verif_press_flag(city, 1) != 1) {} /* Wait for press agency to update */
    sem_wait(sem);
    manage_end_of_the_round();
    if (PRINT) {
        printf("%d\n", city->numberRounds);
    }
    if (INTERFACE) {
        set_gui_flag(city);
        while(verif_gui_flag(city, 1) != 1) {} /* Wait for GUI to update */
    }
    increment_round();
    alarm(1);
}

void init_city(int fd) {
    /*initialisation and write the city at the beginning of the party */
    int i,j ,numberHouses = 0, numberContaminatedLands = 0;
    city = (City *)mmap(NULL ,sizeof(City) ,PROT_WRITE ,MAP_SHARED ,fd ,0);
    memset(city, 0, sizeof(City));
    srand(time(NULL));

    for (i = 0; i < 4; i++) {
        city->news[i][0] = '\0';
    }

    city->map[0][6].type = FIREHOUSE;
    city->map[6][0].type = FIREHOUSE;
    city->map[3][3].type = HOSPITAL;
    while (numberHouses < 12) {
        i = rand()%7;
        j = rand()%7;
        if(city->map[i][j].type == LAND) {
            city->map[i][j].type = HOUSE;
            numberHouses++;
        }
    }
    while (numberContaminatedLands < 3) {
    /* 10% of the lands (~3.4 cases) are contaminated between 20% to 40% at the beginning of the simulation */
            i = rand()%7;
            j = rand()%7;
            if(city->map[i][j].type == LAND) {
                city->map[i][j].contaminationLevel = ((rand()%20) + 20)/100.00;
                numberContaminatedLands++;
            }
        }
    for(i = 0; i < 4; i++) {
        city->citizens[i] = init_citizen(city, i, DOCTOR);
    }
    for(i = 4; i < 10; i++) {
        city->citizens[i] = init_citizen(city, i, FIREMAN);
    }
    for(i = 10; i < 35; i++) {
        city->citizens[i] = init_citizen(city, i, CITIZEN);
    }
    for(i = 35; i < 37; i++) {
        city->citizens[i] = init_citizen(city, i, REPORTER);
    }
    set_flags(city, 1);
}

void manage_end_of_the_round() {
    /* function which write the current state of the city in evolution.txt at the end of each round*/
    int nbOfSick = 0, nbOfHealthy = 0, nbOfDead = 0, nbOfCalcinated = 0;
    int i;
    for (i = 0; i < 37; i++) {
        switch(city->citizens[i].status) {
            case SICK:
                nbOfSick++;
                break;
            case HEALTHY:
                nbOfHealthy++;
                break;
            case DEAD:
                nbOfDead++;
                break;
            case CALCINATED:
                nbOfCalcinated++;
                break;
        }
    }
    if(PRINT) {
        printf("Round: %d\nHealthy: %d\nSick: %d\nDead: %d\nCalcinated: %d\n", city->numberRounds, nbOfHealthy, nbOfSick, nbOfDead, nbOfCalcinated);
    }
    fprintf(fpEvolution, "%d %d %d %d %d\n", city->numberRounds, nbOfHealthy, nbOfSick, nbOfDead, nbOfCalcinated);
}

int start_processes() {
    int pid, shared_memory;
    pthread_t t_gui_id;

    signal(SIGINT, exit_handler);
    signal(SIGQUIT, exit_handler);
    sem_unlink("/sem-simulation");
    sem = sem_open("/sem-simulation", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);
    sem_wait(sem);
    fpEvolution = fopen("res/evolution.txt", "w+"); /* open evolution.txt */
    pid = fork();
    if (pid < 0) {
        printf("Error using fork\n");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* father process */
        shared_memory = shm_open("/simulator", O_CREAT|O_RDWR, 0666); /* initialisation of the shared memory */
        if (shared_memory == -1) {
            printf("Error using shm_open\n");
            exit(EXIT_FAILURE);
        }
        if(ftruncate(shared_memory,sizeof(City)) == -1) { /* allocate the space for the shared memory */
            printf("Error using ftruncate\n");
            exit(EXIT_FAILURE);
        }
        init_city(shared_memory);
        citizen_manager();
        press_agency_manager();

        if (INTERFACE) {
            pthread_create(&t_gui_id, NULL, run_gui, city); /* Create GUI */
        }
        manage_timer();
        if (INTERFACE) {
            pthread_join(t_gui_id, NULL);
        }
        sem_close(sem);
        sem_unlink("/sem-simulation");
        mq_unlink("/pressrelease");
        munmap(city , sizeof(City));
        exit(EXIT_SUCCESS);
    } else {
        /* child process */
        pause();
        fclose(fpEvolution); /* close the use of evolution.txt */
        execlp("gnuplot", "gnuplot","-persist", "res/evolution.gp", (void *)0); /* create the graph at the end of the 100 rounds */
        exit(EXIT_SUCCESS);
    }
}