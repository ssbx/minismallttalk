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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minist8.h"
#include "interpreter.h"
#include "objectmemory.h"
#include "primitives.h"

static int      __ExtractBits_to_of(int firstBitIndex, int lastBitIndex,
                                    int anInteger);

int
getProcPrio(int aProc)
{
    return OBJMEM_fetchPointer_ofObject(PriorityIndex, aProc);
}

int
linkLen(int aLinkedList)
{
    int             len = 0;
    int             link =
        OBJMEM_fetchPointer_ofObject(FirstLinkIndex, aLinkedList);
    while (link != NilPointer) {
        link = OBJMEM_fetchPointer_ofObject(NextLinkIndex, link);
        len++;
    }
    return len;
}

/*
 *
 * Routines
 *
 */

int
Interp_FetchInteger_ofObject(int fieldIndex, int objectPointer)
{
    int             integerPointer;

    integerPointer = OBJMEM_fetchPointer_ofObject(fieldIndex, objectPointer);
    if (OBJMEM_isIntegerObject(integerPointer))
        return OBJMEM_integerValueOf(integerPointer);
    else
        return primitiveFail();
}


void
Interp_StoreInteger_ofObject_withValue(int fieldIndex, int objectPointer,
                                       int integerValue)
{
    int             integerPointer;

    if (OBJMEM_isIntegerValue(integerValue)) {
        integerPointer = OBJMEM_integerObjectOf(integerValue);
        OBJMEM_storePointer_ofObject_withValue(fieldIndex, objectPointer,
                                               integerPointer);
    } else
        primitiveFail();
}

