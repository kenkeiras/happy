#include "model.h"
#include "../lang-model/model.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct transform_model {
    int weight_count;
    int* weights;
    int score;
};

transform_model* new_model(int input_length){
    // 4 x 256 (Mesh)

    // Output (256)

    const int count = pow(256 * 5, 2);

    transform_model* model = malloc(sizeof(transform_model));
    model->weight_count = count;
    model->weights = malloc(sizeof(int) * count);
    return model;
}


transform_model* copy_model(transform_model* source){
    transform_model* transform = malloc(sizeof(transform_model));

    if (transform == NULL){
        return NULL;
    }

    const int count = pow(256 * 5, 2);

    transform->weight_count = count;
    transform->weights = malloc(sizeof(int) * transform->weight_count);
    memcpy(transform->weights,
           source->weights,
           sizeof(int) * transform->weight_count);

    return transform;
}


void mutate(transform_model* transform){
    const int mutation_ratio = 2000;

    int i;
    for (i = 0; i < transform->weight_count; i++){
        if ((rand() % mutation_ratio) == 0){
            transform->weights[i] = (rand() % 16) - 8;
        }
    }
}


transform_model* random_transform(int input_length){
    transform_model* transform = new_model(input_length);

    int i,
        weights = transform->weight_count;

    for (i = 0; i < weights; i++){
        transform->weights[i] = 0;
    }

    mutate(transform);

    return transform;
}


int language_score_cmp(const void* _model1,
                       const void* _model2){

    const transform_model* model1 = _model1;
    const transform_model* model2 = _model2;


    return model1->score - model2->score;
}


void shake(int* weights, int* values){
    int i;
    for (i = 256; i < (256 * 6); i++){

        int j;
        for (j = 0; j < (256 * 5); j++){
            values[i] += values[j]
                * weights[(i - 256) * (256 * 5) + j];
        }
    }
}


void process(transform_model* transform,
              char* input,
              language_model* model){

    int input_length = strlen(input);
    const long max_its = pow(input_length, 1.5);

    int size = 0;
    int mem_size = 128;
    char* output = malloc(sizeof(char) * mem_size);

    int input_i = 0;
    int i;

    int* weights = transform->weights;
    int* values = malloc(sizeof(int) * 256 * 6);
    for (i = 0; i < (256 * 6); i++){
        values[i] = 0;
    }

    for (i = 0; i < max_its; i++){
        // Reset input cells
        {
            int i;
            for (i = 0; i < 256; i++){
                values[i] = 0;
            }
        }

        // Activate input cell for this input
        if (input_i >= input_length) {
            values[0] = 1;
        }
        else {
            values[(unsigned char)input[input_i++]] = 1;
        }

        shake(weights, values);

        // Read output
        int* outputs = &values[256 * 5];
        int num_active = 0;
        int last = 0;
        {
            int i;
            for (i = 0; i < 256; i++){

                /* printf("%i ", outputs[i]); */

                if (outputs[i] > 0){
                    num_active++;
                    last = i;
                }
            }
        }

        printf("%i ", num_active);
        if (num_active == 1){
            if (outputs[0] > 0){
                break;
            }
            else {
                if ((size + 1) >= mem_size){
                    mem_size += 128;
                    output = realloc(output, mem_size);
                }

                output[size++] = last;
            }
        }
    }

    output[size] = '\0';
    transform->score = language_model_score(model, output) + (size != 0);

    printf("\n\x1b[1;97;41m[Sz: %i]\x1b[0m"
           "\x1b[1;97;42m[Sc: %i]\x1b[0m"
           " -> \x1b[1m{%s}\x1b[0m\n",
           size, transform->score, output);

    free(output);
}


void cross(transform_model* population[]){
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
    for (i = 0; i < 5; i++){
        for (j = 0; j < 7; j++){
            if (i != j){
                population[index] = copy_model(population[i]);

                int c = population[index]->weight_count;
                int middle = (rand() % c);
                memcpy(&(population[index]->weights[middle]),
                       &(population[j]->weights[middle]),
                       sizeof(int) * (c - middle));

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

    int input_length = strlen(text);
    const int population_count = 128;
    transform_model* population[population_count];

    // Initial population
    {
        int i;
        for (i = 0; i < population_count; i++){
            population[i] = random_transform(input_length);
        }
    }


    long iterations = pow(input_length, 3);

    long iteration;
    for (iteration = 0; iteration < iterations; iteration++){
        {
            int i;
            for (i = 0; i < population_count; i++){
                printf("<%3i/128>\n", i + 1);
                process(population[i], text, model);
            }
        }


        qsort(population, population_count,
              sizeof(transform_model*), language_score_cmp);


        if (iteration != (iterations - 1)){
            cross(population);
        }
        //if ((iteration % 100) == 0){
            /* char *better = process(population[0], text); */
            printf("Iteration (%li/%li)\n", //  [%i]: %s\n\n",
                    iteration, iterations);
            /*        population[0]->score, better); */
            /* free(better); */
        //}
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
        free(model->weights);
    }

    free(model);
}


void show_transform_model(transform_model* model){
    if (model == NULL){
        printf("(null)");
        return;
    }
}
