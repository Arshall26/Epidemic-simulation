#include "graphic_interface.h"
#include "epidemic_sim.h"

void* run_gui(void *args)
{
   CDKSCREEN *cdkscreen;
   CDKHISTOGRAM *healthy;
   CDKHISTOGRAM *sick;
   CDKHISTOGRAM *dead;
   CDKHISTOGRAM *burned;
   CDKLABEL *day;
   CDKLABEL *news;
   CDKLABEL *map;
   CDKLABEL *mapContaminationLevel;
   CDKLABEL *mapCitizens;
   char *mapLayout[MAP_HEADER_SIZE + MAP_HEIGHT];
   char *contaminationLayout[MAP_HEADER_SIZE + MAP_HEIGHT];
   char *citizensLayout[MAP_HEADER_SIZE + MAP_HEIGHT];
   char *dayLabelMsg[1];
   char *newsLabelMsg[4];
   City *city;
   int healthy_count, sick_count, dead_count, burned_count;
   int i;

   city = (City*)args;

   cdkscreen = initCDKScreen(NULL);

   dayLabelMsg[0] = (char*) malloc(8 * sizeof(char));
   sprintf(dayLabelMsg[0], "%s %d", "DAY", 1);

   for (i = 0; i < 4; i++) {
      newsLabelMsg[i] = (char*) malloc(NEWS_BUFFER * sizeof(char));
      strcpy(newsLabelMsg[i], "                                                    "); /* Otherwise it won't show longer text we add latter */
   }

   /* Start CDK Color. */
   initCDKColor();

   healthy = createHistogram(cdkscreen, HEALTHY_H, 1, QUARTER_WIDTH);
   sick = createHistogram(cdkscreen, SICK_H, 1, QUARTER_WIDTH);
   dead = createHistogram(cdkscreen, DEAD_H, 1, QUARTER_WIDTH);
   burned = createHistogram(cdkscreen, BURNED_H, 1, QUARTER_WIDTH);

   day = newCDKLabel(cdkscreen,
                     BOTTOM_RIGHT_QUARTER_X + HALF(QUARTER_WIDTH) - 1,
                     BOTTOM_RIGHT_QUARTER_Y + HALF(HALF(HALF(QUARTER_HEIGHT))),
                     dayLabelMsg,
                     1,
                     TRUE,
                     FALSE);

   news = newCDKLabel(cdkscreen,
                     BOTTOM_RIGHT_QUARTER_X + HALF(HALF(QUARTER_WIDTH)) - 10,
                     BOTTOM_RIGHT_QUARTER_Y + HALF(HALF(QUARTER_HEIGHT)) + 5,
                     newsLabelMsg,
                     4,
                     TRUE,
                     FALSE);

   map = createMap(city, cdkscreen, MAP, mapLayout,
                  TOP_LEFT_QUARTER_X + 1,
                  TOP_LEFT_QUARTER_Y + 1,
                  3,
                  "<C></B>MAP",
                  "<C></3> <!3> Land </5> <!5> Hospital </2> <!2> Firehouse </4> <!4> House");
   mapContaminationLevel = createMap(city, cdkscreen, MAP_CONTAMINATION, contaminationLayout,
                                    (QUARTER_WIDTH - MAP_WIDTH * MAP_SQUARE_WIDTH) + 1,
                                    1,
                                    3,
                                    "<C></B>CONTAMINATION LEVEL",
                                    "<C></3> <!3> 0-25% </5> <!5> 25-50% </4> <!4> 50-75% </2> <!2> >75%");
   mapCitizens = createMap(city, cdkscreen, MAP_CITIZENS, citizensLayout,
                           (QUARTER_WIDTH + MAP_WIDTH * MAP_SQUARE_WIDTH) + 1, 
                           TOP_RIGHT_QUARTER_Y + 1, 
                           3, 
                           "<C></B>CITIZENS DISTRIBUTION", 
                           "<C></5> <!5> 0 </3> <!3> 1-3 </4> <!4> 4-7 </2> <!2> >8");

   while(run) {
      if(city->gui_flag == 0) {
         healthy_count = 0;
         sick_count = 0;
         dead_count = 0;
         burned_count = 0;
         sprintf(dayLabelMsg[0], "%s %d", "DAY", city->numberRounds);
         updateLabelMessage(day, dayLabelMsg);

         for (i = 0; i < 4; i++) {
            strcpy(newsLabelMsg[i], city->news[i]);
         }
         setCDKLabelMessage(news, newsLabelMsg, 4);


         for (i = 0; i < 37; i++) {
            switch (city->citizens[i].status) {
               case SICK: sick_count++;
                           break;
               case DEAD: dead_count++;
                           break;
               case CALCINATED: burned_count++;
                           break;
               default: healthy_count++;
            }
         }

         updateHistogramValues(healthy, healthy_count, HEALTHY_H);
         updateHistogramValues(sick, sick_count, SICK_H);
         updateHistogramValues(dead, dead_count, DEAD_H);
         updateHistogramValues(burned, burned_count, BURNED_H);

         updateContaminationMap(mapContaminationLevel, city, contaminationLayout, 3);
         updateCitizensMap(mapCitizens, city, citizensLayout, 3);

         refreshCDKScreen(cdkscreen);
         city->gui_flag = 1;
      }
      usleep(100000); /* 100 ms to avoid CPU overload because of while condition check */
   }

   /* Clean up. */
   destroyCDKHistogram(healthy);
   destroyCDKHistogram(sick);
   destroyCDKHistogram(dead);
   destroyCDKHistogram(burned);
   destroyCDKLabel(day);
   destroyCDKLabel(news);
   destroyCDKLabel(map);
   destroyCDKLabel(mapContaminationLevel);
   destroyCDKLabel(mapCitizens);
   destroyCDKScreen(cdkscreen);
   endCDK();
   free(dayLabelMsg[0]);

   for (i = 0; i < 4; i++) {
      free(newsLabelMsg[i]);
   }

   return NULL;
}

