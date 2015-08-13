#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <ctype.h>
#include <math.h>

#include "model.h"
#include "../ht/hash_table.h"

#define TWO_GRAMS_DICTIONARY_SIZE 0x10000
#define THREE_GRAMS_DICTIONARY_SIZE 0x1000000
#define MAX_WORD_SIZE 256

#define TWO_GRAM_SCORE_MODIFIER 1
#define THREE_GRAM_SCORE_MODIFIER 90
#define WORD_SCORE_MODIFIER 10000

struct language_model {
    short two_grams[TWO_GRAMS_DICTIONARY_SIZE];
    short three_grams[THREE_GRAMS_DICTIONARY_SIZE];

    struct hash_table* word_count;
};


language_model* build_language_model(FILE *f){
    language_model* model = malloc(sizeof(language_model));
    if (model == NULL){
        return NULL;
    }

    model->word_count = create_hash_table();
    if (model->word_count == NULL){
        free(model);
        return NULL;
    }

    // Set all two-grams counts to zero
    short* two_grams = model->two_grams;
    short* three_grams = model->three_grams;

    memset(two_grams, 0, sizeof(short) * TWO_GRAMS_DICTIONARY_SIZE);
    memset(three_grams, 0, sizeof(short) * THREE_GRAMS_DICTIONARY_SIZE);

    unsigned char first = '\0';
    unsigned char second = '\0';
    int third = fgetc(f);

    char word[MAX_WORD_SIZE];
    int word_pos = 0;

    if ((third != ' ') && (third != '\n') && (third != '\r')){
        word[word_pos++] = third;
    }

    while (!feof(f)){

        first = second;
        second = third;
        third = fgetc(f);
        if (third == -1){
            break;
        }

        // Build word list
        {
            if ((isalnum(third)) && (word_pos < (MAX_WORD_SIZE - 1))){
                word[word_pos++] = third;
            }
            else {
                if (word_pos > 2){
                    word[word_pos] = '\0';
                    void *v = get_hash_table(model->word_count, word);
                    insert_hash_table(model->word_count, word, (void*) (((long)v) + 1));
                }
                word_pos = 0;
            }
        }

        // Build two-grams
        {
            if (isalpha(second) && isalpha(third)){

                assert((second != 0xFF));
                assert((third != 0xFF));
                unsigned int index = (second << 8) | ((unsigned char)third);
                assert((index >= 0) &&
                       (index < TWO_GRAMS_DICTIONARY_SIZE));

                two_grams[index]++;
            }
        }

        // Build three-grams
        {
            if (isalpha(first) && isalpha(second) && isalpha(third)){

                unsigned int index = (first << 16) | (second << 8) | ((unsigned char)third);
                assert((index >= 0) &&
                       (index < THREE_GRAMS_DICTIONARY_SIZE));

                three_grams[index]++;
            }
        }
    }

    return model;
}


void free_language_model(language_model* model){
    if (model != NULL){
        free_hash_table(model->word_count, NULL);
    }
    free(model);
}


unsigned long language_model_score(const language_model* model,
                                   const char *words){

    if ((words[0] == '\0') || (words[1] == '\0')){
        return 0;
    }

    double two_gram_score = 0;
    double three_gram_score = 0;
    int i;

    const short* two_grams = model->two_grams;
    for (i = 0; words[i + 1] != '\0'; i++){

        unsigned char first = words[i];
        unsigned char second = words[i + 1];

        unsigned int index = (first << 8) | second;

        assert((index >= 0) &&
               (index < TWO_GRAMS_DICTIONARY_SIZE));

        two_gram_score += two_grams[index] != 0;
    }

    const short* three_grams = model->three_grams;
    for (i = 0; words[i + 2] != '\0'; i++){

        unsigned char first = words[i];
        unsigned char second = words[i + 1];
        unsigned char third = words[i + 2];

        unsigned int index = (first << 16) | (second << 8) | third;

        assert((index >= 0) &&
               (index < THREE_GRAMS_DICTIONARY_SIZE));

        three_gram_score +=  three_grams[index] != 0;
    }


    // Classify words and garbage
    unsigned long word_score = 0;

    char word[MAX_WORD_SIZE];
    int word_pos = 0;
    int garbage_size = 0;

    for (i = 0; words[i] != '\0'; i++){
        if (isalpha(words[i])){
            if (word_pos < (MAX_WORD_SIZE - 1)){
                word[word_pos++] = words[i];
            }
            else {
                garbage_size += word_pos;
                word_pos = 0;
            }
        }
        else {
            if (!isblank(words[i])){
                garbage_size++;
            }
            if ((!isprint(words[i])) || (!isascii(words[i]))){
                garbage_size++;
            }

            if (word_pos > 1){
                word[word_pos] = '\0';

                void *v = get_hash_table(model->word_count, word);

                if (v != NULL){
                    word_score += pow(word_pos, 2);
                }
                else {
                    garbage_size += word_pos;
                }
            }
            else {
                garbage_size += word_pos;
            }
            word_pos = 0;
        }
    }

    if (word_pos > 1){
        word[word_pos] = '\0';

        void *v = get_hash_table(model->word_count, word);

        if (v != NULL){
            word_score += (pow(word_pos, 1.5) + (2 + log2((long) v)));
        }
        else {
            garbage_size += word_pos;
        }
    }
    else {
        garbage_size += word_pos;
    }


    const int expected_size = 34;
    int size_diff = abs(expected_size - i);

    unsigned long penalization_divider = garbage_size + 2 + pow(size_diff, 2) / 10;

    double general_score = ((two_gram_score * TWO_GRAM_SCORE_MODIFIER)
                         + (three_gram_score * THREE_GRAM_SCORE_MODIFIER)
                         + (word_score * WORD_SCORE_MODIFIER));

    unsigned long final_score = ((double) (10 * general_score)) / (pow(penalization_divider, 0.5));
    if((general_score * 10) < final_score){
        printf("%lf * 10 = %lf < %li\n\n", general_score, general_score * 10, final_score);
    }

    assert((general_score == 0) || ((general_score * 10) > final_score));

    return final_score;
}
