#include "lang-model/model.h"
#include "transform-model/model.h"
#include "transform-model/controller.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

const char* TEST_STR = "flag stars are made of weird stuff";
const unsigned long max_same_score = 4000;
const long max_time_without_bump  = 40000;


int controller(int iteration, transform_model* transform,
               const char* better_output, unsigned long score){

    static unsigned long last_score = 0;
    static unsigned long last_score_times = 0;
    static unsigned long last_bump_time = 0;

    if (strcmp(better_output, TEST_STR) == 0){
        printf("Found on iteration %i!\n", iteration);
        return EVOLVE_DONE;
    }

    if (score != last_score){
        last_score_times = 0;
        last_score = score;
    }
    else if (last_score_times++ > max_same_score){
        printf("#\x1b[1;40;96m Shake it! \x1b[0m\n");
        last_score = 0;
        last_score_times = 0;
        last_bump_time = 0;

        return EVOLVE_SHAKE;
    }

    if (last_bump_time++ > max_time_without_bump){
        printf("#\x1b[1;40;91m BUMP!! \x1b[0m\n");
        last_score = 0;
        last_score_times = 0;
        last_bump_time = 0;

        return EVOLVE_SHAKE;
    }


    return EVOLVE_CONTINUE;
}


void run(char *program, char *input){
    transform_model *transform = transform_from_program(program);

    char *output = process(transform, input, NULL);

    show_transform_model(transform);

    printf("<%li> %s\n", strlen(output), output);

    free(output);
    free_transform_model(transform);
}


int score(char *fname, char *text){
    FILE *f = fopen(fname, "rt");
    if (f == NULL){
        perror(fname);
        return 1;
    }

    language_model* model = build_language_model(f);
    if (model == NULL){
        perror("Build language model");
        return 2;
    }

    fclose(f);

    assert(language_model_score(model, "flag star")
           <
           language_model_score(model, "flag stars are made of weird"));


    printf("%li\n", language_model_score(model, text));

    free_language_model(model);

    return 0;
}


int evolve(char* fname, char* text){
    FILE *f = fopen(fname, "rt");
    if (f == NULL){
        perror(fname);
        return 1;
    }

    language_model* model = build_language_model(f);
    if (model == NULL){
        perror("Build language model");
        return 2;
    }

    fclose(f);

    assert(language_model_score(model, "flag star")
           <
           language_model_score(model, "flag stars are made of weird"));

    long seed = time(NULL);

    printf("Seed: 0x%lX\n", seed);
    srand(seed);
    transform_model* transform = evolve_transform(model, text, controller);
    if (transform == NULL){
        perror("Evolve transform");
        return 3;
    }

    show_transform_model(transform);
    free_language_model(model);
    free_transform_model(transform);

    return 0;
}


int main(int argc, char **argv){

    if ((argc == 4) && (strcmp(argv[1], "run") == 0)){
        run(argv[2], argv[3]);
        return 0;
    }

    if ((argc >= 4) && (strcmp(argv[1], "score") == 0)){
        int i;
        for (i = 3; i < argc; i++){
            int ret = score(argv[2], argv[i]);
            if (ret != 0){
                return ret;
            }
        }

        return 0;
    }

    if ((argc == 4) || (strcmp(argv[1], "evolve") == 0)){
        return evolve(argv[2], argv[3]);
    }

    printf("Evolve program: %s evolve <file> <text>\n", argc > 0? argv[0] : "happy");
    printf("Score output:   %s score  <file> <text> [...]\n", argc > 0? argv[0] : "happy");
    printf("Run program:    %s run    <file> <input>\n", argc > 0? argv[0] : "happy");

    return 0;

}
