#ifndef LANG_MODEL_MODEL_H
#define LANG_MODEL_MODEL_H

#include <stdio.h>

typedef struct language_model language_model;

language_model* build_language_model(FILE *f);
void free_language_model(language_model* model);
unsigned long language_model_score(const language_model* model, const char* text);

#endif
