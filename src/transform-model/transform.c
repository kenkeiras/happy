#include "model.h"
#include "../lang-model/model.h"
#include "controller.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char* PROGRAM_OPTIONS = ".,+-<>[]";
#define PROGRAM_OPTION_COUNT 8
#define MAX_MUTATION_RATE PROGRAM_OPTION_COUNT
const int PROGRAM_SIZE = 256;
const int POPULATION_SIZE = 256;
#define MAX_CYCLES 1000

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


void mutate(transform_model* transform, const int mutation_ratio){

    int i;
    for (i = 0; i < transform->program_size; i++){
        if ((mutation_ratio >= PROGRAM_OPTION_COUNT)
            || ((rand() % mutation_ratio) == 0)){
            transform->program[i] = PROGRAM_OPTIONS[rand() % PROGRAM_OPTION_COUNT];
        }
    }
}


transform_model* random_transform(){
    transform_model* transform = new_model();

    const int size = PROGRAM_SIZE;
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
    const unsigned long max_cycles = MAX_CYCLES;
    const int max_depth = 256;
    int depth = 0;
    int loop_stack[max_depth];

    int output_size = 0;
    int output_heap_size = 128;
    char* output = malloc(sizeof(char) * output_heap_size);

    int mem_size = 128;
    unsigned char* mem = malloc(sizeof(char) * mem_size);
    assert(mem != NULL);
    memset(mem, 0, sizeof(char) * mem_size);

    int crashed = 0;
    int input_i = 0;
    int mem_dir = 0;

    unsigned long counter;
    unsigned long ip;

    assert(transform->program_size >= 0);

    for (ip = counter = 0;(ip < transform->program_size) && (counter < max_cycles);
         counter++){

        switch(transform->program[ip++]){
        case '+':
            mem[mem_dir] = (mem[mem_dir] + 1) & 0xFF;
            break;

        case '-':
            mem[mem_dir] = (mem[mem_dir] - 1) & 0xFF;
            break;

        case '<':
            mem_dir--;
            if (mem_dir < 0){

                unsigned char* new_mem = malloc(sizeof(char) * (mem_size + 128));
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
            if (mem[mem_dir] == 0){ // Skip to the corresponding ']'
                int search_depth = 1;
                while ((search_depth > 0) && (ip < transform->program_size)){
                    switch(transform->program[ip++]){
                    case '[':
                        search_depth++;
                        break;

                    case ']':
                        search_depth--;
                        break;
                    }
                }

            }
            else if (depth >= max_depth){ // Crash program
                ip = transform->program_size;
                crashed = 1;
            }
            else {
                loop_stack[depth++] = ip - 1;
            }
            break;

        case ']':
            if (depth < 1){  // Crash program
                ip = transform->program_size;
                crashed = 1;
            }
            else {
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

    if (model != NULL){
        transform->score = language_model_score(model, output) / (crashed + 1)
            + ((!crashed) && (output_size != 0));
    }

    transform->output_size = output_size;

    free(mem);

    return output;
}

void shake(transform_model* population[]){
    int i;
    for (i = 0; i < POPULATION_SIZE; i++){
        mutate(population[i], MAX_MUTATION_RATE);
    }
}

void cross(transform_model* population[], int iteration, const char* text){
    // Mutations of the first half, the worst, the more mutations
    int index;
    int cut_point = POPULATION_SIZE / 2;
    for (index = 1; index < cut_point; index++){
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

            if ((iteration % 100) == 0) {
                char* sprog = malloc(sizeof(char) * winner->program_size + 1);
                memcpy(sprog, winner->program, sizeof(char) * winner->program_size);
                sprog[winner->program_size] = '\0';

                if (strlen(better) > 40){
                    strcpy(&better[30],
                           "\x1b[7m%\x1b[0m");
                }

                printf("Iteration (%5li) [%li | %li]: |%s|\n%s\n\n",
                       iteration, winner->score,
                       winner->output_size, better, sprog);

                free(sprog);
            }

            int action = controller(iteration, winner, better, winner->score);

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

    printf("[Sc: %li |Sz: %li]\n\x1b[1m%s\x1b[0m\n",
           model->score, model->output_size, model->program);
}
