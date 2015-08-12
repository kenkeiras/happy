#include "model.h"
#include "../lang-model/model.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char* PROGRAM_OPTIONS = ".,+-<>[] ";
const int PROGRAM_OPTION_COUNT = 9;

struct transform_model {
    long score;
    size_t output_size;
    size_t program_size;
    char* program;
};

transform_model* new_model(){
    transform_model* model = malloc(sizeof(transform_model));

    model->output_size = -2;
    model->score = -2;
    model->program_size = 0;
    model->program = NULL;
    return model;
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


void mutate(transform_model* transform){
    const int mutation_ratio = 5;

    int i;
    for (i = 0; i < transform->program_size; i++){
        if ((rand() % mutation_ratio) == 0){
            transform->program[i] = PROGRAM_OPTIONS[rand() % PROGRAM_OPTION_COUNT];
        }
    }
}


transform_model* random_transform(){
    transform_model* transform = new_model();

    const int size = 100;
    transform->program = malloc(sizeof(char) * size * 2 + 1);

    int i, open_brackets = 0;
    for(i = 0; i < size; i++){
        transform->program[i] = PROGRAM_OPTIONS[rand() % PROGRAM_OPTION_COUNT];
        if (transform->program[i] == '['){
            open_brackets++;
        }
        else if (transform->program[i] == ']'){
            if (open_brackets > 0){
                open_brackets--;
            }
            else {
                i--;
            }
        }
    }

    for (; open_brackets > 0; open_brackets--){
        transform->program[i++] = ']';
    }

    transform->program_size = i;
    transform->program = realloc(transform->program, sizeof(char) * i + 1);
    transform->program[i] = '\0';

    return transform;
}


int inv_language_score_cmp(const void* _model1,
                       const void* _model2){

    const transform_model* model1 = *(transform_model * const *) _model1;
    const transform_model* model2 = *(transform_model * const *) _model2;


    // Sort descendant
    return (model2->score) - (model1->score);
}


char* process(transform_model* transform,
              const char* input,
              const language_model* model){

    assert(transform != NULL);
    assert(transform->program != NULL);

    int input_length = strlen(input);
    const long max_cycles = input_length * 1000;
    const int max_depth = 256;
    int depth = 0;
    int loop_stack[max_depth];

    int output_size = 0;
    int output_heap_size = 128;
    char* output = malloc(sizeof(char) * output_heap_size);

    int mem_size = 128;
    char* mem = malloc(sizeof(char) * mem_size);
    memset(mem, 0, sizeof(char) * 128);

    int crashed = 0;
    int counter;
    int input_i = 0;
    int mem_dir = 0;
    int ip;

    for (ip = 0, counter = 0;
         (ip < transform->program_size) && (counter < max_cycles);
         counter++){

        switch(transform->program[ip++]){
        case '+':
            mem[mem_dir]++;
            break;

        case '-':
            mem[mem_dir]--;
            break;

        case '<':
            mem_dir--;
            if (mem_dir < 0){

                char* new_mem = malloc(sizeof(char) * (mem_size + 128));
                memset(new_mem, 0, sizeof(char) * 128);
                memcpy(&new_mem[128], mem, sizeof(char) * mem_size);

                free(mem);
                mem = new_mem;

                mem_size += 128;
                mem_dir += 128;

                assert(mem_dir >= 0);
            }
            break;

        case '>':
            mem_dir++;
            if (mem_dir >= mem_size){
                mem = realloc(mem, sizeof(char) * (mem_size + 128));
                memset(&mem[mem_size], 0, sizeof(char) * 128);

                mem_size += 128;
                assert(mem_dir < mem_size);
            }
            break;

        case '[':
            if (depth >= max_depth){ // Crash program
                ip = transform->program_size;
                crashed = 1;
            }
            loop_stack[depth++] = ip;
            break;

        case ']':
            if (depth < 1){  // Crash program
                ip = transform->program_size;
                crashed = 1;
            }
            else if (mem[mem_dir] != 0){
                ip = loop_stack[--depth];
            }
            break;

        case ',':
            if (input_i < input_length){
                mem[mem_dir] = input[input_i++];
            }
            else {
                mem[mem_dir] = '\0';
            }
            break;

        case '.':
            if (mem[mem_dir] == '\0'){
                break;
            }
            if ((output_size + 1) >= output_heap_size){
                output_heap_size += 128;
                output = realloc(output, sizeof(char) * output_heap_size);
            }
            output[output_size++] = mem[mem_dir];
            break;
        }
    }

    output[output_size] = '\0';
    transform->score = language_model_score(model, output) / (crashed + 1)
        + ((!crashed) && (output_size != 0));

    if (output_size > 0){
        transform->score = transform->score / pow(output_size / 10, 2);
    }
    transform->output_size = output_size;

    free(mem);

    return output;
}


void halp(){}

void cross(transform_model* population[], int iteration){
    // Consider a 128 individual population
    // Keep the first
    // Mutate the first 32
    // The next 31 are the cross of the first 5x7
    // And the remaining are mutations of the first 64

    // So, remove the last 64
    int i, j;
    for (i = 64; i < 128; i++){
        free_transform_model(population[i]);
    }

    // Cross the 5 x 7 first
    int index = 64;
    for (i = 0; i < 5; i++){ // for (i = 0; i < 5; i++){
        for (j = 0; j < 7; j++){
            if (i != j){
                population[index] = copy_model(population[i]);
                assert(population[index] != NULL);

                size_t i_size = population[i]->program_size;
                size_t j_size = population[j]->program_size;
                size_t middle = (rand() % i_size);

                size_t cp_size = j_size < i_size? j_size : i_size;


                if ((((int) cp_size) - ((int) middle)) > 0){

                    memcpy(&(population[index]->program[middle]),
                           &(population[j]->program[middle]),
                           sizeof(char) * (cp_size - middle));

                    population[index]->program_size = cp_size;
                }
                else {
                    population[index]->program_size = middle;
                }

                index++;
            }
        }
    }


    // Keep a copy of the first and second
    assert(index == (64 + 30));
    population[index++] = copy_model(population[0]);
    population[index++] = copy_model(population[1]);

    // Mutations of the first 32
    for (i = 0; index < 128; index++, i++){
        population[index] = copy_model(population[i]);
        mutate(population[index]);
    }

    // Mutations of the first 64
    for (i = 0; i < 64; i++){
        mutate(population[i]);
    }
}

transform_model* evolve_transform(language_model* model, char* text){

    /* const int input_length = strlen(text); */
    const int population_count = 128;
    transform_model* population[population_count];

    // Initial population
    {
        int i;
        for (i = 0; i < population_count; i++){
            population[i] = random_transform();
        }
    }


    long iterations = 99999999;

    long max_score = 0;
    long iteration;
    for (iteration = 0; iteration < iterations; iteration++){
        {
            int i;
            for (i = 0; i < population_count; i++){
                char* out = process(population[i], text, model);

                if (strcasecmp(out, "stars are made of weird stuff") == 0){
                    printf("Iteration (%li/%li) [Sc: %li |Sz: %li]: |\x1b[1m%s\x1b[0m|\n"
                           "\x1b[1;92;40m%s\x1b[0m\n\n",
                           iteration, iterations, population[i]->score,
                           population[i]->output_size, out, population[i]->program);
                    exit(0);
                }

                free(out);
            }
        }


        qsort(&population, population_count, sizeof(transform_model*),
              inv_language_score_cmp);

        {
            transform_model* winner = population[0];
            char* better = process(winner, text, model);
            char* sprog = malloc(sizeof(char) * winner->program_size + 1);
            memcpy(sprog, winner->program, sizeof(char) * winner->program_size);
            sprog[winner->program_size] = '\0';

            if ((iteration % 100) == 0) {
                printf("Iteration (%li/%li) [%li | %li]: %s\n%s\n\n",
                       iteration, iterations, winner->score,
                       winner->output_size, better, sprog);
            }

            assert(winner->score >= max_score);
            max_score = winner->score;

            free(sprog);
            free(better);
        }

        if (iteration != (iterations - 1)){
            cross(population, iteration);
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
}
