#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "model.h"

#define TWO_GRAMS_DICTIONARY_SIZE 0x10000

struct two_grams {
    char first;
    char second;
};

struct language_model {
    // Two grams
    short two_grams[TWO_GRAMS_DICTIONARY_SIZE];

};


language_model* build_language_model(FILE *f){
    language_model* model = malloc(sizeof(language_model));
    if (model == NULL){
        return NULL;
    }

    // Set all two-grams counts to zero
    short* two_grams = model->two_grams;
    {
        int index;
        for (index = 0; index < TWO_GRAMS_DICTIONARY_SIZE; index++){
            two_grams[index] = 0;
        }
    }


    unsigned char first;
    unsigned char second = fgetc(f);

    while (!feof(f)){

        first = second;
        second = fgetc(f);
        if (((first == ' ') && (second == ' '))){
            continue;
        }

        unsigned int index = (first << 8) | second;
        assert((index >= 0) &&
               (index < TWO_GRAMS_DICTIONARY_SIZE));

        two_grams[index]++;
    }

    return model;
}


void free_language_model(language_model* model){
    free(model);
}


int language_model_score(const language_model* model,
                         const char *words){

    if (words[0] == '\0'){
        return 0;
    }

    int score = 0;
    int i;

    const short* two_grams = model->two_grams;

    for (i = 0; words[i + 1] != '\0'; i++){

        unsigned char first = words[i];
        unsigned char second = words[i + 1];

        unsigned int index = (first << 8) | second;

        assert((index >= 0) &&
               (index < TWO_GRAMS_DICTIONARY_SIZE));

        score += two_grams[index];
    }

    return score;
}