void updateMap(CDKLABEL *label, City *city, char *mapLayout[], int squareWidth) {
   int i;
   int j;
   int k;

   for (i = MAP_HEADER_SIZE; i < MAP_HEADER_SIZE + MAP_HEIGHT; i++) {
      char* colorCode;
      char* colorCodeClose;
      if (label == NULL) {
         mapLayout[i] = (char*) malloc((MAP_WIDTH * (8 + 1 + squareWidth) + 3) * sizeof(char)); /* +8 for </x><!x> (CDK color formatting code), +1 for |, +3 for <C> (CDK formatting code) */
      } else {
         strcpy(mapLayout[i], "");
      }
      strcpy(mapLayout[i], "<C>");
      
      for (j = 0; j < MAP_WIDTH; j++) {
         colorCode = malloc(4 * sizeof(char));
         colorCodeClose = malloc(4 * sizeof(char));
         switch(city->map[i - MAP_HEADER_SIZE][j].type) {
            case HOUSE:
               strcpy(colorCode, "</4>");
               strcpy(colorCodeClose, "<!4>");
               break;
            case HOSPITAL:
               strcpy(colorCode, "</5>");
               strcpy(colorCodeClose, "<!5>");
               break;
            case FIREHOUSE:
               strcpy(colorCode, "</2>");
               strcpy(colorCodeClose, "<!2>");
               break;
            default: /* LAND */
               strcpy(colorCode, "</3>");
               strcpy(colorCodeClose, "<!3>");
         }
         strcat(mapLayout[i], colorCode);
         for (k = 0; k < squareWidth; k++) {
            strcat(mapLayout[i], "_");
         }
         strcat(mapLayout[i], colorCodeClose);
         strcat(mapLayout[i], "|");
      }
   }
   if (label != NULL) {
      setCDKLabelMessage(label, mapLayout, MAP_HEADER_SIZE + MAP_HEIGHT);
   }
}

void updateContaminationMap(CDKLABEL *label, City *city, char *mapLayout[], int squareWidth) {
   int i;
   int j;
   int k;
   double contaminationLevel;

   for (i = MAP_HEADER_SIZE; i < MAP_HEADER_SIZE + MAP_HEIGHT; i++) {
      char* colorCode;
      char* colorCodeClose;
      if (label == NULL) {
         mapLayout[i] = (char*) malloc((MAP_WIDTH * (8 + 1 + squareWidth) + 3) * sizeof(char)); /* +8 for </x><!x> (CDK color formatting code), +1 for |, +3 for <C> (CDK formatting code) */
      } else {
         strcpy(mapLayout[i], "");
      }
      strcpy(mapLayout[i], "<C>");
      
      for (j = 0; j < MAP_WIDTH; j++) {
         colorCode = malloc(4 * sizeof(char));
         colorCodeClose = malloc(4 * sizeof(char));

         contaminationLevel = city->map[i - MAP_HEADER_SIZE][j].contaminationLevel;
         if (contaminationLevel >= 0.75) {
            strcpy(colorCode, "</2>");
            strcpy(colorCodeClose, "<!2>");
         } else if (contaminationLevel < 0.75 && contaminationLevel >= 0.50) {
            strcpy(colorCode, "</4>");
            strcpy(colorCodeClose, "<!4>");
         } else if (contaminationLevel < 0.50 && contaminationLevel >= 0.25) {
            strcpy(colorCode, "</5>");
            strcpy(colorCodeClose, "<!5>");
         } else {
            strcpy(colorCode, "</3>");
            strcpy(colorCodeClose, "<!3>");
         }
         strcat(mapLayout[i], colorCode);
         for (k = 0; k < squareWidth; k++) {
            strcat(mapLayout[i], "_");
         }
         strcat(mapLayout[i], colorCodeClose);
         strcat(mapLayout[i], "|");
      }
   }
   if (label != NULL) {
      setCDKLabelMessage(label, mapLayout, MAP_HEADER_SIZE + MAP_HEIGHT);
   }
}

