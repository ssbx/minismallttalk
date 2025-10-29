
#ifndef _INTERPRETER_ROUTINES_H_
#define _INTERPRETER_ROUTINES_H_

int fetchByte();
void storeInteger_ofObject_withValue(int fieldIndex, int objectPointer,
    int integerValue);
int fetchInteger_ofObject(int fieldIndex, int objectPointer);
void transfer_fromIndex_ofObject_toIndex_ofObject(int count, int firstFrom,
    int fromOop, int firstTo, int toOop);
int extractBits_to_of(int firstBitIndex, int lastBitIndex, int anInteger);
int lowByteOf(int anInteger);
int highByteOf(int anInteger);
bool isInLowMemoryCondition();
#ifdef DEBUGGING_SUPPORT
char* selectorName(int selector);
char* classNameOfObject(int objectPointer);
char* className(int classPointer);
#endif
int stringObjectFor(const char*);
char* stringFromObject(int);
void prepareForCollection();
void collectionCompleted();
#endif // _INTERPRETER_ROUTINES_H_

