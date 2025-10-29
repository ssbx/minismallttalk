#ifndef _CONTEXTS_H_
#define _CONTEXTS_H_
#include <stdbool.h>
/*
 *
 * Contexts p580
 *
 */
int instructionPointerOfContext(int contextPointer);
void storeInstructionPointerValue_inContext(int value, int contextPointer);
int stackPointerOfContext(int contextPointer);
void storeStackPointerValue_inContext(int value, int contextPointer);
int argumentCountOfBlock(int blockPointer);
void fetchContextRegisters();
bool isBlockContext(int contextPointer);
void storeContextRegisters();
void push(int object);
int popStack();
int stackTop();
int stackValue(int offset);
void pop(int number);
void unPop(int number);
void newActiveContext(int aContext);
int sender();
int caller();
int temporary(int offset);
int literal(int offset);

#endif // _CONTEXTS_H_
