CC=gcc
CFLAGS=-I. 
DEPS=code_analyzer.h

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

pipemake: pipeline_sim.o code_analyzer.o
		$(CC) -o pipeline_sim -Wall pipeline_sim.o code_analyzer.o