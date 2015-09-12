#ifndef TRANSFORM_MODEL_INSTRUCTION_SET_H
#define TRANSFORM_MODEL_INSTRUCTION_SET_H

#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "transform_model.h"
#include "opcodes.h"
#include "cross.h"

const int MAX_CYCLES = 100 * 1000;

char* process(transform_model* transform,
              const char* input,
              const language_model* model){

    uint8_t mem[256];
    uint8_t registers[31];

    size_t i;
    for (i = 0; i < 31; i ++){
        registers[i] = i;
    }
    for (i = 0; i < 256; i++){
        mem[i] = i;
    }

    uint8_t process_get(uint8_t target){
        if (target == 0){
            return 0;
        }
        else if(target < 32){
            return registers[target - 1];
        }
        else {
            return mem[process_get(target - 32)];
        }
    }


    unsigned long process_branching(uint8_t value, unsigned long ip){

        // Bit nº6 is the forward/backwards one
        int back = value >> 5;
        int jmp = value & 0x1F;

        // Avoid zero (infinite loop) and multiply per instruction size
        jmp = (jmp + 1) * PROGRAM_INSTRUCTION_SIZE;

        // Invert if the backwards bit was set
        if (back){
            jmp = -jmp;
        }

        return ip + jmp;
    }


    void process_set(uint8_t target, uint8_t value){
        assert(target < 64);

        if (target == 0){  // Cannot do
            return;
        }
        else if(target < 32){
            registers[target - 1] = value;
        }
        else {
            mem[process_get(value - 32)] = value;
        }
    }


    assert(transform != NULL);
    assert(transform->program != NULL);

    size_t input_length = strlen(input);
    const unsigned long max_cycles = MAX_CYCLES;

    unsigned long cycles = 0;
    size_t input_i = 0;
    size_t output_size = 128;
    size_t output_i = 0;
    char *output = malloc(sizeof(uint8_t) * output_size);

    unsigned long ip = 0;
    unsigned long nip = ip;

    assert(PROGRAM_INSTRUCTION_SIZE == 3);

    for (ip = 0; (cycles < max_cycles) && (ip < (transform->program_size)); ip = nip, cycles++){

        nip = ip + 3;

        // Positions (zero indexed)
        enum opcodes opcode = (transform->program[ip] >> 2) & 0x0F; // 02 - 06
        uint8_t first = ((transform->program[ip] & 0x03) << 4) // 06 - 12
            | ((transform->program[ip + 1] & 0xF0) >> 4);
        uint8_t second = ((transform->program[ip + 1] & 0x0F) << 2) // 12 - 18
            | ((transform->program[ip + 2] & 0xC0) >> 6);
        uint8_t third = (transform->program[ip + 2] & 0x3F);   // 18 - 24

        assert(first < 64);
        assert(second < 64);
        assert(third < 64);

        switch(opcode){
            // Arithmetic
        case ADD:  // $1 = $2 + $3
            process_set(first, process_get(second) + process_get(third));
            break;

        case SUB:  // $1 = $2 - $3
            process_set(first, process_get(second) - process_get(third));
            break;

        case MUL:  // $1 = $2 * $3
            process_set(first, process_get(second) * process_get(third));
            break;

        case DIV:  // $1 = $2 / $3
        {
            uint8_t second_value = process_get(second);
            uint8_t third_value = process_get(third);
            if (third_value != 0){
                process_set(first, second_value / third_value);
            }
            else {
                process_set(first, 0);
            }
        }
        break;

            // BITWISE
        case AND:  // $1 = $2 ^ $3
            process_set(first, process_get(second) & process_get(third));
            break;

        case NOT:  // $1 = ~$2
        {
            uint8_t is_logical = third;
            if (is_logical & 1){
                process_set(first, !process_get(second));
            }
            else {
                process_set(first, ~process_get(second));
            }
        }
            break;

        case IOR:  // $1 = $2 | $3
            process_set(first, process_get(second) | process_get(third));
            break;

        case XOR:  // $1 = $2 ^ $3
            process_set(first, process_get(second) ^ process_get(third));
            break;

            // Branching
        case JEQ:  //
            if (process_get(second) == process_get(third)){
                nip = process_branching(first, ip);
            }
            break;

        case JNE:  //
            if (process_get(second) != process_get(third)){
                nip = process_branching(first, ip);
            }
            break;

        case JLT:  //
            if (process_get(second) < process_get(third)){
                nip = process_branching(first, ip);
            }
            break;

        case JLE:  //
            if (process_get(second) <= process_get(third)){
                nip = process_branching(first, ip);
            }
            break;

        case JGT:  //
            if (process_get(second) > process_get(third)){
                nip = process_branching(first, ip);
            }
            break;

        case JGE:  //
            if (process_get(second) >= process_get(third)){
                nip = process_branching(first, ip);
            }
            break;

            // IO
        case OUT:  // $1 → STDOUT
            if ((output_i + 1) >= output_size){
                output = realloc(output, output_size + 128);
                assert(output != NULL);

                memset(&output[output_size], 0, 128);
                output_size += 128;
            }
            output[output_i++] = process_get(first);
            break;

        case INP:  // $1 ← STDIN
            if (input_i < input_length){
                process_set(first, input[input_i++]);
            }
            else {
                process_set(first, '\0');
            }
            break;

        default:
            printf("Unexpected opcode %i\n", opcode);
            abort();
        }
    }

    if ((output_i + 1) >= output_size){
        output = realloc(output, output_size + 128);
        assert(output != NULL);

        memset(&output[output_size], 0, 128);
        output_size += 128;
    }

    transform->output_size = output_i;
    output[output_i++] = '\0';

    transform->score = language_model_score(model, output);

    return output;
}

