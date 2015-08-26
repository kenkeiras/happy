#ifndef TRANSFORM_MODEL_MODEL_H
#define TRANSFORM_MODEL_MODEL_H

#include <stdio.h>
#include "../lang-model/model.h"

typedef struct transform_model transform_model;

char *process(transform_model* transform,
              const char* input,
              const language_model* model);


transform_model* transform_from_program(char *program);


transform_model* evolve_transform(
    const language_model* model, const char* text,
    int (*controller) (
        int iteration, transform_model* transform,
        const char* better_output, unsigned long score));


void free_transform_model(transform_model* model);
void show_transform_model(transform_model* model);

#endif
