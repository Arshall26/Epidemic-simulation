.PHONY : all clean

# PATH

OBJ = obj/
SRC = src/
INC = inc/
BIN = bin/

# DIRECTIVES

CC = gcc
CFLAGS = -Wall -Wextra -pedantic
CPPFLAGS = -I $(INC)

# LIBRARIES

LIBNCURSES = -lncurses
LIBCDK = -lcdk

LIBRARIES = $(LIBCDK) $(LIBNCURSES)

# TARGETS

all : $(BIN)sim

# MAIN

$(OBJ)main.o : $(SRC)main.c $(INC)epidemic_sim.h $(INC)citizen_manager.h $(INC)utils.h
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) -o $@

# GUI

$(OBJ)graphic_interface.o : $(SRC)graphic_interface.c $(INC)graphic_interface.h $(INC)epidemic_sim.h
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) -o $@

# EPIDEMIC SIM

$(BIN)sim : $(OBJ)main.o $(OBJ)epidemic_sim.o $(OBJ)citizen_manager.o $(OBJ)utils.o $(OBJ)graphic_interface.o $(OBJ)press_agency_manager.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBRARIES) -lrt -lpthread

$(OBJ)epidemic_sim.o : $(SRC)epidemic_sim.c $(INC)epidemic_sim.h $(INC)utils.h
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) -o $@

# CITIZEN MANAGER

$(OBJ)citizen_manager.o : $(SRC)citizen_manager.c $(INC)citizen_manager.h $(INC)utils.h
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) -o $@

# UTILS

$(OBJ)utils.o : $(SRC)utils.c $(INC)utils.h
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) -o $@

# PRESS AGENCY

$(OBJ)press_agency_manager.o : $(SRC)press_agency_manager.c $(INC)press_agency_manager.h $(INC)utils.h
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) -o $@

# RUN

run_sim :
	./$(BIN)sim

run_journalist :
	./$(BIN)journalist

run_press_agency :
	./$(BIN)press_agency

# CLEAN

clean :
	-rm -f $(OBJ)*
	-rm -f $(BIN)*