void
Interp_Transfer_fromIndex_ofObject_toIndex_ofObject(int count, int firstFrom,
                                                    int fromOop, int firstTo,
                                                    int toOop)
{
    int             fromIndex;
    int             toIndex;
    int             lastFrom;
    int             oop;
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

static int
__ExtractBits_to_of(int firstBitIndex, int lastBitIndex, int anInteger)
{
    uint16_t        mask = (1 << (lastBitIndex - firstBitIndex + 1)) - 1;
    uint16_t        shift = anInteger >> (15 - lastBitIndex);
    return shift & mask;
}

int
Interp_HighByteOf(int anInteger)
{
    return __ExtractBits_to_of(0, 7, anInteger);
}

int
Interp_LowByteOf(int anInteger)
{
    return __ExtractBits_to_of(8, 15, anInteger);
}

/**  **/



bool
Interp_IsInLowMemoryCondition()
{
    return R_mem.oopsLeftLimit > 0 && R_mem.wordsLeftLimit > 0
        && (OBJMEM_oopsLeft() < R_mem.oopsLeftLimit
            || OBJMEM_coreLeft() < R_mem.wordsLeftLimit);
}



char           *
Interp_StringFromObject(int stringOrSymbolPointer)
{
    int             len;
    char           *s;
    int             i;

    len = OBJMEM_fetchByteLengthOf(stringOrSymbolPointer);
    s = (char *) calloc(sizeof(char), len + 1);

    for (i = 0; i < len; i++) {
        s[i] = (char) OBJMEM_fetchByte_ofObject(i, stringOrSymbolPointer);
    }
    s[len] = '\0';
    return s;
}

int
Interp_StringObjectFor(const char *s)
{

    const char     *p;
    int             length;
    int             i;
    int             objectPointer;

    if (!s) {
        return NilPointer;
    }
    p = s;
    while (*p) {
        p++;
    }

    length = (int) (p - s);
    objectPointer = OBJMEM_instantiateClass_withBytes(ClassStringPointer,
                                                      length);
    p = s;
    for (i = 0; i < length; i++) {
        OBJMEM_storeByte_ofObject_withValue(i, objectPointer, *p++);
    }
    return objectPointer;
}


/*
 * Return a oop to the display bitmap
 */
/*
 * Reference may become invalid on the next cycle of the interpreter...
 */
int
Interp_GetDisplayBits(int width, int height)
{
    int             displayBits;
    int             computedSize;
    int             actualSize;

    /*
     * Return oop of display bits object
     */
    /*
     * sanity checking display width/height
     */
    if (G_currentDisplay == 0) {
        return 0;               /* No display yet */
    }
    displayBits = OBJMEM_fetchPointer_ofObject(BitsInForm, G_currentDisplay);
    computedSize = G_currentDisplayHeight * ((G_currentDisplayWidth + 15) / 16);
    actualSize = OBJMEM_fetchWordLengthOf(displayBits);
    /*
     * The sizes do not match...
     */
    /*
     * Smalltalk shrinks the display bitmap to a height of 100 behind our back
     */
    /*
     * when saving for example.
     */
    if (computedSize != actualSize) {
        return 0;
    }
    return displayBits;         /* ok then... */
}
/*
 * Allow read-only access to display form data
 */
int
Interp_FetchWord_OfDisplayBits(int wordIndex, int displayBits)
{
    return OBJMEM_fetchWord_ofObject(wordIndex, displayBits);
}


void
Interp_CheckLowMemoryConditions()
{
    R_mem.checkLowMemory = true;
}


void
Interp_AsynchronousSignal(int aSemaphore)
{
    asynchronousSignal(aSemaphore);
}


/*
 *
 * Contexts p580
 *
 */
int
instructionPointerOfContext(int contextPointer)
{
    return Interp_FetchInteger_ofObject(InstructionPointerIndex,
                                        contextPointer);
}
void
storeInstructionPointerValue_inContext(int value, int contextPointer)
{
    Interp_StoreInteger_ofObject_withValue(InstructionPointerIndex,
                                           contextPointer, value);
}
int
stackPointerOfContext(int contextPointer)
{
    return Interp_FetchInteger_ofObject(StackPointerIndex, contextPointer);
}
void
storeStackPointerValue_inContext(int value, int contextPointer)
{
    Interp_StoreInteger_ofObject_withValue(StackPointerIndex, contextPointer,
                                           value);
}
int
argumentCountOfBlock(int blockPointer)
{
    return Interp_FetchInteger_ofObject(BlockArgumentCountIndex, blockPointer);
}
void
fetchContextRegisters()
{
    if (isBlockContext(R_ctx.activeContext))
        R_ctx.homeContext =
            OBJMEM_fetchPointer_ofObject(HomeIndex, R_ctx.activeContext);
    else
        R_ctx.homeContext = R_ctx.activeContext;
    R_ctx.receiver =
        OBJMEM_fetchPointer_ofObject(ReceiverIndex, R_ctx.homeContext);
    R_ctx.method = OBJMEM_fetchPointer_ofObject(MethodIndex, R_ctx.homeContext);
    R_ctx.instructionPointer =
        instructionPointerOfContext(R_ctx.activeContext) - 1;
    R_ctx.stackPointer =
        stackPointerOfContext(R_ctx.activeContext) + TempFrameStart - 1;
}
bool
isBlockContext(int contextPointer)
{
    int             methodOrArguments;
    methodOrArguments =
        OBJMEM_fetchPointer_ofObject(MethodIndex, contextPointer);
    return OBJMEM_isIntegerObject(methodOrArguments);
}
void
storeContextRegisters()
{
    storeInstructionPointerValue_inContext(R_ctx.instructionPointer + 1,
                                           R_ctx.activeContext);
    storeStackPointerValue_inContext(R_ctx.stackPointer - TempFrameStart + 1,
                                     R_ctx.activeContext);
}
void
push(int object)
{
    R_ctx.stackPointer = R_ctx.stackPointer + 1;
    OBJMEM_storePointer_ofObject_withValue(R_ctx.stackPointer,
                                           R_ctx.activeContext, object);
}
int
popStack()
{
    int             topOfStack;
    topOfStack =
        OBJMEM_fetchPointer_ofObject(R_ctx.stackPointer, R_ctx.activeContext);
    R_ctx.stackPointer = R_ctx.stackPointer - 1;
    return topOfStack;
}
int
stackTop()
{
    return OBJMEM_fetchPointer_ofObject(R_ctx.stackPointer,
                                        R_ctx.activeContext);
}
int
stackValue(int offset)
{
    return OBJMEM_fetchPointer_ofObject(R_ctx.stackPointer - offset,
                                        R_ctx.activeContext);
}
void
pop(int number)
{
    R_ctx.stackPointer = R_ctx.stackPointer - number;
}
void
unPop(int number)
{
    R_ctx.stackPointer = R_ctx.stackPointer + number;
}
void
newActiveContext(int aContext)
{
    storeContextRegisters();
    OBJMEM_decreaseReferencesTo(R_ctx.activeContext);
    R_ctx.activeContext = aContext;
    OBJMEM_increaseReferencesTo(R_ctx.activeContext);
    fetchContextRegisters();
}
int
sender()
{
    return OBJMEM_fetchPointer_ofObject(SenderIndex, R_ctx.homeContext);
}
int
caller()
{
    return OBJMEM_fetchPointer_ofObject(SenderIndex, R_ctx.activeContext);
}
int
temporary(int offset)
{
    return OBJMEM_fetchPointer_ofObject(offset + TempFrameStart,
                                        R_ctx.homeContext);
}
int
literal(int offset)
{
    return literal_ofMethod(offset, R_ctx.method);
}

/*
 *
 * Classes p586
 *
 */
int
hash(int objectPointer)
{
    return objectPointer >> 1;
}
bool
lookupMethodInDictionary(int dictionary)
{
    int             length;
    int             index;
    int             mask;
    bool            wrapAround;
    int             nextSelector;
    int             methodArray;
    length = OBJMEM_fetchWordLengthOf(dictionary);
    mask = length - SelectorStart - 1;
    index = (mask & hash(R_cls.messageSelector)) + SelectorStart;
    wrapAround = false;
    for (;;) {
        nextSelector = OBJMEM_fetchPointer_ofObject(index, dictionary);
        if (nextSelector == NilPointer)
            return false;
        if (nextSelector == R_cls.messageSelector) {
            methodArray =
                OBJMEM_fetchPointer_ofObject(MethodArrayIndex, dictionary);
            R_cls.newMethod =
                OBJMEM_fetchPointer_ofObject(index - SelectorStart,
                                             methodArray);
            R_cls.primitiveIndex = primitiveIndexOf(R_cls.newMethod);
            return true;
        }
        index = index + 1;
        if (index == length) {
            if (wrapAround)
                return false;
            wrapAround = true;
            index = SelectorStart;
        }
    }
}
bool
lookupMethodInClass(int cls)
{
    int             currentClass;
    int             dictionary;
    currentClass = cls;
    while (currentClass != NilPointer) {
        dictionary = OBJMEM_fetchPointer_ofObject(MessageDictionaryIndex,
                                                  currentClass);
        if (lookupMethodInDictionary(dictionary))
            return true;
        currentClass = superclassOf(currentClass);
    }
    if (R_cls.messageSelector == DoesNotUnderstandSelector)
        error("Recursive not understood error encountered'");
    createActualMessage();
    R_cls.messageSelector = DoesNotUnderstandSelector;
    return lookupMethodInClass(cls);
}
int
superclassOf(int classPointer)
{
    return OBJMEM_fetchPointer_ofObject(SuperclassIndex, classPointer);
}
void
createActualMessage()
{
    int             argumentArray;
    int             message;
    argumentArray = OBJMEM_instantiateClass_withPointers(ClassArrayPointer,
                                                         R_cls.argumentCount);
    message = OBJMEM_instantiateClass_withPointers(ClassMessagePointer,
                                                   MessageSize);
    OBJMEM_storePointer_ofObject_withValue(MessageSelectorIndex, message,
                                           R_cls.messageSelector);
    OBJMEM_storePointer_ofObject_withValue(MessageArgumentsIndex, message,
                                           argumentArray);
    Interp_Transfer_fromIndex_ofObject_toIndex_ofObject(R_cls.argumentCount,
                                                        R_ctx.stackPointer -
                                                        (R_cls.argumentCount -
                                                         1),
                                                        R_ctx.activeContext, 0,
                                                        argumentArray);
    pop(R_cls.argumentCount);
    push(message);
    R_cls.argumentCount = 1;
}
int
instanceSpecificationOf(int classPointer)
{
    return OBJMEM_fetchPointer_ofObject(InstanceSpecificationIndex,
                                        classPointer);
}

bool
isPointers(int classPointer)
{
    int             pointersFlag;
    pointersFlag =
        __ExtractBits_to_of(0, 0, instanceSpecificationOf(classPointer));
    return pointersFlag == 1;
}
bool
isWords(int classPointer)
{
    int             wordsFlag;
    wordsFlag =
        __ExtractBits_to_of(1, 1, instanceSpecificationOf(classPointer));
    return wordsFlag == 1;
}
bool
isIndexable(int classPointer)
{
    int             indexableFlag;
    indexableFlag = __ExtractBits_to_of(2, 2,
                                        instanceSpecificationOf(classPointer));
    return indexableFlag == 1;
}
int
fixedFieldsOf(int classPointer)
{
    return __ExtractBits_to_of(4, 14, instanceSpecificationOf(classPointer));
}

/*
 *
 * Method Contexts
 *
 */

int
headerOf(int methodPointer)
{
    return OBJMEM_fetchPointer_ofObject(HeaderIndex, methodPointer);
}
int
literal_ofMethod(int offset, int methodPointer)
{
    return OBJMEM_fetchPointer_ofObject(offset + LiteralStart, methodPointer);
}

int
temporaryCountOf(int methodPointer)
{
    return __ExtractBits_to_of(3, 7, headerOf(methodPointer));
}

int
largeContextFlagOf(int methodPointer)
{
    return __ExtractBits_to_of(8, 8, headerOf(methodPointer));
}


int
literalCountOf(int methodPointer)
{
    return literalCountOfHeader(headerOf(methodPointer));
}
int
literalCountOfHeader(int headerPointer)
{
    return __ExtractBits_to_of(9, 14, headerPointer);
}
int
objectPointerCountOf(int methodPointer)
{
    return literalCountOf(methodPointer) + LiteralStart;
}

int
initialInstructionPointerOfMethod(int methodPointer)
{
    return (literalCountOf(methodPointer) + LiteralStart) * 2 + 1;
}

int
flagValueOf(int methodPointer)
{
    return __ExtractBits_to_of(0, 2, headerOf(methodPointer));
}
int
fieldIndexOf(int methodPointer)
{
    return __ExtractBits_to_of(3, 7, headerOf(methodPointer));
}
int
headerExtensionOf(int methodPointer)
{
    int             literalCount;
    literalCount = literalCountOf(methodPointer);
    return literal_ofMethod(literalCount - 2, methodPointer);
}
int
argumentCountOf(int methodPointer)
{
    int             flagValue;
    flagValue = flagValueOf(methodPointer);
    if (flagValue < 5)
        return flagValue;
    if (flagValue < 7)
        return 0;
    else
        return __ExtractBits_to_of(2, 6, headerExtensionOf(methodPointer));
}
int
primitiveIndexOf(int methodPointer)
{
    int             flagValue;
    flagValue = flagValueOf(methodPointer);
    if (flagValue == 7) {
        return __ExtractBits_to_of(7, 14, headerExtensionOf(methodPointer));
    }
    return 0;
}

int
methodClassOf(int methodPointer)
{
    int             literalCount;
    int             association;
    literalCount = literalCountOf(methodPointer);
    association = literal_ofMethod(literalCount - 1, methodPointer);
    return OBJMEM_fetchPointer_ofObject(ValueIndex, association);
}

/*
 *
 * bytecodes
 *
 */

/*
 * stack bytecodes
 */

void
pushReceiverVariableBytecode()
{
    int             fieldIndex;
    fieldIndex = __ExtractBits_to_of(12, 15, G_currentBytecode);
    pushReceiverVariable(fieldIndex);
}

void
pushReceiverVariable(int fieldIndex)
{
    push(OBJMEM_fetchPointer_ofObject(fieldIndex, R_ctx.receiver));
}

void
pushTemporaryVariableBytecode()
{
    int             fieldIndex;
    fieldIndex = __ExtractBits_to_of(12, 15, G_currentBytecode);
    pushTemporaryVariable(fieldIndex);
}
void
pushTemporaryVariable(int temporaryIndex)
{
    push(temporary(temporaryIndex));
}
void
pushLiteralConstantBytecode()
{
    int             fieldIndex;
    fieldIndex = __ExtractBits_to_of(11, 15, G_currentBytecode);
    pushLiteralConstant(fieldIndex);
}
void
pushLiteralConstant(int literalIndex)
{
    push(literal(literalIndex));
}
void
pushLiteralVariableBytecode()
{
    int             fieldIndex;
    fieldIndex = __ExtractBits_to_of(11, 15, G_currentBytecode);
    pushLiteralVariable(fieldIndex);
}
void
pushLiteralVariable(int literalIndex)
{
    int             association;
    association = literal(literalIndex);
    push(OBJMEM_fetchPointer_ofObject(ValueIndex, association));
}

void
pushReceiverBytecode()
{
    push(R_ctx.receiver);
}
void
duplicateTopBytecode()
{
    push(stackTop());
}

void
pushActiveContextBytecode()
{
    push(R_ctx.activeContext);
}
void
storeAndPopReceiverVariableBytecode()
{
    int             variableIndex;
    variableIndex = __ExtractBits_to_of(13, 15, G_currentBytecode);
    OBJMEM_storePointer_ofObject_withValue(variableIndex, R_ctx.receiver,
                                           popStack());
}

void
storeAndPopTemporaryVariableBytecode()
{
    int             variableIndex;
    variableIndex = __ExtractBits_to_of(13, 15, G_currentBytecode);
    OBJMEM_storePointer_ofObject_withValue(variableIndex + TempFrameStart,
                                           R_ctx.homeContext, popStack());
}

void
extendedStoreAndPopBytecode()
{
    extendedStoreBytecode();
    popStackBytecode();
}

void
extendedStoreBytecode()
{
    int             descriptor;
    int             variableType;
    int             variableIndex;
    int             association;
    descriptor = fetchByte();
    variableType = __ExtractBits_to_of(8, 9, descriptor);
    variableIndex = __ExtractBits_to_of(10, 15, descriptor);
    switch (variableType) {
    case 0:
        OBJMEM_storePointer_ofObject_withValue(variableIndex, R_ctx.receiver,
                                               stackTop());
        break;
    case 1:
        OBJMEM_storePointer_ofObject_withValue(variableIndex + TempFrameStart,
                                               R_ctx.homeContext, stackTop());
        break;
    case 2:
        error("illegal store");
        break;
    case 3:
        association = literal(variableIndex);
        OBJMEM_storePointer_ofObject_withValue(ValueIndex, association,
                                               stackTop());
        break;
    }
}

void
popStackBytecode()
{
    popStack();
}

void
jump(int offset)
{
    R_ctx.instructionPointer = R_ctx.instructionPointer + offset;
}
void
shortUnconditionalJump()
{
    int             offset;
    offset = __ExtractBits_to_of(13, 15, G_currentBytecode);
    jump(offset + 1);
}

void
longUnconditionalJump()
{
    int             offset;
    offset = __ExtractBits_to_of(13, 15, G_currentBytecode);
    jump((offset - 4) * 256 + fetchByte());
}

void
jumpIf_by(int condition, int offset)
{
    int             boolean;
    boolean = popStack();
    if (boolean == condition) {
        jump(offset);
    } else {
        if (!(boolean == TruePointer || boolean == FalsePointer)) {
            unPop(1);
            sendMustBeBoolean();
        }
    }
}

void
sendMustBeBoolean()
{
    sendSelector_argumentCount(MustBeBooleanSelector, 0);
}

void
shortConditionalJump()
{
    int             offset;
    offset = __ExtractBits_to_of(13, 15, G_currentBytecode);
    jumpIf_by(FalsePointer, offset + 1);
}


/*
 * send bytecodes
 */
void
sendLiteralSelectorBytecode()
{
    int             selector;
    selector = literal(__ExtractBits_to_of(12, 15, G_currentBytecode));
    sendSelector_argumentCount(selector,
                               __ExtractBits_to_of(10, 11,
                                                   G_currentBytecode) - 1);
}


/*
 * sendSelector:argumentCount:
 */
void
sendSelector_argumentCount(int selector, int count)
{
    int             newReceiver;
    R_cls.messageSelector = selector;
    R_cls.argumentCount = count;
    newReceiver = stackValue(R_cls.argumentCount);

#if 0
#ifdef DEBUGGING_SUPPORT
    /*
     * Debugging aid that helped me figure out why stuff
     */
    /*
     * wasn't working - intercept show: and error: messages and
     */
    /*
     * display them
     */
    const int       showSelector = 6640;        /* #show: */
    const int       errorSelector = 282;        /* #error: */
#if 0
    bool            showSend = true;
    if (selector == showSelector) {
        int             s = stackValue(argumentCount - 1);
        assert(OBJMEM_fetchClassOf(s) == ClassStringPointer);
        char           *text = Interp_StringFromObject(s);
        printf("Transcript: %s\n", text);
        free(text);
        showSend = false;
    } else if (selector == errorSelector) {
        int             s = stackValue(argumentCount - 1);
        assert(OBJMEM_fetchClassOf(s) == ClassStringPointer);
        char           *text = Interp_StringFromObject(s);
        printf("ERROR: %s\n", text);
        free(text);
        showSend = false;
    }
    if (showSend) {
        int             cls = OBJMEM_fetchClassOf(newReceiver);
        char           *className = classNameOfObject(newReceiver);
        char           *sel = selectorName(selector);
        printf("sendSelector %s [%d] (args = %d) to %d (%s) 0x%d\n",
               sel, selector, count, newReceiver, className, cls);
        free(className);
        free(sel);
    }
#endif
#endif
#endif
    sendSelectorToClass(OBJMEM_fetchClassOf(newReceiver));
}

void
sendSelectorToClass(int classPointer)
{
    findNewMethodInClass(classPointer);
    executeNewMethod();
}

void
findNewMethodInClass(int cls)
{
    int             hash;
    /*
     * This is not a great hash function
     */
    /*
     * Bits of History, pg.244 has a discussion and a better one
     */
    /*
     * hash = ((messageSelector ^ cls) & 0xff) << 2;
     */
    hash = (R_cls.messageSelector & cls & 0xff) << 2;   /* removed +1 since C */
    /*
     * arrays are zero
     */
    /*
     * based
     */
    if (G_methodCache[hash] == R_cls.messageSelector &&
        G_methodCache[hash + 1] == cls) {
        R_cls.newMethod = G_methodCache[hash + 2];
        R_cls.primitiveIndex = G_methodCache[hash + 3];
    } else {
        lookupMethodInClass(cls);
        G_methodCache[hash] = R_cls.messageSelector;
        G_methodCache[hash + 1] = cls;
        G_methodCache[hash + 2] = R_cls.newMethod;
        G_methodCache[hash + 3] = R_cls.primitiveIndex;
    }
}

void
initializeMethodCache()
{
    int             i;
    for (i = 0; i < sizeof(G_methodCache) / sizeof(G_methodCache[0]); i++)
        G_methodCache[i] = NilPointer;
}

void
executeNewMethod()
{
    if (!primitiveResponse())
        activateNewMethod();
}

void
activateNewMethod()
{
    int             contextSize;
    int             newContext;
    if (largeContextFlagOf(R_cls.newMethod) == 1)
        contextSize = 32 + TempFrameStart;
    else
        contextSize = 12 + TempFrameStart;
    newContext = OBJMEM_instantiateClass_withPointers(ClassMethodContextPointer,
                                                      contextSize);
    OBJMEM_storePointer_ofObject_withValue(SenderIndex, newContext,
                                           R_ctx.activeContext);
    storeInstructionPointerValue_inContext(initialInstructionPointerOfMethod
                                           (R_cls.newMethod), newContext);
    storeStackPointerValue_inContext(temporaryCountOf(R_cls.newMethod),
                                     newContext);
    OBJMEM_storePointer_ofObject_withValue(MethodIndex, newContext,
                                           R_cls.newMethod);
    Interp_Transfer_fromIndex_ofObject_toIndex_ofObject(R_cls.argumentCount + 1,
                                                        R_ctx.stackPointer -
                                                        R_cls.argumentCount,
                                                        R_ctx.activeContext,
                                                        ReceiverIndex,
                                                        newContext);
    pop(R_cls.argumentCount + 1);
    newActiveContext(newContext);
}

void
singleExtendedSendBytecode()
{
    int             descriptor;
    int             selectorIndex;

    descriptor = fetchByte();
    selectorIndex = __ExtractBits_to_of(11, 15, descriptor);
    sendSelector_argumentCount(literal(selectorIndex),
                               __ExtractBits_to_of(8, 10, descriptor));
}

void
doubleExtendedSendBytecode()
{
    int             count;
    int             selector;

    count = fetchByte();
    selector = literal(fetchByte());
    sendSelector_argumentCount(selector, count);
}

void
singleExtendedSuperBytecode()
{
    int             descriptor;
    int             selectorIndex;
    int             methodClass;

    descriptor = fetchByte();
    R_cls.argumentCount = __ExtractBits_to_of(8, 10, descriptor);
    selectorIndex = __ExtractBits_to_of(11, 15, descriptor);
    R_cls.messageSelector = literal(selectorIndex);
    methodClass = methodClassOf(R_ctx.method);
    sendSelectorToClass(superclassOf(methodClass));
}

void
doubleExtendedSuperBytecode()
{
    int             methodClass;
    R_cls.argumentCount = fetchByte();
    R_cls.messageSelector = literal(fetchByte());
    methodClass = methodClassOf(R_ctx.method);
    sendSelectorToClass(superclassOf(methodClass));
}

void
sendSpecialSelectorBytecode()
{
    int             selectorIndex;
    int             selector;
    int             count;

    if (!specialSelectorPrimitiveResponse()) {
        selectorIndex = (G_currentBytecode - 176) * 2;
        selector = OBJMEM_fetchPointer_ofObject(selectorIndex,
                                                SpecialSelectorsPointer);
        count =
            Interp_FetchInteger_ofObject(selectorIndex + 1,
                                         SpecialSelectorsPointer);
        sendSelector_argumentCount(selector, count);
    }
}

void
returnValue_to(int resultPointer, int contextPointer)
{
    int             sendersIP;

    if (contextPointer == NilPointer) {
        push(R_ctx.activeContext);
        push(resultPointer);
        sendSelector_argumentCount(CannotReturnSelector, 1);
        return;
    }
    sendersIP = OBJMEM_fetchPointer_ofObject(InstructionPointerIndex,
                                             contextPointer);
    if (sendersIP == NilPointer) {
        push(R_ctx.activeContext);
        push(resultPointer);
        sendSelector_argumentCount(CannotReturnSelector, 1);
        return;
    }
    OBJMEM_increaseReferencesTo(resultPointer);
    returnToActiveContext(contextPointer);
    push(resultPointer);
    OBJMEM_decreaseReferencesTo(resultPointer);
}

void
returnToActiveContext(int aContext)
{
    OBJMEM_increaseReferencesTo(aContext);
    nilContextFields();
    OBJMEM_decreaseReferencesTo(R_ctx.activeContext);
    R_ctx.activeContext = aContext;
    fetchContextRegisters();
}

void
nilContextFields()
{
    OBJMEM_storePointer_ofObject_withValue(SenderIndex, R_ctx.activeContext,
                                           NilPointer);
    OBJMEM_storePointer_ofObject_withValue(InstructionPointerIndex,
                                           R_ctx.activeContext, NilPointer);
}

void
dispatchOnThisBytecode()
{
    switch (G_currentBytecode) {
    case 0 ... 15:
        /*
         * Push Receiver Variable #iiii
         */
        pushReceiverVariableBytecode();
        break;
    case 16 ... 31:
        /*
         * Push Temporary Location #iiii
         */
        pushTemporaryVariableBytecode();
        break;
    case 32 ... 63:
        /*
         * Push Literal Constant #iiiii
         */
        pushLiteralConstantBytecode();
        break;
    case 64 ... 95:
        /*
         * Push Literal Variable #iiiii
         */
        pushLiteralVariableBytecode();
        break;
    case 96 ... 103:
        /*
         * Pop and Store Receiver Variable #iii
         */
        storeAndPopReceiverVariableBytecode();
        break;
    case 104 ... 111:
        /*
         * Pop and Store Temporary Location #iii
         */
        storeAndPopTemporaryVariableBytecode();
        break;
    case 112:
        pushReceiverBytecode();
        break;
    case 113:
        push(TruePointer);
        break;
    case 114:
        push(FalsePointer);
        break;
    case 115:
        push(NilPointer);
        break;
    case 116:
        push(MinusOnePointer);
        break;
    case 117:
        push(ZeroPointer);
        break;
    case 118:
        push(OnePointer);
        break;
    case 119:
        push(TwoPointer);
        break;
    case 120:
        /*
         * Returnreceiver [ii] From Message
         */
        returnValue_to(R_ctx.receiver, sender());
        break;
    case 121:
        /*
         * Return True [ii] From Message
         */
        returnValue_to(TruePointer, sender());
        break;
    case 122:
        /*
         * Return False [ii] From Message
         */
        returnValue_to(FalsePointer, sender());
        break;
    case 123:
        /*
         * Return Nil [ii] From Message
         */
        returnValue_to(NilPointer, sender());
        break;
    case 124:
        returnValue_to(popStack(), sender());
        /*
         * Return Stack Top From Message [i]
         */
        break;
    case 125:
        /*
         * Return Stack Top From Block [i]
         */
        returnValue_to(popStack(), caller());
        break;
    case 126 ... 127:
        /*
         * unused
         */
        break;
    case 128:
        /*
         * Push (Receiver Variable, Temporary Location, Lit eral Constant,
         */
        /*
         * Literal Variable) [jj] #kkkkkk
         */
        {
            int             descriptor;
            int             variableType;
            int             variableIndex;
            descriptor = fetchByte();
            variableType = __ExtractBits_to_of(8, 9, descriptor);
            variableIndex = __ExtractBits_to_of(10, 15, descriptor);
            switch (variableType) {
            case 0:
                pushReceiverVariable(variableIndex);
                break;
            case 1:
                pushTemporaryVariable(variableIndex);
                break;
            case 2:
                pushLiteralConstant(variableIndex);
                break;
            case 3:
                pushLiteralVariable(variableIndex);
                break;
            }
        }
        break;
    case 129:
        /*
         * Store (Receiver Variable, Temporary Location, Ille gal, Literal
         */
        /*
         * Variable) [jj] #kkkkkk
         */
        extendedStoreBytecode();
        break;
    case 130:
        /*
         * Pop and Store (Receiver Variable, Temporary Location, Illegal,
         */
        /*
         * Literal Variable) [jj] #kkkkkk
         */
        extendedStoreAndPopBytecode();
        break;
    case 131:
        /*
         * 131 Send Literal Selector #kkkkk With #jjj Arguments
         */
        singleExtendedSendBytecode();
        break;
    case 132:
        /*
         * 132 Send Literal Selector #kkkkkkkk With #jjjjjjjj Arguments
         */
        doubleExtendedSendBytecode();
        break;
    case 133:
        /*
         * 133 Send Literal Selector #kkkkk To Superclass With jjj Arguments
         */
        singleExtendedSuperBytecode();
        break;
    case 134:
        break;
        /*
         * 134 Send Literal Selector # kkkkkkkk To Superclass With jjjjjjjj
         */
        /*
         * Arguments
         */
        doubleExtendedSuperBytecode();
        break;
    case 135:
        /*
         * Pop Stack Top
         */
        popStackBytecode();
        break;
    case 136:
        /*
         * Duplicate Stack Top
         */
        duplicateTopBytecode();
        break;
    case 137:
        /*
         * Push Active Context
         */
        pushActiveContextBytecode();
        break;
    case 138 ... 143:
        /*
         * unused
         */
        break;
    case 144 ... 151:
        /*
         * Jump iii+ 1 (i.e., 1 through 8)
         */
        shortUnconditionalJump();
        break;
    case 152 ... 159:
        /*
         * Pop and Jump On False iii+ 1 (i.e., 1 through 8)
         */
        shortConditionalJump();
        break;
    case 160 ... 167:
        /*
         * Jump (iii .. 4).256 + jjjjjjjj
         */
        longUnconditionalJump();
        break;
    case 168 ... 171:
        /*
         * 168 ... 171 Pop and Jump On True i i*256 .. I-jjjjjjjj
         */
        {
            int             offset;
            offset = __ExtractBits_to_of(14, 15, G_currentBytecode);
            offset = offset * 256 + fetchByte();
            jumpIf_by(TruePointer, offset);
        }
        break;
    case 172 ... 175:
        /*
         * 172 ... 175 Pop and Jump On False ii.256 + jjjjjjjj
         */
        {
            int             offset;
            offset = __ExtractBits_to_of(14, 15, G_currentBytecode);
            offset = offset * 256 + fetchByte();
            jumpIf_by(FalsePointer, offset);
        }
        break;
    case 176 ... 207:
        /*
         * 176 ... 191 Send Arthmetic Message #iiii
         */
        /*
         * 192 ... 207 Send Special Message #iiii
         */
        sendSpecialSelectorBytecode();
        break;
    case 208 ... 255:
        /*
         * 208 ... 223 Send Literal Selector #iiii With No Arguments
         */
        /*
         * 224 ... 239 end Literal Selector #iiii With 1 Argument
         */
        /*
         * 240 ... 255 Send Literal Selector #iiii With 2 Arguments
         */
        sendLiteralSelectorBytecode();
        break;
    default:{
        }
    }
}


/*
 *
 *
 * Interpreter
 *
 *
 */

#ifdef GC_MARK_SWEEP
void
prepareForCollection()
{
    storeContextRegisters();
    OBJMEM_addRoot(SmalltalkPointer);
    OBJMEM_addRoot(R_ctx.activeContext);
    if (R_sch.newProcess != NilPointer) {
        OBJMEM_addRoot(R_sch.newProcess);
    }
}

void
collectionCompleted()
{
    OBJMEM_increaseReferencesTo(R_ctx.activeContext);
    fetchContextRegisters();
    if (R_sch.newProcessWaiting) {
        OBJMEM_increaseReferencesTo(R_sch.newProcess);
    }
}
#endif

#ifdef DEBUGGING_SUPPORT

char           *
selectorName(int selector)
{
    return Interp_StringFromObject(selector);
}

char           *
classNameOfObject(int objectPointer)
{
    return className(OBJMEM_fetchClassOf(objectPointer));
}

char           *
className(int classPointer)
{
    if (classPointer == ClassSmallInteger)
        return "SmallInteger";
    if (classPointer == NilPointer)
        return "UndefinedObject";
    int             symbol = OBJMEM_fetchPointer_ofObject(6, classPointer);
    if (OBJMEM_fetchClassOf(symbol) != ClassSymbolPointer)
        return "<unknown>";
    return Interp_StringFromObject(symbol);
}
#endif


/*
 * registers
 */
ContextRegisters R_ctx;
ClassRegisters  R_cls;
SchedRegisters  R_sch;
MemSignalRegister R_mem;

/*
 * other globals
 */
int             G_currentBytecode;
bool            G_successFlag;
int             G_methodCache[1024];
int             G_currentDisplay;
int             G_currentDisplayWidth;
int             G_currentDisplayHeight;
int             G_currentCursor;

int
fetchByte()
{
    int             byte;
    byte = OBJMEM_fetchByte_ofObject(R_ctx.instructionPointer, R_ctx.method);
    R_ctx.instructionPointer = R_ctx.instructionPointer + 1;
    return byte;
}


bool
INTERP_Cycle(bool unlock)
{
    checkProcessSwitch();
    G_currentBytecode = fetchByte();
    dispatchOnThisBytecode();
    return false;
}

bool
INTERP_Init(const char *image)
{
    initializeMethodCache();
    R_sch.semaphoreIndex = -1;
#ifdef GC_MARK_SWEEP
    struct gc_notif_i notif;
    notif.prepareForCollection = prepareForCollection;
    notif.collectionCompleted = collectionCompleted;
    OBJMEM_init(notif);
#else
    OBJMEM_init();
#endif
    if (!OBJMEM_loadSnapshot(image)) {
        fprintf(stderr,
                "VM failed to initialize (invalid/missing directory or snapshot?)");
        return false;
    }
    R_ctx.activeContext = firstContext();
    OBJMEM_increaseReferencesTo(R_ctx.activeContext);
    fetchContextRegisters();
    R_mem.checkLowMemory = false;
    R_mem.memoryIsLow = false;
    R_mem.lowSpaceSemaphore = NilPointer;
    R_mem.oopsLeftLimit = 0;
    R_mem.wordsLeftLimit = 0;
    G_currentDisplay = 0;
    G_currentCursor = 0;
    G_currentDisplayWidth = 0;
    G_currentDisplayHeight = 0;
    return true;
}