char *disassemble(transform_model* transform){

    size_t ip, nip;

    size_t disassembled_size = 0;
    char *disassembled = NULL;

    for (ip = 0; ip < (transform->program_size); ip = nip){

        nip = ip + 3;

        // Positions (zero indexed)
        enum opcodes opcode = (transform->program[ip] >> 2) & 0x0F; // 02 - 06
        uint8_t first = ((transform->program[ip] & 0x03) << 4) // 06 - 12
            | ((transform->program[ip + 1] & 0xF0) >> 4);
        uint8_t second = ((transform->program[ip + 1] & 0x0F) << 2) // 12 - 18
            | ((transform->program[ip + 2] & 0xC0) >> 6);
        uint8_t third = (transform->program[ip + 2] & 0x3F);   // 18 - 24

        assert(first < 64);
        assert(second < 64);
        assert(third < 64);

        char *mnemonic = NULL;

        switch(opcode){
            // Arithmetic
        case ADD:  // $1 = $2 + $3
            mnemonic = "add";
            break;

        case SUB:  // $1 = $2 - $3
            mnemonic = "sub";
            break;

        case MUL:  // $1 = $2 * $3
            mnemonic = "mul";
            break;

        case DIV:  // $1 = $2 / $3
            mnemonic = "div";
            break;

            // BITWISE
        case AND:  // $1 = $2 ^ $3
            mnemonic = "and";
            break;

        case NOT:  // $1 = ~$2
        {
            uint8_t is_logical = third;
            if (is_logical & 1){
                mnemonic = "lnot";
            }
            else {
                mnemonic = "bnot";
            }
        }
            break;

        case IOR:  // $1 = $2 | $3
                mnemonic = "ior";
            break;

        case XOR:  // $1 = $2 ^ $3
                mnemonic = "xor";
            break;

            // Branching
        case JEQ:  //
            mnemonic = "jeq";
            break;

        case JNE:  //
            mnemonic = "jne";
            break;

        case JLT:  //
            mnemonic = "jlt";
            break;

        case JLE:  //
            mnemonic = "jle";
            break;

        case JGT:  //
            mnemonic = "jgt";
            break;

        case JGE:  //
            mnemonic = "jge";
            break;

            // IO
        case OUT:  // $1 → STDOUT
            mnemonic = "out";
            break;

        case INP:  // $1 ← STDIN
            mnemonic = "in";
            break;

        default:
            printf("Unexpected opcode %i\n", opcode);
            abort();
        }


        size_t new_size = disassembled_size + strlen(mnemonic)
            + (3 + 2) * 3 + 1; // ", $XX" * 3 + "\n"
        disassembled = realloc(disassembled, new_size + 1);
        sprintf(&disassembled[disassembled_size], "%s, $%02X, $%02X, $%02X\n",
                mnemonic, first, second, third);
        disassembled_size = new_size;
    }

    return disassembled;
}


#endif
