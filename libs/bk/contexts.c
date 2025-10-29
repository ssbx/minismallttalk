/*
 * Copyright © 2025 Sébastien Serre. All rights reserved.
 * Copyright © 2020 Dan Banay. All rights reserved.
 *
 * This file is part of Minist8.
 *
 * Minist8 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Minist8 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Minist8. If not, see <https://www.gnu.org/licenses/>.
 */
#include "config.h"

#include "minist8.h"
#include "contexts.h"
#include "routines.h"
#include "compiledmethods.h"
#include "objectmemory.h"
/*
 *
 * Contexts p580
 *
 */
int instructionPointerOfContext(int contextPointer) {
  return fetchInteger_ofObject(InstructionPointerIndex, contextPointer);
}
void storeInstructionPointerValue_inContext(int value, int contextPointer) {
  storeInteger_ofObject_withValue(InstructionPointerIndex, contextPointer,
      value);
}
int stackPointerOfContext(int contextPointer) {
  return fetchInteger_ofObject(StackPointerIndex, contextPointer);
}
void storeStackPointerValue_inContext(int value, int contextPointer) {
  storeInteger_ofObject_withValue(StackPointerIndex, contextPointer, value);
}
int argumentCountOfBlock(int blockPointer) {
  return fetchInteger_ofObject(BlockArgumentCountIndex, blockPointer);
}
void fetchContextRegisters() {
  if (isBlockContext(R_ctx.activeContext))
    R_ctx.homeContext = OBJMEM_fetchPointer_ofObject(HomeIndex, R_ctx.activeContext);
  else
    R_ctx.homeContext = R_ctx.activeContext;
  R_ctx.receiver = OBJMEM_fetchPointer_ofObject(ReceiverIndex, R_ctx.homeContext);
  R_ctx.method = OBJMEM_fetchPointer_ofObject(MethodIndex, R_ctx.homeContext);
  R_ctx.instructionPointer = instructionPointerOfContext(R_ctx.activeContext) - 1;
  R_ctx.stackPointer = stackPointerOfContext(R_ctx.activeContext) + TempFrameStart - 1;
}
bool isBlockContext(int contextPointer) {
  int methodOrArguments;
  methodOrArguments = OBJMEM_fetchPointer_ofObject(MethodIndex, contextPointer);
  return OBJMEM_isIntegerObject(methodOrArguments);
}
void storeContextRegisters() {
  storeInstructionPointerValue_inContext(R_ctx.instructionPointer + 1, R_ctx.activeContext);
  storeStackPointerValue_inContext(R_ctx.stackPointer - TempFrameStart + 1,
      R_ctx.activeContext);
}
void push(int object) {
  R_ctx.stackPointer = R_ctx.stackPointer + 1;
  OBJMEM_storePointer_ofObject_withValue(R_ctx.stackPointer, R_ctx.activeContext, object);
}
int popStack() {
  int topOfStack;
  topOfStack = OBJMEM_fetchPointer_ofObject(R_ctx.stackPointer, R_ctx.activeContext);
  R_ctx.stackPointer = R_ctx.stackPointer - 1;
  return topOfStack;
}
int stackTop() {
  return OBJMEM_fetchPointer_ofObject(R_ctx.stackPointer, R_ctx.activeContext);
}
int stackValue(int offset) {
  return OBJMEM_fetchPointer_ofObject(R_ctx.stackPointer - offset, R_ctx.activeContext);
}
void pop(int number) {
  R_ctx.stackPointer = R_ctx.stackPointer - number;
}
void unPop(int number) {
  R_ctx.stackPointer = R_ctx.stackPointer + number;
}
void newActiveContext(int aContext) {
  storeContextRegisters();
  OBJMEM_decreaseReferencesTo(R_ctx.activeContext);
  R_ctx.activeContext = aContext;
  OBJMEM_increaseReferencesTo(R_ctx.activeContext);
  fetchContextRegisters();
}
int sender() {
  return OBJMEM_fetchPointer_ofObject(SenderIndex, R_ctx.homeContext);
}
int caller() {
  return OBJMEM_fetchPointer_ofObject(SenderIndex, R_ctx.activeContext);
}
int temporary(int offset) {
  return OBJMEM_fetchPointer_ofObject(offset + TempFrameStart, R_ctx.homeContext);
}
int literal(int offset) {
  return literal_ofMethod(offset, R_ctx.method);
}
