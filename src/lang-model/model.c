#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "model.h"

#define TWO_GRAMS_DICTIONARY_SIZE 0x10000
#define THREE_GRAMS_DICTIONARY_SIZE 0x1000000

struct two_grams {
    char first;
    char second;
};

struct language_model {
    // Two grams
    short two_grams[TWO_GRAMS_DICTIONARY_SIZE];
    short three_grams[THREE_GRAMS_DICTIONARY_SIZE];
};


language_model* build_language_model(FILE *f){
    language_model* model = malloc(sizeof(language_model));
    if (model == NULL){
        return NULL;
    }

    // Set all two-grams counts to zero
    short* two_grams = model->two_grams;
    short* three_grams = model->three_grams;

    memset(two_grams, 0, sizeof(short) * TWO_GRAMS_DICTIONARY_SIZE);
    memset(three_grams, 0, sizeof(short) * THREE_GRAMS_DICTIONARY_SIZE);



    unsigned char first = '\0';
    unsigned char second = '\0';
    unsigned char third = fgetc(f);

    while (!feof(f)){

        first = second;
        second = third;
        third = fgetc(f);

        // Build two-grams
        {
            if (((second != ' ') || (third != ' '))){

                unsigned int index = (second << 8) | third;
                assert((index >= 0) &&
                       (index < TWO_GRAMS_DICTIONARY_SIZE));

                two_grams[index]++;
            }
        }

        // Build three-grams
        {
            if ((first != '\0') && ((second != ' ') || (third != ' '))){

                unsigned int index = (first << 16) | (second << 8) | third;
                assert((index >= 0) &&
                       (index < THREE_GRAMS_DICTIONARY_SIZE));

                three_grams[index]++;
            }
        }
    }

    return model;
}


void free_language_model(language_model* model){
    free(model);
}


int language_model_score(const language_model* model,
                         const char *words){

    if ((words[0] == '\0') || (words[1] == '\0')){
        return 0;
    }

    int two_gram_score = 0;
    int three_gram_score = 0;
    int i;

    const short* two_grams = model->two_grams;
    const short* three_grams = model->three_grams;

    for (i = 0; words[i + 1] != '\0'; i++){

        unsigned char first = words[i];
        unsigned char second = words[i + 1];

        unsigned int index = (first << 8) | second;

        assert((index >= 0) &&
               (index < TWO_GRAMS_DICTIONARY_SIZE));

        two_gram_score += two_grams[index];
    }

    for (i = 0; words[i + 2] != '\0'; i++){

        unsigned char first = words[i];
        unsigned char second = words[i + 1];
        unsigned char third = words[i + 2];

        unsigned int index = (first << 16) | (second << 8) | third;

        assert((index >= 0) &&
               (index < THREE_GRAMS_DICTIONARY_SIZE));

        three_gram_score += three_grams[index];
    }

    return two_gram_score
        + three_gram_score * 9;
}
