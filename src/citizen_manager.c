#include "citizen_manager.h"

City *city;
mqd_t newsQueue;

void citizen_manager() {
    int shared_memory;
    int i;
    pthread_t tid[37];

    shared_memory = shm_open("/simulator", O_RDWR, 0666); /* initialisation of the shared memory */
    if (shared_memory == -1) {
        printf("Error using shm_open\n");
        exit(EXIT_FAILURE);
    }
    city = (City *)mmap(NULL ,sizeof(City) ,PROT_WRITE|PROT_READ ,MAP_SHARED ,shared_memory ,0);
    
    mq_unlink("/pressrelease"); /* Avoid problem if it was not closed properly */
    newsQueue = mq_open("/pressrelease", O_WRONLY | O_CREAT | O_NONBLOCK, 0644, NULL);
    for (i = 0; i < 37; i++) {
    		pthread_create(&tid[i] , NULL , citizen_thread , &(city->citizens[i]));
    }
}

void *citizen_thread(void *args) {
	Citizen *citizen = (Citizen *)args;
    sem_t *sem;
    double probabilityMove;

    sem = sem_open("/sem-simulation", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);

	while(city->numberRounds < ROUNDS && run) {
	    if(city->flag[citizen->number] == 0) {
            sem_wait(sem);
            probabilityMove = (rand()%100)/100.00;
            if(probabilityMove < 0.4) {
                if((citizen->status != DEAD) || (citizen->status != CALCINATED)) {
                    switch(citizen->role) {
                        case REPORTER:
                            reporter_round(citizen);
                            break;
                        case DOCTOR:
                            doctor_round(citizen);
                            break;
                        case FIREMAN:
                            fireman_round(citizen);
                            break;
                        default:
                            citizen_round(citizen);
                            break;
                    }
                }
            } else {
                update_citizen_contamination_level(citizen, 5);
                update_case_contamination_level(citizen);
            }
            update_citizen_status(citizen);
            contamination_spread(citizen);
            city->flag[citizen->number] = 1;
            sem_post(sem);
        }
        usleep(100000); /* 100 ms to avoid CPU overload because of while condition check */
    }
    return NULL;
}

void update_city(Position oldPosition, Citizen *citizen) {
    if((citizen->position.y < 0 || citizen->position.x < 0) || ((citizen->position.y > 6 || citizen->position.x > 6))) {
           printf("Error %d %d\n", citizen->position.x, citizen->position.y);
           exit(0);
    }
    city->map[oldPosition.x][oldPosition.y].populationCount--;
    city->map[citizen->position.x][citizen->position.y].populationCount++;
    update_case_contamination_level(citizen);
    city->map[oldPosition.x][oldPosition.y].citizensOn[citizen->number] = 0;
    city->map[citizen->position.x][citizen->position.y].citizensOn[citizen->number] = 1;

}

void update_citizen_status(Citizen *citizen) {
    double probability = (rand()%100)/100.00;
    int factor;
    switch(citizen->status) {
        case HEALTHY:
            if (probability < citizen->contaminationLevel) {
                if (PRINT) {
                    printf("Malade citoyen n°%d", citizen->number);
                    printf("--> proba : %f pourcentage de chance:%f\n", probability, citizen->contaminationLevel);
                }
                citizen->status = SICK;
                citizen->sickDays = 0;
            }
            break;
        case SICK:
            if(citizen_on_hospital(citizen)) {
                factor = 4;
            } else {
                factor = 1;
            }
            if ((probability < (0.05 * citizen->sickDays)/factor) && (city->numberRounds >= 4)) {
                if (PRINT) {
                    printf("Mort citoyen n°%d", citizen->number);
                    printf("--> proba : %f pourcentage de chance:%f\n", probability, (0.05 * citizen->sickDays)/factor);
                }
                citizen->status = DEAD;
            }
            citizen->sickDays++;
            break;
        default:
            break;
    }
}

void contamination_spread(Citizen *citizen) {
    double probability = (rand()%100)/100.00;
    int count = 0, i;
    if (probability < 0.1) {
        i = 0;
        while(count < city->map[citizen->position.x][citizen->position.y].populationCount) {
            if(city->map[citizen->position.x][citizen->position.y].citizensOn[i] == 1) {
                city->citizens[i].contaminationLevel += 0.01;
                count++;
            }
            i++;
        }
    }
    if (probability < 0.01) {
        /*TO DO spread on adjacent cases */
    }
}

