#include "lang-model/model.h"
#include "transform-model/model.h"
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv){
    if (argc < 3){
        printf("%s <file> <text>\n", argc > 0? argv[0] : "happy");
        return 0;
    }

    FILE *f = fopen(argv[1], "rt");
    if (f == NULL){
        perror(argv[1]);
        return 1;
    }

    language_model* model = build_language_model(f);
    if (model == NULL){
        perror("Build language model");
        return 2;
    }

    fclose(f);

    long seed = 0x55CB9B70; //time(NULL);

    printf("Seed: 0x%lX\n", seed);
    srand(seed);
    transform_model* transform = evolve_transform(model, argv[2]);
    if (transform == NULL){
        perror("Evolve transform");
        return 3;
    }

    show_transform_model(transform);

    free_language_model(model);
    free_transform_model(transform);

    return 0;
}
