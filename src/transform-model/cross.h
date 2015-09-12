#ifndef TRANSFORM_MODEL_CROSS_H
#define TRANSFORM_MODEL_CROSS_H

#define PROGRAM_INSTRUCTION_SIZE 3
const int PROGRAM_SIZE = 128 * PROGRAM_INSTRUCTION_SIZE;
const int POPULATION_SIZE = 128;

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include "transform_model.h"
#include "instruction_set.h"

void mutate(transform_model* transform, const float mutation_ratio){

    size_t i;
    for (i = 0; (i + (PROGRAM_INSTRUCTION_SIZE - 1)) < transform->program_size; i += PROGRAM_INSTRUCTION_SIZE){
        if ((rand() / RAND_MAX) <= mutation_ratio){

            assert(PROGRAM_INSTRUCTION_SIZE == 3);
            transform->program[i + 0] = rand() & 255;
            transform->program[i + 1] = rand() & 255;
            transform->program[i + 2] = rand() & 255;
        }
    }
}


transform_model* random_transform(){
    transform_model* transform = new_model();

    transform->program = malloc(sizeof(char) * PROGRAM_SIZE + 1);
    transform->program_size = PROGRAM_SIZE;

    size_t i;
    for (i = 0; i < PROGRAM_SIZE; i++){
        transform->program[i] = rand() & 255;
    }
    transform->program[transform->program_size] = '\0';

    return transform;
}


void cross(transform_model* population[], int iteration, const char* text){
    // Mutations of the first half, the worst, the more mutations
    int index;
    for (index = 1; index < POPULATION_SIZE; index++){
        mutate(population[index], POPULATION_SIZE / index);
    }

    // Then cross the following
    for (; index < POPULATION_SIZE; index++){

        free_transform_model(population[index]);

        int i, j;

        do {
            i = rand() % POPULATION_SIZE;
        } while (i == index);

        do {
            j = rand() % POPULATION_SIZE;
        } while ((j == index) || (j == i));

        population[index] = copy_model(population[i]);
        assert(population[index] != NULL);

        char* from = population[j]->program;
        char* to = population[index]->program;

        // The cross alternates a symbol of each individual
        int pos;
        for (pos = 0;
             (from[pos] != '\0') && (from[pos + 1] != '\0')
                 && (to[pos] != '\0') && (to[pos + 1] != '\0');
             pos += 2){

            to[pos] = from[pos];

        }
    }
}

#endif
