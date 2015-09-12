#ifndef TRANSFORM_MODEL_OPCODES_H
#define TRANSFORM_MODEL_OPCODES_H

enum opcodes {
    // Arithmetic
    ADD =  0, // 0000
    SUB =  1, // 0001
    MUL =  2, // 0010
    DIV =  3, // 0011

    // Bitwise
    AND =  4, // 0100
    NOT =  5, // 0101
    IOR =  6, // 0110
    XOR =  7, // 0111

    // Branching
    JEQ =  8, // 1000
    JNE =  9, // 1001
    JLT = 10, // 1010
    JLE = 11, // 1011
    JGT = 12, // 1100
    JGE = 13, // 1101

    // IO
    OUT = 14, // 1110
    INP = 15, // 1111
};

#endif