void burn_dead_citizens(Position position) {
    int count = 0, i = 0;
    while(count < city->map[position.x][position.y].populationCount) {
         if(city->map[position.x][position.y].citizensOn[i] == 1) {
             if (city->citizens[i].status == DEAD) {
                if (PRINT) {
                    printf("J'ai brulé le citoyen n°%d\n", city->citizens[i].number);
                }
                city->citizens[i].status = CALCINATED;
             }
            count++;
         }
         i++;
    }
}

void heal_citizens(Position position){
    int count = 0, i = 0;
    double healPercentage;
    while(count < city->map[position.x][position.y].populationCount) {
         if(city->map[position.x][position.y].citizensOn[i] == 1) {
             healPercentage = (rand()%20)/100.00;
             if ((city->citizens[i].status == HEALTHY || city->citizens[i].status == SICK) && (city->citizens[i].contaminationLevel > 0)) {
                if (PRINT) {
                     printf("J'ai soigné le citoyen n°%d\n", city->citizens[i].number);
                     printf("--> contamination avant : %f\n", city->citizens[i].contaminationLevel);
                 }
                city->citizens[i].contaminationLevel -= city->citizens[i].contaminationLevel * healPercentage;
                if (PRINT) {
                    printf("--> contamination après : %f\n", city->citizens[i].contaminationLevel);
                }
             }
            count++;
         }
         i++;
    }
}

void update_citizen_contamination_level(Citizen *citizen, int percentage) {
    if(citizen_on_hospital(citizen)) {
        if(citizen->contaminationLevel > city->map[citizen->position.x][citizen->position.y].contaminationLevel) {
            if (PRINT) {
                printf("++++++citizen n°%d à l'hopital\n", citizen->number);
            }
            citizen->contaminationLevel -= (citizen->contaminationLevel*10)/100.00;
        }
    } else if(citizen_on_firehouse(citizen)) {
        if (PRINT) {
            printf("++++++citizen n°%d à la caserne\n", citizen->number);
        }
        citizen->contaminationLevel -= (citizen->contaminationLevel*20)/100.00;
    } else {
        citizen->contaminationLevel += (city->map[citizen->position.x][citizen->position.y].contaminationLevel*percentage)/100.00;
    }

}

void update_case_contamination_level(Citizen *citizen) {
    city->map[citizen->position.x][citizen->position.y].contaminationLevel += (citizen->contaminationLevel)/100.00;
}

Position movePosition(Position position) {
    Position pos;
    if (position.x == 0 && position.y == 0) {
        pos = top_left_move(position);
    } else if (position.x == 0 && position.y == 6) {
        pos = top_right_move(position);
    } else if (position.x == 6 && position.y == 0) {
        pos = bottom_left_move(position);
    } else if (position.x == 6 && position.y == 6) {
        pos = bottom_right_move(position);
    } else if (position.x == 0) {
        pos = top_move(position);
    } else if (position.x == 6) {
        pos = bottom_move(position);
    } else if (position.y == 0) {
        pos = left_move(position);
    } else if (position.y == 6) {
        pos = right_move(position);
    } else {
        pos = random_move(position);
    }
    return pos;
}

