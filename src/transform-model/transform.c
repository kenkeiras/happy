#include "transform_model.h"
#include "../lang-model/model.h"
#include "cross.h"
#include "instruction_set.h"
#include "controller.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#define SHOW_INTERVAL 20

transform_model* new_model(){
    transform_model* model = malloc(sizeof(transform_model));

    model->output_size = -2;
    model->score = -2;
    model->program_size = -2;
    model->program = NULL;
    return model;
}


transform_model* transform_from_program(char *program){
    transform_model* transform = new_model();

    transform->program = strdup(program);
    transform->program_size = strlen(program);

    return transform;
}


transform_model* copy_model(transform_model* source){
    if (source == NULL){
        return NULL;
    }

    transform_model* transform = malloc(sizeof(transform_model));
    if (transform == NULL){
        return NULL;
    }

    if (source->program_size > 0){
        transform->program = malloc(sizeof(char) * source->program_size + 1);
        if (transform->program == NULL){
            free(transform);
            return NULL;
        }

        memcpy(transform->program, source->program, sizeof(char) * source->program_size);
        transform->program[source->program_size] = '\0';
    }
    else {
        transform->program = NULL;
    }

    transform->program_size = source->program_size;

    return transform;
}


int inv_language_score_cmp(const void* _model1,
                           const void* _model2){

    const transform_model* model1 = *(transform_model * const *) _model1;
    const transform_model* model2 = *(transform_model * const *) _model2;


    // Sort descendant
    return (model2->score) - (model1->score);
}


void shake(transform_model* population[]){
    int i;
    for (i = 0; i < POPULATION_SIZE; i++){
        mutate(population[i], 1.0f);
    }
}

transform_model* evolve_transform(
    const language_model* model,
    const char* text,
    int (*controller) (
        int iteration, transform_model* transform,
        const char* better_output, unsigned long score)){

    const int population_count = POPULATION_SIZE;
    transform_model* population[population_count];

    // Initial population
    {
        int i;
        for (i = 0; i < population_count; i++){
            population[i] = random_transform();
        }
    }


    long iteration;
    int done = 0;
    for (iteration = 0;!done;iteration++){
        {
            int i;
            for (i = 0; i < population_count; i++){
                char* out = process(population[i], text, model);
                free(out);
            }
        }


        qsort(&population, population_count, sizeof(transform_model*),
              inv_language_score_cmp);



        {
            transform_model* winner = population[0];
            char* better = process(winner, text, model);

            int action = controller(iteration, winner, better, winner->score);

            if ((iteration % SHOW_INTERVAL) == 0) {

                int limit = strlen(better);
                // Make the “better” string readable
                if (limit > 50){
                    strcpy(&better[40],
                           "\x1b[7m%\x1b[0m");

                    limit = 40;
                }

                int i;
                for (i = 0; i < limit; i++){
                    if ((!isalnum(better[i])) && (!ispunct(better[i])) && (better[i] != ' ')){

                        better[i] = '.';
                    }
                }

                printf("Iteration (%5li) [%5li | %3li]: |\x1b[1m%s\x1b[0m|\n",
                       iteration, winner->score,
                       winner->output_size, better);
            }

            free(better);
            switch(action){
            case EVOLVE_SHAKE:
                shake(population);

            case EVOLVE_CONTINUE:
                cross(population, iteration, text);
                break;

            case EVOLVE_DONE:
                done = 1;
                break;

            default:
                printf("Unknown action code: %i\n", action);
            }
        }
    }

    // Free population
    {
        int i;
        for (i = 1; i < population_count; i++){
            free_transform_model(population[i]);
        }
    }

    return population[0];
}


void free_transform_model(transform_model* model){
    if (model != NULL){
        free(model->program);
    }

    free(model);
}


void show_transform_model(transform_model* model){
    if (model == NULL){
        printf("(null)");
        return;
    }

    char *disassembled = disassemble(model);

    printf("[Sc: %li |Sz: %li]\n\x1b[1m%s\x1b[0m\n",
           model->score, model->output_size, disassembled);

    free(disassembled);
}
