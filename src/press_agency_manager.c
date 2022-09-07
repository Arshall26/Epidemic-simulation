#include "press_agency_manager.h"

mqd_t news;

void press_agency_manager() {
    int shared_memory;
    pthread_t tid;
    City* city;

    news = mq_open("/pressrelease", O_RDONLY | O_NONBLOCK);

    shared_memory = shm_open("/simulator", O_RDWR, 0666);
    if (shared_memory == -1) {
        printf("Error using shm_open\n");
        exit(EXIT_FAILURE);
    }
    city = (City *)mmap(NULL, sizeof(City) ,PROT_WRITE|PROT_READ, MAP_SHARED, shared_memory, 0);
    
    pthread_create(&tid, NULL, press_agency_thread, city);
}

void *press_agency_thread(void *args) {
	City *city = (City *)args;
    sem_t *sem;
    struct mq_attr attr;
    char *buffer;
    char newsBuffer[NEWS_BUFFER];
    int n;
    unsigned int priority;

    sem = sem_open("/sem-simulation", O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);
    mq_getattr(news, &attr);
    buffer = malloc(attr.mq_msgsize);

	while(city->numberRounds < ROUNDS && run) {
	    if(city->press_flag == 0) {
            sem_wait(sem);
            while ((n = mq_receive(news, buffer, attr.mq_msgsize, &priority)) > 0) {
                buffer[n] = '\0';
                switch (priority) {
                    case 10:
                        strcpy(newsBuffer, "BREAKING NEWS: ");
                        strcat(newsBuffer, buffer);
                        strcat(newsBuffer, " people died from the epidemic");
                        strcpy(city->news[0], newsBuffer);
                        break;
                    case 5:
                        strcpy(newsBuffer, "BREAKING: ");
                        
                        strcat(newsBuffer, "contamination level has raised to ");
                        strcat(newsBuffer, buffer);
                        strcat(newsBuffer, "% !");
                        strcpy(city->news[1], newsBuffer);
                        break;
                    case 2:
                        strcpy(newsBuffer, "NEWS: ");
                        strcat(newsBuffer, buffer);
                        strcat(newsBuffer, " people are now contaminated");
                        strcpy(city->news[2], newsBuffer);
                        break;
                    default:
                        if (atoi(buffer) > 80) {
                            strcpy(newsBuffer, "NEWS: ");
                            strcat(newsBuffer, buffer);
                            strcat(newsBuffer, "%, contamination level of one of our reporter");
                            strcpy(city->news[3], newsBuffer);
                        }
                        break;
                }
            }
            city->press_flag = 1;
            sem_post(sem);
        }
        usleep(100000); /* 100 ms to avoid CPU overload because of while condition check */
    }
    free(buffer);
    return NULL;
}