void updateCitizensMap(CDKLABEL *label, City *city, char *mapLayout[], int squareWidth) {
   int i;
   int j;
   int k;
   double citizensCount;

   for (i = MAP_HEADER_SIZE; i < MAP_HEADER_SIZE + MAP_HEIGHT; i++) {
      char* colorCode;
      char* colorCodeClose;
      if (label == NULL) {
         mapLayout[i] = (char*) malloc((MAP_WIDTH * (8 + 1 + squareWidth) + 3) * sizeof(char)); /* +8 for </x><!x> (CDK color formatting code), +1 for |, +3 for <C> (CDK formatting code) */
      } else {
         strcpy(mapLayout[i], "");
      }
      strcpy(mapLayout[i], "<C>");
      
      for (j = 0; j < MAP_WIDTH; j++) {
         colorCode = malloc(4 * sizeof(char));
         colorCodeClose = malloc(4 * sizeof(char));

         citizensCount = city->map[i - MAP_HEADER_SIZE][j].populationCount;
         if (citizensCount >= 8) {
            strcpy(colorCode, "</2>");
            strcpy(colorCodeClose, "<!2>");
         } else if (citizensCount < 8 && citizensCount >= 4) {
            strcpy(colorCode, "</4>");
            strcpy(colorCodeClose, "<!4>");
         } else if (citizensCount < 4 && citizensCount >= 1) {
            strcpy(colorCode, "</3>");
            strcpy(colorCodeClose, "<!3>");
         } else {
            strcpy(colorCode, "</5>");
            strcpy(colorCodeClose, "<!5>");
         }
         strcat(mapLayout[i], colorCode);
         for (k = 0; k < squareWidth; k++) {
            strcat(mapLayout[i], "_");
         }
         strcat(mapLayout[i], colorCodeClose);
         strcat(mapLayout[i], "|");
      }
   }
   if (label != NULL) {
      setCDKLabelMessage(label, mapLayout, MAP_HEADER_SIZE + MAP_HEIGHT);
   }
}

CDKLABEL* createMap(City *city, CDKSCREEN *cdkscreen, int type, char *mapLayout[], int posX, int posY, int squareWidth, char* title, char* legends) {
   mapLayout[0] = title;
   mapLayout[1] = BLANK_LINE;
   mapLayout[2] = legends;
   mapLayout[3] = BLANK_LINE;
   switch(type) {
      case MAP_CONTAMINATION:
         updateContaminationMap(NULL, city, mapLayout, squareWidth);
         break;
      default:
         updateMap(NULL, city, mapLayout, squareWidth);
   }
   return newCDKLabel(cdkscreen, posX, posY, mapLayout, MAP_HEADER_SIZE + MAP_HEIGHT, TRUE, FALSE);
}

CDKHISTOGRAM* createHistogram(CDKSCREEN *cdkscreen, enum histogramType hType, int height, int width) {
   CDKHISTOGRAM *histogram;

   switch(hType) {
      case HEALTHY_H:
         histogram = newCDKHistogram(cdkscreen, BOTTOM_LEFT_QUARTER_X, BOTTOM_LEFT_QUARTER_Y, height, width, HORIZONTAL, "<C></3>Healthy<!3>", TRUE, FALSE);
         break;
      case SICK_H:
         histogram = newCDKHistogram(cdkscreen, BOTTOM_LEFT_QUARTER_X, BOTTOM_LEFT_QUARTER_Y + 4, height, width, HORIZONTAL, "<C></4>Sick<!4>", TRUE, FALSE);
         break;
      case DEAD_H:
         histogram = newCDKHistogram(cdkscreen, BOTTOM_LEFT_QUARTER_X, BOTTOM_LEFT_QUARTER_Y + 8, height, width, HORIZONTAL, "<C></2>Dead<!2>", TRUE, FALSE);
         break;
      case BURNED_H:
         histogram = newCDKHistogram(cdkscreen, BOTTOM_LEFT_QUARTER_X, BOTTOM_LEFT_QUARTER_Y + 12, height, width, HORIZONTAL, "<C></6>Burned<!6>", TRUE, FALSE);
         break;
      default:
         NULL;
   }

   return histogram;
}

void updateHistogramValues(CDKHISTOGRAM* histogram, int newValue, enum histogramType hType) {
   switch(hType) {
      case HEALTHY_H:
         if (newValue >= CITIZEN_COUNT / 2) {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(3), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(3), TRUE);
         } else {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(8), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(3), TRUE);
         }
         break;
      case SICK_H:
         if (newValue >= CITIZEN_COUNT / 2) {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(4), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(4), TRUE);
         } else {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(8), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(4), TRUE);
         }
         break;
      case DEAD_H:
         if (newValue >= CITIZEN_COUNT / 2) {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(2), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(2), TRUE);
         } else {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(8), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(2), TRUE);
         }
         break;
      case BURNED_H:
         if (newValue >= CITIZEN_COUNT / 2) {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(6), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(6), TRUE);
         } else {
            setCDKHistogram(histogram, vREAL, CENTER, A_BOLD|COLOR_PAIR(8), 0, CITIZEN_COUNT, newValue, ' '|A_BOLD|COLOR_PAIR(6), TRUE);
         }
         break;
      default:
         return;
   }
}

void updateLabelMessage(CDKLABEL* label, char** message) {
   setCDKLabelMessage(label, message, 1);
}