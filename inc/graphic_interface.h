#ifndef GRAPHIC_INTERFACE_H
#define GRAPHIC_INTERFACE_H

#include <cdk.h>
#include <string.h>
#include <signal.h>
#include "epidemic_sim.h"

#define CITIZEN_COUNT 37

#define MAP_WIDTH 7
#define MAP_HEIGHT 7

#define MAP_HEADER_SIZE 4
#define MAP_SQUARE_WIDTH 3

#define QUARTER_WIDTH COLS / 2
#define QUARTER_HEIGHT LINES / 2

#define TOP_LEFT_QUARTER_X 0
#define TOP_LEFT_QUARTER_Y 0
#define TOP_RIGHT_QUARTER_X COLS / 2
#define TOP_RIGHT_QUARTER_Y 0
#define BOTTOM_LEFT_QUARTER_X 0
#define BOTTOM_LEFT_QUARTER_Y LINES / 2
#define BOTTOM_RIGHT_QUARTER_X COLS / 2
#define BOTTOM_RIGHT_QUARTER_Y LINES / 2

#define PADDING 1

#define BLANK_LINE ""

#define MAP 0
#define MAP_CONTAMINATION 1
#define MAP_CITIZENS 2

extern sig_atomic_t run;

void* run_gui(void* args);

enum histogramType { HEALTHY_H, SICK_H, DEAD_H, BURNED_H };

void updateMap(CDKLABEL *label, City *city, char *mapLayout[], int squareWidth);
void updateContaminationMap(CDKLABEL *label, City *city, char *mapLayout[], int squareWidth);
void updateCitizensMap(CDKLABEL *label, City *city, char *mapLayout[], int squareWidth);
CDKLABEL* createMap(City *city, CDKSCREEN *cdkscreen, int type, char *mapLayout[], int posX, int posY, int squareWidth, char* title, char* legends);
void updateHistogramValues(CDKHISTOGRAM* histogram, int newValue, enum histogramType hType);
CDKHISTOGRAM* createHistogram(CDKSCREEN *cdkscreen, enum histogramType hType, int height, int width);

void updateLabelMessage(CDKLABEL* label, char** message);

#endif