Position random_move(Position position) {
    int direction = rand()%8;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x + 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x;
            pos.y = position.y + 1;
            break;
        case 2:
            pos.x = position.x;
            pos.y = position.y - 1;
            break;
        case 3:
            pos.x = position.x - 1;
            pos.y = position.y;
            break;
        case 4:
             pos.x = position.x + 1;
             pos.y = position.y + 1;
             break;
        case 5:
            pos.x = position.x - 1;
            pos.y = position.y + 1;
            break;
        case 6:
            pos.x = position.x + 1;
            pos.y = position.y - 1;
            break;
        case 7:
            pos.x = position.x - 1;
            pos.y = position.y - 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position top_left_move(Position position) {
    int direction = rand()%3;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x + 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x;
            pos.y = position.y + 1;
            break;
        case 2:
            pos.x = position.x + 1;
            pos.y = position.y + 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position top_right_move(Position position) {
    int direction = rand()%3;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x + 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x;
            pos.y = position.y - 1;
            break;
        case 2:
            pos.x = position.x + 1;
            pos.y = position.y - 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position bottom_right_move(Position position) {
    int direction = rand()%3;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x - 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x;
            pos.y = position.y - 1;
            break;
        case 2:
            pos.x = position.x - 1;
            pos.y = position.y - 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position bottom_left_move(Position position) {
    int direction = rand()%3;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x - 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x;
            pos.y = position.y + 1;
            break;
        case 2:
            pos.x = position.x - 1;
            pos.y = position.y + 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position top_move(Position position) {
    int direction = rand()%5;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x + 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x;
            pos.y = position.y + 1;
            break;
        case 2:
            pos.x = position.x;
            pos.y = position.y - 1;
            break;
        case 3:
            pos.x = position.x + 1;
            pos.y = position.y + 1;
            break;
        case 4:
            pos.x = position.x + 1;
            pos.y = position.y - 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position bottom_move(Position position) {
    int direction = rand()%5;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x - 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x;
            pos.y = position.y + 1;
            break;
        case 2:
            pos.x = position.x;
            pos.y = position.y - 1;
            break;
        case 3:
            pos.x = position.x - 1;
            pos.y = position.y + 1;
            break;
        case 4:
            pos.x = position.x - 1;
            pos.y = position.y - 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position left_move(Position position) {
    int direction = rand()%5;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x + 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x - 1;
            pos.y = position.y;
            break;
        case 2:
            pos.x = position.x;
            pos.y = position.y + 1;
            break;
        case 3:
            pos.x = position.x + 1;
            pos.y = position.y + 1;
            break;
        case 4:
            pos.x = position.x - 1;
            pos.y = position.y + 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

Position right_move(Position position) {
    int direction = rand()%5;
    Position pos;
    switch(direction) {
        case 0:
            pos.x = position.x + 1;
            pos.y = position.y;
            break;
        case 1:
            pos.x = position.x - 1;
            pos.y = position.y;
            break;
        case 2:
            pos.x = position.x;
            pos.y = position.y - 1;
            break;
        case 3:
            pos.x = position.x + 1;
            pos.y = position.y - 1;
            break;
        case 4:
            pos.x = position.x - 1;
            pos.y = position.y - 1;
            break;
        default:
            pos.x = position.x;
            pos.y = position.y;
            break;
    }
    return pos;
}

void citizen_round(Citizen *citizen) {
    Position before = citizen->position;
    citizen->position = movePosition(citizen->position) ;
    if (had_move(before, citizen->position)) {
        update_citizen_contamination_level(citizen, 2);
        update_city(before, citizen);
    } else {
        update_citizen_contamination_level(citizen, 5);
        update_case_contamination_level(citizen);
    }
}

void reporter_round(Citizen *citizen) {
    char* contaminated_count;
    char* mean_contamination;
    char* dead_count;
    char* reporter_contamination_level;
    int c_count;
    double c_mean;
    int d_count;
    int i;
    int j;

    citizen_round(citizen); /* A reporter is a citizen */

    contaminated_count = (char*) malloc(50 * sizeof(char));
    mean_contamination = (char*) malloc(50 * sizeof(char));
    dead_count = (char*) malloc(50 * sizeof(char));
    reporter_contamination_level = (char*) malloc(50 * sizeof(char));

    for (i = 0, c_mean = 0.0; i < 7; i++) {
        for (j = 0; j < 7; j++) {
            c_mean += city->map[i][j].contaminationLevel;
        }
    }
    c_mean = c_mean / 49.0;

    for (i = 0, c_count = 0, d_count = 0; i < 37; i++) {
        if (city->citizens[i].status == SICK) {
            c_count++;
        } else if (city->citizens[i].status == DEAD || city->citizens[i].status == CALCINATED) {
            d_count++;
        }
    }

    c_count = c_count * 0.9;
    c_mean = c_mean * 0.9;
    d_count = d_count * 0.65;

    sprintf(contaminated_count, "%d", c_count);
    sprintf(mean_contamination, "%.2f", c_mean * 100.0);
    sprintf(dead_count, "%d", d_count);
    sprintf(reporter_contamination_level, "%.2f", citizen->contaminationLevel * 100.0);

    
    if (c_count > 0) {
        mq_send(newsQueue, contaminated_count, strlen(contaminated_count), 2);
    }
    mq_send(newsQueue, mean_contamination, strlen(mean_contamination), 5);
    if (d_count > 0) {
        mq_send(newsQueue, dead_count, strlen(dead_count), 10);
    }
    mq_send(newsQueue, reporter_contamination_level, strlen(reporter_contamination_level), 1);

    free(contaminated_count);
    free(mean_contamination);
    free(dead_count);
    free(reporter_contamination_level);
}

void fireman_round(Citizen *citizen) {
    Position before = citizen->position;

    if (fireman_can_move(citizen)) {
       citizen->position = movePosition(citizen->position) ;
    }
    burn_dead_citizens(citizen->position);
    heal_citizens(citizen->position);
    if (had_move(before, citizen->position)) {
        city->map[before.x][before.y].firemanCount--;
        city->map[citizen->position.x][citizen->position.y].firemanCount++;
        update_citizen_contamination_level(citizen, 2);
        update_city(before, citizen);
    } else {
        update_citizen_contamination_level(citizen, 5);
        update_case_contamination_level(citizen);
    }
}

int fireman_can_move(Citizen *citizen) {
    if (city->map[citizen->position.x][citizen->position.y].type == FIREHOUSE) {
        return(city->map[citizen->position.x][citizen->position.y].firemanCount > 1);
    } else {
        return 1;
    }
}

void doctor_round(Citizen *citizen) {
    Position before = citizen->position;
    if (doctor_can_move(citizen)) {
       citizen->position = movePosition(citizen->position);
    }
    if(city->map[citizen->position.x][citizen->position.y].type == HOSPITAL) {
        citizen->numberKits += 10;
    }
    if(doctor_have_to_cure_himself(citizen)) {
        if (doctor_can_cure(citizen)) {
            cure(citizen);
            if(!citizen_on_hospital(citizen)) {
                citizen->numberKits--;
            }
        }
    } else if (doctor_can_cure(citizen)){
        cure_someone(citizen);
    }
    if (had_move(before, citizen->position)) {
        city->map[before.x][before.y].doctorCount--;
        city->map[citizen->position.x][citizen->position.y].doctorCount++;
        update_citizen_contamination_level(citizen, 2);
        update_city(before, citizen);
    } else {
        update_citizen_contamination_level(citizen, 5);
        update_case_contamination_level(citizen);
    }
}

void cure_someone(Citizen *doctor)
{
    int count = 0, i = 0, mostContaminated = doctor->number;
    while(count < city->map[doctor->position.x][doctor->position.y].populationCount) {
        if(city->map[doctor->position.x][doctor->position.y].citizensOn[i] == 1) {
             if ((city->citizens[i].status == HEALTHY || city->citizens[i].status == SICK)) {
                if (city->citizens[i].contaminationLevel > city->citizens[mostContaminated].contaminationLevel) {
                    mostContaminated = i;
                }
             }
            count++;
         }
         i++;
    }
    if(mostContaminated != doctor->number) {
        cure(&city->citizens[mostContaminated]);
        if(!citizen_on_hospital(doctor)) {
            doctor->numberKits--;
        }
    }
}

void cure(Citizen *citizen)
{
    if (PRINT) {
        printf("J'ai guéri le citoyen n°%d\n", citizen->number);
    }
    citizen->status = HEALTHY;
    citizen->contaminationLevel = 0.0;
}

int doctor_can_move(Citizen *doctor)
{
    if (city->map[doctor->position.x][doctor->position.y].type == HOSPITAL) {
        return(city->map[doctor->position.x][doctor->position.y].doctorCount > 1);
    } else {
        return 1;
    }
}
int doctor_have_to_cure_himself(Citizen *doctor)
{
    return (doctor->status == SICK && doctor->sickDays < 10);
}

int doctor_can_cure(Citizen *doctor)
{
    return (city->map[doctor->position.x][doctor->position.y].type == HOSPITAL || doctor->numberKits > 0);
}

int citizen_on_hospital(Citizen *citizen)
{
    return (city->map[citizen->position.x][citizen->position.y].type == HOSPITAL);
}

int citizen_on_firehouse(Citizen *citizen)
{
    return (city->map[citizen->position.x][citizen->position.y].type == FIREHOUSE);
}

int had_move(Position before, Position current)
{
    return ((before.x != current.x) || (before.y != current.y));
}