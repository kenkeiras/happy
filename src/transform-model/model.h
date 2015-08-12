#ifndef TRANSFORM_MODEL_MODEL_H
#define TRANSFORM_MODEL_MODEL_H

#include <stdio.h>
#include "../lang-model/model.h"

typedef struct transform_model transform_model;

transform_model* evolve_transform(const language_model* model, const char* text);
void free_transform_model(transform_model* model);
void show_transform_model(transform_model* model);

#endif
