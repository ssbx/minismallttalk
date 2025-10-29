#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "minist8.h"
#include "objectmemory.h"
#include "routines.h"
#include "primitives.h"


int fetchByte() {
  int byte;
  byte = OBJMEM_fetchByte_ofObject(R_ctx.instructionPointer, R_ctx.method);
  R_ctx.instructionPointer = R_ctx.instructionPointer + 1;
  return byte;
}

int getProcPrio(int aProc) {
    return OBJMEM_fetchPointer_ofObject(PriorityIndex, aProc);
}

int linkLen(int aLinkedList) {
    int len = 0;
    int link = OBJMEM_fetchPointer_ofObject(FirstLinkIndex, aLinkedList);
    while (link != NilPointer) {
      //prtf(stderr, " %i:%i ", link, getProcPrio(link));
      link = OBJMEM_fetchPointer_ofObject(NextLinkIndex, link);
      len++;
    }
    return len;
}
void storeInteger_ofObject_withValue(int fieldIndex, int objectPointer,
    int integerValue) {
  int integerPointer;
  if (OBJMEM_isIntegerValue(integerValue)) {
    integerPointer = OBJMEM_integerObjectOf(integerValue);
    OBJMEM_storePointer_ofObject_withValue(fieldIndex, objectPointer,
        integerPointer);
  } else
    primitiveFail();
}

int fetchInteger_ofObject(int fieldIndex, int objectPointer) {
  int integerPointer;
  integerPointer = OBJMEM_fetchPointer_ofObject(fieldIndex, objectPointer);
  if (OBJMEM_isIntegerObject(integerPointer))
    return OBJMEM_integerValueOf(integerPointer);
  else
    return primitiveFail();
}
void transfer_fromIndex_ofObject_toIndex_ofObject(int count, int firstFrom,
    int fromOop, int firstTo, int toOop) {
  int fromIndex;
  int toIndex;
  int lastFrom;
  int oop;
  fromIndex = firstFrom;
  lastFrom = firstFrom + count;
  toIndex = firstTo;
  while (fromIndex < lastFrom) {
    oop = OBJMEM_fetchPointer_ofObject(fromIndex, fromOop);
    OBJMEM_storePointer_ofObject_withValue(toIndex, toOop, oop);
    OBJMEM_storePointer_ofObject_withValue(fromIndex, fromOop, NilPointer);
    fromIndex = fromIndex + 1;
    toIndex = toIndex + 1;
  }
}
int extractBits_to_of(int firstBitIndex, int lastBitIndex, int anInteger) {
  uint16_t mask = (1 << (lastBitIndex - firstBitIndex + 1)) - 1;
  uint16_t shift = anInteger >> (15 - lastBitIndex);
  return shift & mask;
}

int lowByteOf(int anInteger) {
  return extractBits_to_of(8, 15, anInteger);
}

int highByteOf(int anInteger) {
  return extractBits_to_of(0, 7, anInteger);
}



bool isInLowMemoryCondition() {
  return R_mem.oopsLeftLimit > 0 && R_mem.wordsLeftLimit > 0
      && (OBJMEM_oopsLeft() < R_mem.oopsLeftLimit
          || OBJMEM_coreLeft() < R_mem.wordsLeftLimit);
}


#ifdef DEBUGGING_SUPPORT

char* selectorName(int selector)
{
  return stringFromObject(selector);
}

char* classNameOfObject(int objectPointer)
{
  return className(OBJMEM_fetchClassOf(objectPointer));
}

char* className(int classPointer)
{
  if (classPointer == ClassSmallInteger)
    return "SmallInteger";
  if (classPointer == NilPointer)
    return "UndefinedObject";
  int symbol = OBJMEM_fetchPointer_ofObject(6, classPointer);
  if (OBJMEM_fetchClassOf(symbol) != ClassSymbolPointer)
    return "<unknown>";
  return stringFromObject(symbol);
}
#endif



char* stringFromObject(int stringOrSymbolPointer) {
  int len = OBJMEM_fetchByteLengthOf(stringOrSymbolPointer);
  char *s = (char*) calloc(sizeof(char), len + 1);
  for (int i = 0; i < len; i++) {
    s[i] = (char) OBJMEM_fetchByte_ofObject(i, stringOrSymbolPointer);
  }
  s[len] = '\0';
  return s;
}

int stringObjectFor(const char *s) {
  if (!s) return NilPointer;
  const char *p = s;
  while (*p)
    p++;
  int length = (int) (p - s);
  int objectPointer = OBJMEM_instantiateClass_withBytes(ClassStringPointer,
      length);
  p = s;
  for (int i = 0; i < length; i++) {
    OBJMEM_storeByte_ofObject_withValue(i, objectPointer, *p++);
  }
  return objectPointer;
}


// Return a oop to the display bitmap
// Reference may become invalid on the next cycle of the interpreter...
int INTERP_GetDisplayBits(int width, int height) {
  // Return oop of display bits object
  // sanity checking display width/height
  if (G_currentDisplay == 0) return 0; // No display yet
  int displayBits = OBJMEM_fetchPointer_ofObject(BitsInForm, G_currentDisplay);
  int computedSize = G_currentDisplayHeight * ((G_currentDisplayWidth + 15) / 16);
  int actualSize = OBJMEM_fetchWordLengthOf(displayBits);
  // The sizes do not match...
  // Smalltalk shrinks the display bitmap to a height of 100 behind our back
  // when saving for example.
  if (computedSize != actualSize) return 0;
  return displayBits; // ok then...
}
// Allow read-only access to display form data
int INTERP_FetchWord_OfDisplayBits(int wordIndex, int displayBits) {
  return OBJMEM_fetchWord_ofObject(wordIndex, displayBits);
}




#ifdef GC_MARK_SWEEP
void prepareForCollection() {
  storeContextRegisters();
  OBJMEM_addRoot(SmalltalkPointer);
  OBJMEM_addRoot(R_ctx.activeContext);
  if (R_sch.newProcess != NilPointer) {
    OBJMEM_addRoot(R_sch.newProcess);
  }
}

void collectionCompleted() {
  OBJMEM_increaseReferencesTo(R_ctx.activeContext);
  fetchContextRegisters();
  if (R_sch.newProcessWaiting) {
    OBJMEM_increaseReferencesTo(R_sch.newProcess);
  }
}
#endif

void INTERP_CheckLowMemoryConditions() {
  R_mem.checkLowMemory = true;
}


void INTERP_AsynchronousSignal(int aSemaphore) {
  asynchronousSignal(aSemaphore);
}
