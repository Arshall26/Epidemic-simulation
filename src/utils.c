#include "utils.h"
#include "epidemic_sim.h"

void printMap(City *city) {
    int i, j;
    for(i=0; i<7; i++){
        for(j=0; j<7; j++){
            printf("%d ", city->map[i][j].populationCount);
        }
        printf("\n");
    }
}
void printMapDoctorPosition(City *city) {
    int i, j;
    for(i=0; i<7; i++){
        for(j=0; j<7; j++){
            printf("%d ", city->map[i][j].doctorCount);
        }
        printf("\n");
    }
}
void printMapFiremanPosition(City *city) {
    int i, j;
    for(i=0; i<7; i++){
        for(j=0; j<7; j++){
            printf("%d ", city->map[i][j].firemanCount);
        }
        printf("\n");
    }
}

int test_population_count(City *city) {
    int i, j;
    int count = 0;
    for(i=0; i<7; i++){
        for(j=0; j<7; j++){
            count += city->map[i][j].populationCount;
        }
    }
    return count;
}


void printMapContaminationLevel(City *city) {
    int i, j;
    for(i=0; i<7; i++){
        for(j=0; j<7; j++){
            printf("%f ", city->map[i][j].contaminationLevel);
        }
        printf("\n");
    }
}

void printCitizens(City  *city) {
    int i;
    for(i=4; i<10; i++) {
        printf("role :%d\t status :%d\t cLevel :%f\t",city->citizens[i].role, city->citizens[i].status, city->citizens[i].contaminationLevel);
        printf("position :%d\t %d\n", city->citizens[i].position.x, city->citizens[i].position.y);
    }
}

void printCitizensContaminationLevel(City  *city) {
    int i;
    for(i=0; i<37; i++) {
        printf("contamination level :%f\t", city->citizens[i].contaminationLevel);
    }
}

int verif_flags(City *city, int value) {
    int i;
    for(i = 0; i < 37; i++) {
        if(city->flag[i] != value) {return 0;}
    }
    return 1;
}

void set_flags(City *city, int value){
    int i;
    for(i = 0; i < 37; i++) {
        city->flag[i] = value;
    }
}

int verif_gui_flag(City *city, int value) {
    if (city->gui_flag != value) {
        return 0;
    }
    return 1;
}

void set_gui_flag(City *city) {
    city->gui_flag = 0;
}

int verif_press_flag(City *city, int value) {
    if (city->press_flag != value) {
        return 0;
    }
    return 1;
}

void set_press_flag(City *city) {
    city->press_flag = 0;
}

Citizen init_citizen(City *city, int number, int role) {
    Citizen citizen;
    citizen.role = role;
    citizen.status = HEALTHY;
    citizen.contaminationLevel = 0.0;
    citizen.number = number;
    citizen.sickDays =0;
    switch(role) {
        case REPORTER:
            citizen.position = random_position();
            break;
        case DOCTOR:
            citizen.numberKits = 5;
            citizen.position = doctor_position(city);
            city->map[citizen.position.x][citizen.position.y].doctorCount++;
            break;
        case FIREMAN:
            citizen.position = fireman_position(city);
            city->map[citizen.position.x][citizen.position.y].firemanCount++;
            break;
        default:
            citizen.position = random_position();
            break;
    }
    city->map[citizen.position.x][citizen.position.y].populationCount++;
    city->map[citizen.position.x][citizen.position.y].citizensOn[number] = 1;
    return citizen;
}

Position random_position() {
    Position pos;
    pos.x = rand()%7;
    pos.y = rand()%7;
    return pos;
}

Position doctor_position(City *city) {
    Position pos;
    if(check_hospital(city)) {
        return random_position();
    }
    pos.x = 3;
    pos.y = 3;
    return pos;

}

Position fireman_position(City *city) {
    Position pos;
    if(check_firehouse(city)) {
        return random_position();
    }
    if(city->map[0][6].firemanCount == 0) {
        pos.x = 0;
        pos.y = 6;
        return pos;
    } else {
        pos.x = 6;
        pos.y = 0;
        return pos;
    }
}

int check_hospital(City *city) {
    return city->map[3][3].doctorCount > 0;
}

int check_firehouse(City *city) {
    return (city->map[0][6].firemanCount > 0 && city->map[6][0].firemanCount > 0);
}

void test_doctor_move(City *city) {
    if(check_hospital(city)) {
        printf("Il y a toujours un docteur à l'hopital\n");
    }
    printf("Nombre de docteurs : %d\n", test_doctor_count(city));
    printMapDoctorPosition(city);

}

int test_doctor_count(City *city) {
    int i, j;
    int count = 0;
    for(i=0; i<7; i++){
        for(j=0; j<7; j++){
            count += city->map[i][j].doctorCount;
        }
    }
    return count;
}

