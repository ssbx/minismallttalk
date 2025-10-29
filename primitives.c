#include "config.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <stdio.h>

#include "minist8.h"
#include "system.h"
#include "interpreter.h"
#include "bitblt.h"
#include "objectmemory.h"

bool            get_success();
bool            primitiveResponse();
float           extractFloat(int objectPointer);
int             popInteger();
int             lengthOf(int array);
int             positive16BitIntegerFor(int integerValue);
int             removeFirstLinkOfList(int aLinkedList);
int             positive16BitValueOf(int integerPointer);
int             positive32BitIntegerFor(int integerValue);
int             primitiveFail();
int             isEmptyList(int aLinkedList);
int             specialSelectorPrimitiveResponse();
uint32_t        positive32BitValueOf(int integerPointer);
void            arithmeticSelectorPrimitive();
void            commonSelectorPrimitive();
void            dispatchArithmeticPrimitives();
void            dispatchPrimitives();
void            initPrimitive();
void            pushInteger(int integerValue);
void            quickInstanceLoad();
void            quickReturnSelf();
void            success(bool successValue);
int             schedulerPointer();
void            memWarn();
void            primitiveBlockCopy();
void            primitiveValue();
void            primitiveValueWithArgs();
void            primitivePerform();
void            primitivePerformWithArgs();
void            synchronousSignal(int aSemaphore);
void            asynchronousSignal(int aSemaphore);
void            synchronousSignal(int aSemaphore);
void            transferTo(int aProcess);
void            checkProcessSwitch();
void            addLastLink_toList(int aLink, int aLinkedList);
void            sleep(int aProcess);
void            suspendActive();
void            resume(int aProcess);
void            primitiveSignal();
void            primitiveWait();
void            primitiveResume();
void            primitiveSuspend();
int             activeProcess();
void            primitiveFlushCache();
void            primitiveMousePoint();
void            primitiveCursorLocPut();
void            primitiveCursorLink();
void            primitiveInputSemaphore();
void            primitiveSampleInterval();
void            primitiveInputWord();
void            primitiveCopyBits();
void            primitiveSnapshot();
void            primitiveTimeWordsInto();
void            primitiveTickWordsInto();
void            primitiveSignalAtTick();
void            primitiveBeCursor();
void            primitiveBeDisplay();
void            primitiveScanCharacters();
void            primitiveDrawLoop();
void            primitiveStringReplace();
void            checkIndexableBoundsOf_in(int index, int array);
void            subscript_with_storing(int array, int index, int value);
void            primitiveAt();
void            primitiveAtPut();
void            primitiveSize();
void            primitiveStringAt();
void            primitiveStringAtPut();
void            primitiveNext();
void            primitiveNextPut();
void            primitiveAtEnd();
void            pushFloat(float f);
void            primitiveAsFloat();
void            primitiveFloatAdd();
void            primitiveFloatSubtract();
void            primitiveFloatLessThan();
void            primitiveFloatGreaterThan();
void            primitiveFloatLessOrEqual();
void            primitiveFloatGreaterOrEqual();
void            primitiveFloatEqual();
void            primitiveFloatNotEqual();
void            primitiveFloatMultiply();
void            primitiveFloatDivide();
void            primitiveTruncated();
void            primitiveFractionalPart();
void            primitiveExponent();
void            primitiveTimesTwoPower();
void            primitiveAdd();
void            primitiveSubtract();
void            primitiveLessThan();
void            primitiveGreaterThan();
void            primitiveLessOrEqual();
void            primitiveGreaterOrEqual();
void            primitiveEqual();
void            primitiveNotEqual();
void            primitiveMultiply();
void            primitiveDivide();
void            primitiveMod();
void            primitiveDiv();
void            primitiveQuo();
void            primitiveBitAnd();
void            primitiveBitOr();
void            primitiveBitXor();
void            primitiveBitShift();
void            primitiveMakePoint();
void            primitiveBeSnapshotFile();
void            primitivePosixFileOperation();
void            primitivePosixDirectoryOperation();
void            primitivePosixLastErrorOperation();
void            primitivePosixErrorStringOperation();
void            primitiveInstVarAtPut();
void            primitiveAsOop();
void            primitiveAsObject();
void            primitiveSomeInstance();
void            primitiveNextInstance();
void            primitiveNewMethod();
void            primitiveObjectAt();
void            primitiveObjectAtPut();
void            primitiveNew();
void            primitiveNewWithArg();
void            primitiveBecome();
void            primitiveInstVarAt();
void            checkInstanceVariableBoundsOf_in(int index, int object);
void            primitiveEquivalent();
void            primitiveClass();
void            primitiveCoreLeft();
void            primitiveQuit();
void            primitiveExitToDebugger();
void            primitiveOopsLeft();
void            primitiveSignalAtOopsLeftWordsLeft();
void            primitiveTestSeb();
void            primitiveNextInputString();
void            primitiveShowStdout();

void
error(const char *message)
{
    System_Error(message);
}



/*
 * --------------------- prim_Control.inc.h -----------------------------
 */
/*
 * util
 */
void
memWarn()
{
    if (R_mem.checkLowMemory) {
        bool            memoryWasLow = R_mem.memoryIsLow;
        R_mem.memoryIsLow = false;
        /* the Smalltalk system treat oops or words limits being zero as don't */
        /* check */
        if (R_mem.lowSpaceSemaphore != NilPointer && R_mem.oopsLeftLimit > 0
            && R_mem.wordsLeftLimit > 0) {
            if (Interp_IsInLowMemoryCondition()) {
                OBJMEM_garbageCollect();        /* Try to get some memory */
                /* back... */
                if (Interp_IsInLowMemoryCondition()) {
                    R_mem.memoryIsLow = true;
                    if (!memoryWasLow)
                        asynchronousSignal(R_mem.lowSpaceSemaphore);
                }
            }
        }
        R_mem.checkLowMemory = false;
    }
}

/*
 * bluebook
 */
void
primitiveBlockCopy()
{
    int             context;
    int             methodContext;
    int             blockArgumentCount;
    int             newContext;
    int             initialIP;
    int             contextSize;
    blockArgumentCount = popStack();
    context = popStack();
    if (isBlockContext(context))
        methodContext = OBJMEM_fetchPointer_ofObject(HomeIndex, context);
    else
        methodContext = context;
    contextSize = OBJMEM_fetchWordLengthOf(methodContext);
    newContext = OBJMEM_instantiateClass_withPointers(ClassBlockContextPointer,
                                                      contextSize);
    initialIP = OBJMEM_integerObjectOf(R_ctx.instructionPointer + 3);
    OBJMEM_storePointer_ofObject_withValue(InitialIPIndex, newContext,
                                           initialIP);
    OBJMEM_storePointer_ofObject_withValue(InstructionPointerIndex, newContext,
                                           initialIP);
    storeStackPointerValue_inContext(0, newContext);
    OBJMEM_storePointer_ofObject_withValue(BlockArgumentCountIndex, newContext,
                                           blockArgumentCount);
    OBJMEM_storePointer_ofObject_withValue(HomeIndex, newContext,
                                           methodContext);
    push(newContext);
}

void
primitiveValue()
{
    int             blockContext;
    int             blockArgumentCount;
    int             initialIP;
    blockContext = stackValue(R_cls.argumentCount);
    blockArgumentCount = argumentCountOfBlock(blockContext);
    success(R_cls.argumentCount == blockArgumentCount);
    if (get_success()) {
        Interp_Transfer_fromIndex_ofObject_toIndex_ofObject(R_cls.argumentCount,
                                                            R_ctx.stackPointer -
                                                            R_cls.
                                                            argumentCount + 1,
                                                            R_ctx.activeContext,
                                                            TempFrameStart,
                                                            blockContext);
        pop(R_cls.argumentCount + 1);
        initialIP = OBJMEM_fetchPointer_ofObject(InitialIPIndex, blockContext);
        OBJMEM_storePointer_ofObject_withValue(InstructionPointerIndex,
                                               blockContext, initialIP);
        storeStackPointerValue_inContext(R_cls.argumentCount, blockContext);
        OBJMEM_storePointer_ofObject_withValue(CallerIndex, blockContext,
                                               R_ctx.activeContext);
        newActiveContext(blockContext);
    }
}

void
primitiveValueWithArgs()
{
    int             argumentArray;
    int             blockContext;
    int             blockArgumentCount;
    int             arrayClass;
    int             arrayArgumentCount = 0;
    int             initialIP;

    argumentArray = popStack();
    blockContext = popStack();
    blockArgumentCount = argumentCountOfBlock(blockContext);
    arrayClass = OBJMEM_fetchClassOf(argumentArray);
    success(arrayClass == ClassArrayPointer);
    if (get_success()) {
        arrayArgumentCount = OBJMEM_fetchWordLengthOf(argumentArray);
        success(arrayArgumentCount == blockArgumentCount);
    }
    if (get_success()) {
        Interp_Transfer_fromIndex_ofObject_toIndex_ofObject(arrayArgumentCount,
                                                            0, argumentArray,
                                                            TempFrameStart,
                                                            blockContext);
        initialIP = OBJMEM_fetchPointer_ofObject(InitialIPIndex, blockContext);
        OBJMEM_storePointer_ofObject_withValue(InstructionPointerIndex,
                                               blockContext, initialIP);
        storeStackPointerValue_inContext(arrayArgumentCount, blockContext);
        OBJMEM_storePointer_ofObject_withValue(CallerIndex, blockContext,
                                               R_ctx.activeContext);
        newActiveContext(blockContext);
    } else
        unPop(2);
}

void
primitivePerform()
{
    int             performSelector;
    int             newReceiver;
    int             selectorIndex;
    performSelector = R_cls.messageSelector;
    R_cls.messageSelector = stackValue(R_cls.argumentCount - 1);
    newReceiver = stackValue(R_cls.argumentCount);
    lookupMethodInClass(OBJMEM_fetchClassOf(newReceiver));
    success(argumentCountOf(R_cls.newMethod) == R_cls.argumentCount - 1);
    if (get_success()) {
        selectorIndex = R_ctx.stackPointer - R_cls.argumentCount + 1;
        Interp_Transfer_fromIndex_ofObject_toIndex_ofObject(R_cls.
                                                            argumentCount - 1,
                                                            selectorIndex + 1,
                                                            R_ctx.activeContext,
                                                            selectorIndex,
                                                            R_ctx.
                                                            activeContext);
        pop(1);
        R_cls.argumentCount = R_cls.argumentCount - 1;
        executeNewMethod();
    } else {
        R_cls.messageSelector = performSelector;
    }
}

void
primitivePerformWithArgs()
{
    int             thisReceiver;
    int             performSelector;
    int             argumentArray;
    int             arrayClass;
    int             arraySize;
    int             index;

    argumentArray = popStack();
    arraySize = OBJMEM_fetchWordLengthOf(argumentArray);
    arrayClass = OBJMEM_fetchClassOf(argumentArray);
    success((R_ctx.stackPointer + arraySize) <
            OBJMEM_fetchWordLengthOf(R_ctx.activeContext));
    success(arrayClass == ClassArrayPointer);
    if (get_success()) {
        performSelector = R_cls.messageSelector;
        R_cls.messageSelector = popStack();
        thisReceiver = stackTop();
        R_cls.argumentCount = arraySize;
        index = 1;
        while (index <= R_cls.argumentCount) {
            push(OBJMEM_fetchPointer_ofObject(index - 1, argumentArray));
            index = index + 1;
        }
        lookupMethodInClass(OBJMEM_fetchClassOf(thisReceiver));
        success(argumentCountOf(R_cls.newMethod) == R_cls.argumentCount);
        if (get_success())
            executeNewMethod();
        else {
            unPop(R_cls.argumentCount);
            push(R_cls.messageSelector);
            push(argumentArray);
            R_cls.argumentCount = 2;
            R_cls.messageSelector = performSelector;
        }
    } else
        unPop(1);

}

void
asynchronousSignal(int aSemaphore)
{
    R_sch.semaphoreIndex = R_sch.semaphoreIndex + 1;
    if (R_sch.semaphoreIndex ==
        sizeof(R_sch.semaphoreList) / sizeof(R_sch.semaphoreList[0]))
        error("overflow semaphore list");
    R_sch.semaphoreList[R_sch.semaphoreIndex] = aSemaphore;
}

void
synchronousSignal(int aSemaphore)
{
    int             excessSignals;
    if (isEmptyList(aSemaphore)) {
        excessSignals =
            Interp_FetchInteger_ofObject(ExcessSignalsIndex, aSemaphore);
        Interp_StoreInteger_ofObject_withValue(ExcessSignalsIndex, aSemaphore,
                                               excessSignals + 1);
    } else {
        /* removeFirstLinkOfList returns pointer that must be released */
        int             aProcess = removeFirstLinkOfList(aSemaphore);
        resume(aProcess);
        OBJMEM_decreaseReferencesTo(aProcess);
    }
}

void
transferTo(int aProcess)
{
    R_sch.newProcessWaiting = true;
    if (R_sch.newProcess != NilPointer)
        OBJMEM_decreaseReferencesTo(R_sch.newProcess);
    R_sch.newProcess = aProcess;
    OBJMEM_increaseReferencesTo(R_sch.newProcess);
}


void
checkProcessSwitch()
{
    memWarn();                  /* dbanay -- warn about low memory -- once */

    while (R_sch.semaphoreIndex >= 0) {
        synchronousSignal(R_sch.semaphoreList[R_sch.semaphoreIndex]);
        R_sch.semaphoreIndex = R_sch.semaphoreIndex - 1;
    }

    int             theActiveProcess;

    if (R_sch.newProcessWaiting) {
        R_sch.newProcessWaiting = false;
        theActiveProcess = activeProcess();

        OBJMEM_storePointer_ofObject_withValue(SuspendedContextIndex,
                                               theActiveProcess,
                                               R_ctx.activeContext);
        OBJMEM_storePointer_ofObject_withValue(ActiveProcessIndex,
                                               schedulerPointer(),
                                               R_sch.newProcess);
        newActiveContext(OBJMEM_fetchPointer_ofObject
                         (SuspendedContextIndex, R_sch.newProcess));

        OBJMEM_decreaseReferencesTo(R_sch.newProcess);
        R_sch.newProcess = NilPointer;

        /* countp(activeProcess()); */
    }
}

int
activeProcess()
{
    if (R_sch.newProcessWaiting)
        return R_sch.newProcess;
    return OBJMEM_fetchPointer_ofObject(ActiveProcessIndex, schedulerPointer());
}

int
schedulerPointer()
{
    return OBJMEM_fetchPointer_ofObject(ValueIndex,
                                        SchedulerAssociationPointer);
}
int
firstContext()
{
    R_sch.newProcessWaiting = false;
    R_sch.newProcess = NilPointer;
    return OBJMEM_fetchPointer_ofObject(SuspendedContextIndex, activeProcess());
}

int
removeFirstLinkOfList(int aLinkedList)
{
    int             firstLink;
    int             lastLink;
    int             nextLink;
    /*
     * The routines listed here ignore the reference-counting problem in the
     * interest of clarity. (pg. 644 G&R). Found and fixed. -dbanay
     */
    firstLink = OBJMEM_fetchPointer_ofObject(FirstLinkIndex, aLinkedList);
    /* When the link list is updated the references to fistLink */
    /* will drop to zero. So, we need to return an increased pointer that */
    /* must be decreased by the caller. */
    /* Calls: */
    /* suspendActive->wakeHighestPriority->removeFirstLinkOfList */
    /* synchronousSignal->removeFirstLinkOfList */
    OBJMEM_increaseReferencesTo(firstLink);

    lastLink = OBJMEM_fetchPointer_ofObject(LastLinkIndex, aLinkedList);
    if (lastLink == firstLink) {
        OBJMEM_storePointer_ofObject_withValue(FirstLinkIndex, aLinkedList,
                                               NilPointer);
        OBJMEM_storePointer_ofObject_withValue(LastLinkIndex, aLinkedList,
                                               NilPointer);
    } else {
        nextLink = OBJMEM_fetchPointer_ofObject(NextLinkIndex, firstLink);
        OBJMEM_storePointer_ofObject_withValue(FirstLinkIndex, aLinkedList,
                                               nextLink);
    }
    OBJMEM_storePointer_ofObject_withValue(NextLinkIndex, firstLink,
                                           NilPointer);
    return firstLink;
}

void
addLastLink_toList(int aLink, int aLinkedList)
{
    int             lastLink;
    if (isEmptyList(aLinkedList)) {
        OBJMEM_storePointer_ofObject_withValue(FirstLinkIndex, aLinkedList,
                                               aLink);
    } else {
        lastLink = OBJMEM_fetchPointer_ofObject(LastLinkIndex, aLinkedList);
        OBJMEM_storePointer_ofObject_withValue(NextLinkIndex, lastLink, aLink);
    }
    OBJMEM_storePointer_ofObject_withValue(LastLinkIndex, aLinkedList, aLink);
    OBJMEM_storePointer_ofObject_withValue(MyListIndex, aLink, aLinkedList);
}

int
isEmptyList(int aLinkedList)
{
    return OBJMEM_fetchPointer_ofObject(FirstLinkIndex,
                                        aLinkedList) == NilPointer;
}

int
wakeHighestPriority()
{
    int             priority;
    int             processLists;
    int             processList;
    processLists =
        OBJMEM_fetchPointer_ofObject(ProcessListsIndex, schedulerPointer());
    priority = OBJMEM_fetchWordLengthOf(processLists);
    for (;;) {
        assert(priority > 0);
        processList = OBJMEM_fetchPointer_ofObject(priority - 1, processLists);
        if (!isEmptyList(processList))
            break;
        priority = priority - 1;
    }
    int             proc = removeFirstLinkOfList(processList);
    return proc;
}

void
sleep(int aProcess)
{
    int             priority;
    int             processLists;
    int             processList;
    priority = Interp_FetchInteger_ofObject(PriorityIndex, aProcess);
    processLists = OBJMEM_fetchPointer_ofObject(ProcessListsIndex,
                                                schedulerPointer());
    processList = OBJMEM_fetchPointer_ofObject(priority - 1, processLists);
    addLastLink_toList(aProcess, processList);
}

/* NOTE: Returns a referenced increased pointer that must be decreased by */
/* caller */
void
suspendActive()
{
    /*
     * "source" "dbanay: ref counting issue - wakeHighestPriority" aProcess =
     * self wakeHighestPriority. self transferTo: self wakeHighestPriority.
     * memory decreaseReferencesTo: aProcess
     */
    /* Note: wakeHighestPriority returns a pointer that must be decreased by */
    /* caller */
    int             aProcess = wakeHighestPriority();
    transferTo(aProcess);
    OBJMEM_decreaseReferencesTo(aProcess);
}

void
resume(int aProcess)
{
    int             theActiveProcess;
    int             activePriority;
    int             newPriority;
    theActiveProcess = activeProcess();
    activePriority =
        Interp_FetchInteger_ofObject(PriorityIndex, theActiveProcess);
    newPriority = Interp_FetchInteger_ofObject(PriorityIndex, aProcess);

    if (newPriority > activePriority) {
        sleep(theActiveProcess);
        transferTo(aProcess);
    } else {
        sleep(aProcess);
    }
}

void
primitiveSignal()
{
    synchronousSignal(stackTop());
}

void
primitiveWait()
{
    int             thisReceiver;
    int             excessSignals;
    thisReceiver = stackTop();
    excessSignals =
        Interp_FetchInteger_ofObject(ExcessSignalsIndex, thisReceiver);
    if (excessSignals > 0) {
        Interp_StoreInteger_ofObject_withValue(ExcessSignalsIndex, thisReceiver,
                                               excessSignals - 1);
    } else {
        addLastLink_toList(activeProcess(), thisReceiver);
        suspendActive();
    }
}

void
primitiveResume()
{
    resume(stackTop());
}

void
primitiveSuspend()
{
    success(stackTop() == activeProcess());
    if (get_success()) {
        popStack();
        push(NilPointer);
        suspendActive();
    }
}

void
primitiveFlushCache()
{
    initializeMethodCache();
}


/*
 * --------------------- prim_InputOutput.inc.h -----------------------------
 */
void
updateDisplay(int destForm, int updatedX, int updatedY, int updatedWidth,
              int updatedHeight)
{
    int             width =
        Interp_FetchInteger_ofObject(WidthInForm, G_currentDisplay);
    int             height =
        Interp_FetchInteger_ofObject(HeightInForm, G_currentDisplay);
    if (G_currentDisplayWidth != width || G_currentDisplayHeight != height) {
        /* Display bits changed... possibly due to screen size change... */
        if (height <= 100)
            return;             /* See note in primitiveBeDisplay... */
        System_SetDisplaySize(width, height);
        G_currentDisplayWidth = width;
        G_currentDisplayHeight = height;
    }
    if (updatedWidth > 0 && updatedHeight > 0)
        System_DisplayChanged(updatedX, updatedY, updatedWidth, updatedHeight);
}

void
primitiveMousePoint()
{
    int             mouseX,
                    mouseY;
    pop(1);                     /* remove receiver */
    System_GetCursorLocation(&mouseX, &mouseY);
    int             pointResult =
        OBJMEM_instantiateClass_withPointers(ClassPointPointer,
                                             ClassPointSize);
    Interp_StoreInteger_ofObject_withValue(XIndex, pointResult, mouseX);
    Interp_StoreInteger_ofObject_withValue(YIndex, pointResult, mouseY);
    push(pointResult);
}

void
primitiveCursorLocPut()
{
    int             point = popStack();
    success(OBJMEM_fetchClassOf(point) == ClassPointPointer);
    if (get_success()) {
        int             x = Interp_FetchInteger_ofObject(XIndex, point);
        int             y = Interp_FetchInteger_ofObject(YIndex, point);
        System_SetCursorLocation(x, y);
    } else
        unPop(1);
}

void
primitiveCursorLink()
{
    int             flag = popStack();
    System_SetLinkCursor(flag == TruePointer);
}

void
primitiveInputSemaphore()
{
    int             semaphore = popStack();
    success(semaphore == NilPointer
            || OBJMEM_fetchClassOf(semaphore) == ClassSemaphorePointer);
    if (get_success())
        System_SetInputSemaphore(semaphore);
    else
        unPop(1);
}

void
primitiveSampleInterval()
{
    primitiveFail();

}

void
primitiveInputWord()
{
    uint16_t        word;
    bool            result = System_NextInputQueueWord(&word);
    if (result) {
        pop(1);                 /* remove receiver */
        push(positive16BitIntegerFor(word));
    } else {
        primitiveFail();
    }
}

void
primitiveCopyBits()
{
    int             bitBltPointer = stackTop();
    int             destForm =
        OBJMEM_fetchPointer_ofObject(DestFormIndex, bitBltPointer);
    int             sourceForm =
        OBJMEM_fetchPointer_ofObject(SourceFormIndex, bitBltPointer);
    int             destX =
        Interp_FetchInteger_ofObject(DestXIndex, bitBltPointer);
    int             destY =
        Interp_FetchInteger_ofObject(DestYIndex, bitBltPointer);
    int             clipX =
        Interp_FetchInteger_ofObject(ClipXIndex, bitBltPointer);
    int             clipY =
        Interp_FetchInteger_ofObject(ClipYIndex, bitBltPointer);
    int             clipWidth =
        Interp_FetchInteger_ofObject(ClipWidthIndex, bitBltPointer);
    int             clipHeight =
        Interp_FetchInteger_ofObject(ClipHeightIndex, bitBltPointer);
    int             sourceX =
        Interp_FetchInteger_ofObject(SourceXIndex, bitBltPointer);
    int             sourceY =
        Interp_FetchInteger_ofObject(SourceYIndex, bitBltPointer);
    int             width =
        Interp_FetchInteger_ofObject(WidthIndex, bitBltPointer);
    int             height =
        Interp_FetchInteger_ofObject(HeightIndex, bitBltPointer);
    int             rule =
        Interp_FetchInteger_ofObject(CombinationRuleIndex, bitBltPointer);
    success(between_and(rule, 0, 15));
    if (get_success()) {
        BB_init(destForm, sourceForm,
                OBJMEM_fetchPointer_ofObject(HalftoneFormIndex, bitBltPointer),
                rule, destX, destY, width, height, sourceX, sourceY, clipX,
                clipY, clipWidth, clipHeight);
        int             updatedX,
                        updatedY,
                        updatedWidth,
                        updatedHeight;
        BB_copyBits();
        if (destForm == G_currentDisplay) {
            BB_getUpdatedBounds(&updatedX, &updatedY, &updatedWidth,
                                &updatedHeight);
            if (updatedWidth > 0 && updatedHeight > 0)
                updateDisplay(destForm, updatedX, updatedY, updatedWidth,
                              updatedHeight);
        }
        BB_free();
    }
}

void
primitiveSnapshot()
{

    /*
     * The primitiveSnapshot routine writes the current state of the object
     * memory on a file of the same format as the Smalltalk-80 release file.
     * This file can be resumed in exactly the same way that the release file
     * was originally started. Note that the pointer of the active context at
     * the time of the primitive call must be stored in the active Process on
     * the file. (G&R) pg 651.
     */
    int             activeProcess =
        OBJMEM_fetchPointer_ofObject(ActiveProcessIndex,
                                     schedulerPointer());
    OBJMEM_storePointer_ofObject_withValue(SuspendedContextIndex,
                                           activeProcess, R_ctx.activeContext);
    storeContextRegisters();
    OBJMEM_garbageCollect();
    OBJMEM_saveSnapshot(System_GetImageName());


    /*
     * This is poorly documented by the Bluebook. There is an actual return
     * value that is important. see snapshotAs:thenQuit: in the Smalltalk
     * sources. When the system resumes a snapshot the interpreter will be back
     * at the point of the save, and 'self' will be at the top of the stack.
     * But after saving we return nil to tell the caller that we just wrote a
     * snapshot. This is how we can distinguish the case were we save a session
     * (to be continued potentially followed by an exit) or we are resuming a
     * saved session.
     */
    pop(1);                     /* DO NOT POP BEFORE WRITING IMAGE. */

    push(NilPointer);           /* return of nil signals we just saved */
}

void
primitiveTimeWordsInto()
{
    uint32_t        time = System_GetSmalltalkEpochTime();
    int             byteIndexObject = popStack();
    OBJMEM_storeByte_ofObject_withValue(0, byteIndexObject, time & 0xff);
    OBJMEM_storeByte_ofObject_withValue(1, byteIndexObject, (time >> 8) & 0xff);
    OBJMEM_storeByte_ofObject_withValue(2, byteIndexObject,
                                        (time >> 16) & 0xff);
    OBJMEM_storeByte_ofObject_withValue(3, byteIndexObject,
                                        (time >> 24) & 0xff);
}

void
primitiveTickWordsInto()
{
    uint32_t        time = System_GetMsClock();
    int             byteIndexObject = popStack();
    OBJMEM_storeByte_ofObject_withValue(0, byteIndexObject, time & 0xff);
    OBJMEM_storeByte_ofObject_withValue(1, byteIndexObject, (time >> 8) & 0xff);
    OBJMEM_storeByte_ofObject_withValue(2, byteIndexObject,
                                        (time >> 16) & 0xff);
    OBJMEM_storeByte_ofObject_withValue(3, byteIndexObject,
                                        (time >> 24) & 0xff);
}

void
primitiveSignalAtTick()
{
    fprintf(stderr, "primitiveSignalAtTick()\n");
    /* prt("signalat"); */
    /*
     * The primitiveSignalAtTick routine is associated with the signal:atTick:
     * messages in ProcessorScheduler. his message takes a Semaphore as the
     * first argument and a byte indexable object of at least four bytes as the
     * second argument. The first four bytes of the second argument are
     * interpreted as an unsigned 32-bit integer of the type stored by the
     * primitiveTickWordsInto routine. The interpreter should signal the
     * Semaphore argument when the millisecond clock reaches the value
     * specified by the second argument. If the specified time has passed,the
     * Semaphore is signaled immediately. This primitive signals the last
     * Semaphore to be passed to it. If a new call is made on it before the
     * last timer value has been reached the last Semaphore will not be
     * signaled. If the first argument is not a Semaphore any currently waiting
     * Semaphore will be forgotten. (pg. 652 G&R)
     *
     * Signal the semaphore when the millisecond clock reaches the value of the
     * second argument.  The second argument is a byte indexable object at
     * least four bytes long (a 32-bit unsigned number with the low order
     * 8-bits stored in the byte with the lowest index).  Fail if the first
     * argument is neither a Semaphore nor nil.
     */
    int             timePointer = popStack();
    int             semaphore = popStack();
    success(semaphore == NilPointer
            || OBJMEM_fetchClassOf(semaphore) == ClassSemaphorePointer);
    if (get_success()) {
        if (semaphore == NilPointer)
            semaphore = 0;      /* Tells HAL to cancel outstanding request */
        uint32_t        time = OBJMEM_fetchByte_ofObject(0, timePointer)
            | (OBJMEM_fetchByte_ofObject(1, timePointer) << 8)
            | (OBJMEM_fetchByte_ofObject(2, timePointer) << 16)
            | (OBJMEM_fetchByte_ofObject(3, timePointer) << 24);
        System_SignalSemaphoreAt(semaphore, time);
    } else
        unPop(2);
}

void
primitiveBeCursor()
{
    int             cursorBitmap;
    int             i;
    uint16_t        bits[16];

    if (G_currentCursor != stackTop()) {
        G_currentCursor = stackTop();
        cursorBitmap =
            OBJMEM_fetchPointer_ofObject(BitsInForm, G_currentCursor);
        for (i = 0; i < 16; i++) {
            bits[i] = OBJMEM_fetchWord_ofObject(i, cursorBitmap);
        }
        System_SetCursorImage(bits);
    }

}

void
primitiveBeDisplay()
{
    int             newDisplay = stackTop();
    int             width;
    int             height;
    if (G_currentDisplay != newDisplay) {
        width =
            Interp_FetchInteger_ofObject(WidthInForm, newDisplay);
        height =
            Interp_FetchInteger_ofObject(HeightInForm, newDisplay);
        if (height > 100) {
            if (!System_SetDisplaySize(width, height)) {
                primitiveFail();
            }
            G_currentDisplay = newDisplay;
            G_currentDisplayWidth = width;
            G_currentDisplayHeight = height;
        } else
            G_currentDisplay = 0;
    }
}

void
primitiveScanCharacters()
{
#ifdef IMPLEMENT_PRIMITIVE_SCANCHARS
    bool            displaying = popStack() == TruePointer;
    int             stops = popStack();
    int             rightX = popInteger();
    int             sourceString = popStack();
    int             stopIndex = popInteger();
    int             startIndex = popInteger();
    int             scannerPointer = popStack();
    int             destForm =
        OBJMEM_fetchPointer_ofObject(DestFormIndex, scannerPointer);
    int             sourceForm = OBJMEM_fetchPointer_ofObject(SourceFormIndex,
                                                              scannerPointer);
    int             destX =
        Interp_FetchInteger_ofObject(DestXIndex, scannerPointer);
    int             destY;
    int             clipX;
    int             clipY;
    int             clipWidth;
    int             clipHeight;
    int             sourceX = 0;        /* uninitialized... assigned during */
    /* scanning */
    int             sourceY;
    int             rule;
    int             width = 0;
    int             height;
    int             xTable =
        OBJMEM_fetchPointer_ofObject(XTableIndexIndex, scannerPointer);
    int             lastIndex =
        Interp_FetchInteger_ofObject(LastIndexIndex, scannerPointer);
    int             stopConditions =
        OBJMEM_fetchPointer_ofObject(StopConditionsIndex,
                                     scannerPointer);
    /* If not displaying, some of the fields are nil, so only */
    /* fetch integers that are actually used... */
    if (displaying) {
        destY = Interp_FetchInteger_ofObject(DestYIndex, scannerPointer);
        clipX = Interp_FetchInteger_ofObject(ClipXIndex, scannerPointer);
        clipY = Interp_FetchInteger_ofObject(ClipYIndex, scannerPointer);
        clipWidth =
            Interp_FetchInteger_ofObject(ClipWidthIndex, scannerPointer);
        clipHeight =
            Interp_FetchInteger_ofObject(ClipHeightIndex, scannerPointer);
        sourceY = Interp_FetchInteger_ofObject(SourceYIndex, scannerPointer);
        rule =
            Interp_FetchInteger_ofObject(CombinationRuleIndex, scannerPointer);
        height = Interp_FetchInteger_ofObject(HeightIndex, scannerPointer);
    } else {
        destY = 0;
        clipX = 0;
        clipY = 0;
        clipWidth = 0;
        clipHeight = 0;
        sourceY = 0;
        rule = 0;
        height = 0;
    }

    if (get_success()) {
        CHSCAN_init(destForm, sourceForm,
                    OBJMEM_fetchPointer_ofObject(HalftoneFormIndex,
                                                 scannerPointer), rule, destX,
                    destY, width, height, sourceX, sourceY, clipX, clipY,
                    clipWidth, clipHeight, xTable, lastIndex, stopConditions);
        int             result =
            CHSCAN_scanCharactersFrom_to_in_rightX_stopConditions_displaying
            (startIndex, stopIndex, sourceString, rightX, stops, displaying);
        /*
         * Need to pull out the following modified values and store them back
         * into the scanner: destX, width, sourceX in BitBlt fields lastIndex
         * in CharacterScanner
         */
        Interp_StoreInteger_ofObject_withValue(DestXIndex, scannerPointer,
                                               CHSCAN_getUpdateDestX());
        Interp_StoreInteger_ofObject_withValue(WidthIndex, scannerPointer,
                                               CHSCAN_getUpdatedWidth());
        Interp_StoreInteger_ofObject_withValue(SourceXIndex, scannerPointer,
                                               CHSCAN_getUpdatedSourceX());
        Interp_StoreInteger_ofObject_withValue(LastIndexIndex, scannerPointer,
                                               CHSCAN_getUpdatedLastIndex());
        if (destForm == G_currentDisplay && displaying) {
            updateDisplay(destForm, clipX, clipY, clipWidth, clipHeight);
        }
        CHSCAN_free();
        push(result);
    } else {
        unPop(7);
    }
#else
    primitiveFail();
#endif
}

void
primitiveDrawLoop()
{
    /* optional */
    primitiveFail();
}

void
primitiveStringReplace()
{
    /* optional */
    primitiveFail();
}

/*
 * --------------------- prim_ArrayAndStream.inc.h
 * -----------------------------
 */
void
checkIndexableBoundsOf_in(int index, int array)
{
    success(index >= 1);
    success(index <= lengthOf(array));
}

int
lengthOf(int array)
{
    return
        isWords(OBJMEM_fetchClassOf(array)) ?
        OBJMEM_fetchWordLengthOf(array) : OBJMEM_fetchByteLengthOf(array);
}

int
subscript_with(int array, int index)
{
    int             cls;
    int             value;
    cls = OBJMEM_fetchClassOf(array);
    if (isWords(cls)) {
        if (isPointers(cls)) {
            return OBJMEM_fetchPointer_ofObject(index - 1, array);
        } else {
            value = OBJMEM_fetchWord_ofObject(index - 1, array);
            return positive16BitIntegerFor(value);
        }
    }
    value = OBJMEM_fetchByte_ofObject(index - 1, array);
    return OBJMEM_integerObjectOf(value);
}

void
subscript_with_storing(int array, int index, int value)
{
    int             cls;
    cls = OBJMEM_fetchClassOf(array);
    if (isWords(cls)) {
        if (isPointers(cls)) {
            OBJMEM_storePointer_ofObject_withValue(index - 1, array, value);
        } else {
            success(OBJMEM_isIntegerObject(value)
                    || OBJMEM_fetchClassOf(value)
                    == ClassLargePositiveIntegerPointer);
            if (get_success()) {
                OBJMEM_storeWord_ofObject_withValue(index - 1, array,
                                                    positive16BitValueOf
                                                    (value));
            }
        }
    } else {
        success(OBJMEM_isIntegerObject(value));
        if (get_success()) {
            OBJMEM_storeByte_ofObject_withValue(index - 1, array,
                                                Interp_LowByteOf
                                                (OBJMEM_integerValueOf(value)));
        }
    }
}

void
primitiveAt()
{
    int             index;
    int             array;
    int             arrayClass;
    int             result = 0;
    index = positive16BitValueOf(popStack());
    array = popStack();
    arrayClass = OBJMEM_fetchClassOf(array);
    checkIndexableBoundsOf_in(index, array);
    if (get_success()) {
        index = index + fixedFieldsOf(arrayClass);
        result = subscript_with(array, index);
    }
    if (get_success()) {
        push(result);
    } else
        unPop(2);
}

void
primitiveAtPut()
{
    int             array;
    int             index;
    int             arrayClass;
    int             value;
    value = popStack();
    index = positive16BitValueOf(popStack());
    array = popStack();
    arrayClass = OBJMEM_fetchClassOf(array);
    checkIndexableBoundsOf_in(index, array);
    if (get_success()) {
        index = index + fixedFieldsOf(arrayClass);
        subscript_with_storing(array, index, value);
    }
    if (get_success())
        push(value);
    else
        unPop(3);
}

void
primitiveSize()
{
    int             array;
    int             cls;
    int             length = 0;
    array = popStack();
    success(!OBJMEM_isIntegerObject(array));
    if (get_success()) {
        cls = OBJMEM_fetchClassOf(array);
        length = positive16BitIntegerFor(lengthOf(array) - fixedFieldsOf(cls));
    }
    if (get_success())
        push(length);
    else
        unPop(1);
}

void
primitiveStringAt()
{
    int             index;
    int             array;
    int             ascii;
    int             character = 0;
    index = positive16BitValueOf(popStack());
    array = popStack();
    checkIndexableBoundsOf_in(index, array);
    if (get_success()) {
        ascii = OBJMEM_integerValueOf(subscript_with(array, index));
        character = OBJMEM_fetchPointer_ofObject(ascii, CharacterTablePointer);
    }
    if (get_success()) {
        push(character);
    } else
        unPop(2);
}

void
primitiveStringAtPut()
{
    int             index;
    int             array;
    int             ascii;
    int             character;
    character = popStack();
    index = positive16BitValueOf(popStack());
    array = popStack();
    checkIndexableBoundsOf_in(index, array);
    success(OBJMEM_fetchClassOf(character) == ClassCharacterPointer);
    if (get_success()) {
        ascii = OBJMEM_fetchPointer_ofObject(CharacterValueIndex, character);
        subscript_with_storing(array, index, ascii);
    }
    if (get_success())
        push(character);
    else
        unPop(3);
}

void
primitiveNext()
{
#ifndef IMPLEMENT_PRIMITIVE_NEXT
    primitiveFail();
#else
    int             stream;
    int             index;
    int             limit;
    int             array;
    int             arrayClass;
    int             result = 0;
    int             ascii;
    stream = popStack();
    array = OBJMEM_fetchPointer_ofObject(StreamArrayIndex, stream);
    arrayClass = OBJMEM_fetchClassOf(array);
    index = Interp_FetchInteger_ofObject(StreamIndexIndex, stream);
    limit = Interp_FetchInteger_ofObject(StreamReadLimitIndex, stream);
    success(index < limit);
    success(arrayClass == ClassArrayPointer ||
            arrayClass == ClassStringPointer);
    checkIndexableBoundsOf_in(index + 1, array);
    if (get_success()) {
        index = index + 1;
        result = subscript_with(array, index);
    }
    if (get_success()) {
        Interp_StoreInteger_ofObject_withValue(StreamIndexIndex, stream, index);
    }
    if (get_success()) {
        if (arrayClass == ClassArrayPointer) {
            push(result);
        } else {
            ascii = OBJMEM_integerValueOf(result);
            push(OBJMEM_fetchPointer_ofObject(ascii, CharacterTablePointer));
        }
    } else
        unPop(1);
#endif
}

void
primitiveNextPut()
{
#ifndef IMPLEMENT_PRIMITIVE_NEXT_PUT
    primitiveFail();

#else
    int             value;
    int             stream;
    int             index;
    int             limit;
    int             array;
    int             arrayClass;
    int             ascii;
    value = popStack();
    stream = popStack();
    array = OBJMEM_fetchPointer_ofObject(StreamArrayIndex, stream);
    arrayClass = OBJMEM_fetchClassOf(array);
    index = Interp_FetchInteger_ofObject(StreamIndexIndex, stream);
    limit = Interp_FetchInteger_ofObject(StreamWriteLimitIndex, stream);
    success(index < limit);
    success(arrayClass == ClassArrayPointer ||
            arrayClass == ClassStringPointer);
    checkIndexableBoundsOf_in(index + 1, array);
    if (get_success()) {
        index = index + 1;
        if (arrayClass == ClassArrayPointer) {
            subscript_with_storing(array, index, value);
        } else {
            ascii = OBJMEM_fetchPointer_ofObject(CharacterValueIndex, value);
            subscript_with_storing(array, index, ascii);
        }
    }
    if (get_success())
        Interp_StoreInteger_ofObject_withValue(StreamIndexIndex, stream, index);
    if (get_success())
        push(value);
    else
        unPop(2);
#endif
}

/* primitiveAtEnd */
void
primitiveAtEnd()
{
#ifndef IMPLEMENT_PRIMITIVE_AT_END
    primitiveFail();
#else
    int             stream;
    int             array;
    int             arrayClass;
    int             length;
    int             index;
    int             limit;
    stream = popStack();
    array = OBJMEM_fetchPointer_ofObject(StreamArrayIndex, stream);
    arrayClass = OBJMEM_fetchClassOf(array);
    length = lengthOf(array);
    index = Interp_FetchInteger_ofObject(StreamIndexIndex, stream);
    limit = Interp_FetchInteger_ofObject(StreamReadLimitIndex, stream);
    success(arrayClass == ClassArrayPointer ||
            arrayClass == ClassStringPointer);
    if (get_success()) {
        if (index >= limit || index >= length)
            push(TruePointer);
        else
            push(FalsePointer);
    } else {
        unPop(1);
    }
#endif
}

/*
 * --------------------- prim_FloatArithmetic.inc.h
 * -----------------------------
 */
void
pushFloat(float f)
{
    uint32_t        uint32;
    memcpy(&uint32, &f, sizeof(f));
    int             objectPointer =
        OBJMEM_instantiateClass_withWords(ClassFloatPointer, 2);
    OBJMEM_storeWord_ofObject_withValue(0, objectPointer, uint32 & 0xffff);
    OBJMEM_storeWord_ofObject_withValue(1, objectPointer, uint32 >> 16);
    push(objectPointer);
}

float
popFloat()
{
    int             objectPointer = popStack();
    success(OBJMEM_fetchClassOf(objectPointer) == ClassFloatPointer);
    if (get_success()) {
        return extractFloat(objectPointer);
    }
    return nanf("");
}

/* primitiveDivide */
/* The primitive routine for division (associated with the selector/) is */
/* different than */
/* the other three arithmetic primitives since it only produces a result if the */
/* division */
/* is exact, otherwise the primitive fails. */
/* The primitiveAsFloat routine converts its SmallInteger receiver into a */
/* Float. */
void
primitiveAsFloat()
{
    int             integerArgument = popInteger();
    if (get_success()) {
        pushFloat((float) integerArgument);
    } else
        unPop(1);
}

void
primitiveFloatAdd()
{
    float           floatReceiver;
    float           floatArgument;
    float           floatResult;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        floatResult = floatReceiver + floatArgument;
        pushFloat(floatResult);
    } else
        unPop(2);
}

void
primitiveFloatSubtract()
{
    float           floatReceiver;
    float           floatArgument;
    float           floatResult;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        floatResult = floatReceiver - floatArgument;
        pushFloat(floatResult);
    } else
        unPop(2);
}

void
primitiveFloatLessThan()
{
    float           floatReceiver;
    float           floatArgument;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        if (floatReceiver < floatArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveFloatGreaterThan()
{
    float           floatReceiver;
    float           floatArgument;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        if (floatReceiver > floatArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveFloatLessOrEqual()
{
    float           floatReceiver;
    float           floatArgument;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        if (floatReceiver <= floatArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveFloatGreaterOrEqual()
{
    float           floatReceiver;
    float           floatArgument;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        if (floatReceiver >= floatArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveFloatEqual()
{
    float           floatReceiver;
    float           floatArgument;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        if (floatReceiver == floatArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveFloatNotEqual()
{
    float           floatReceiver;
    float           floatArgument;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        if (floatReceiver != floatArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveFloatMultiply()
{
    float           floatReceiver;
    float           floatArgument;
    float           floatResult;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    if (get_success()) {
        floatResult = floatReceiver * floatArgument;
        pushFloat(floatResult);
    } else
        unPop(2);
}

void
primitiveFloatDivide()
{
    float           floatReceiver;
    float           floatArgument;
    float           floatResult;
    floatArgument = popFloat();
    floatReceiver = popFloat();
    success(floatArgument != 0);
    if (get_success()) {
        floatResult = floatReceiver / floatArgument;
        pushFloat(floatResult);
    } else
        unPop(2);
}

void
primitiveTruncated()
{
    float           floatReceiver;
    int             integerResult = 0;
    floatReceiver = popFloat();
    if (get_success()) {
        integerResult = (int) floatReceiver;
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success()) {
        pushInteger(integerResult);
    } else
        unPop(1);
}

void
primitiveFractionalPart()
{
    float           floatReceiver;
    float           floatResult;
    floatReceiver = popFloat();
    if (get_success()) {
        floatResult = floatReceiver - (int) floatReceiver;
        pushFloat(floatResult);
    } else
        unPop(1);
}

void
primitiveExponent()
{
    primitiveFail();            /* optional */
}

void
primitiveTimesTwoPower()
{
    primitiveFail();            /* optional */
}

/*
 * --------------------- prim_IntegerArithmetic.inc.h
 * -----------------------------
 */
void
primitiveAdd()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        integerResult = integerReceiver + integerArgument;
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        pushInteger(integerResult);
    else
        unPop(2);
}

void
primitiveSubtract()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        integerResult = integerReceiver - integerArgument;
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        pushInteger(integerResult);
    else
        unPop(2);
}

void
primitiveLessThan()
{
    int             integerReceiver;
    int             integerArgument;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        if (integerReceiver < integerArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);

}

void
primitiveGreaterThan()
{
    int             integerReceiver;
    int             integerArgument;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        if (integerReceiver > integerArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveLessOrEqual()
{
    int             integerReceiver;
    int             integerArgument;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        if (integerReceiver <= integerArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveGreaterOrEqual()
{
    int             integerReceiver;
    int             integerArgument;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        if (integerReceiver >= integerArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveEqual()
{
    int             integerReceiver;
    int             integerArgument;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        if (integerReceiver == integerArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else {
        unPop(2);
    }
}

void
primitiveNotEqual()
{
    int             integerReceiver;
    int             integerArgument;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        if (integerReceiver != integerArgument)
            push(TruePointer);
        else
            push(FalsePointer);
    } else
        unPop(2);
}

void
primitiveMultiply()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        integerResult = integerReceiver * integerArgument;
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        pushInteger(integerResult);
    else
        unPop(2);
}

void
primitiveDivide()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;  /* eliminate warning */
    /*
     * "source" integerArgument <- self popInteger. integerReceiver <- self
     * popInteger. self success: integerArgument ~= 0. self success:
     * integerReceiver \\ integerArgument = 0. self success ifTrue:
     * [integerResult <- integerReceiver // integerArgument. self success:
     * (memory isIntegerValue: integerResult)]. self success ifTrue: [ self
     * push: (memory integerObjectOf: integerResult)] ifFalse: [self unPop: 2]
     */
    integerArgument = popInteger();
    integerReceiver = popInteger();
    success(integerArgument != 0);
    success(integerReceiver % integerArgument == 0);
    if (get_success()) {
        integerResult = integerReceiver / integerArgument;
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        push(OBJMEM_integerObjectOf(integerResult));
    else
        unPop(2);
}

void
primitiveMod()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;  /* silence warning */
    integerArgument = popInteger();
    integerReceiver = popInteger();
    success(integerArgument != 0);
    if (get_success()) {
        integerResult = integerReceiver % integerArgument;
        if (integerArgument < 0) {
            if (integerResult > 0) {
                integerResult += integerArgument;
            }
        } else {
            if (integerResult < 0) {
                integerResult += integerArgument;
            }
        }
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        pushInteger(integerResult);
    else
        unPop(2);
}

void
primitiveDiv()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    success(integerArgument != 0);
    if (get_success()) {
        integerResult = integerReceiver / integerArgument;
        if (integerReceiver % integerArgument != 0) {
            /* Smalltalk truncates to negative infinity ( -7 // 2 == -4 not -3) */
            if (integerResult < 0)
                integerResult--;
        }
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        push(OBJMEM_integerObjectOf(integerResult));
    else
        unPop(2);
}

void
primitiveQuo()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    success(integerArgument != 0);
    if (get_success()) {
        integerResult = integerReceiver / integerArgument;
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        push(OBJMEM_integerObjectOf(integerResult));
    else
        unPop(2);
}

void
primitiveBitAnd()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        integerResult = integerReceiver & integerArgument;
        pushInteger(integerResult);
    } else
        unPop(2);
}

void
primitiveBitOr()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        integerResult = integerReceiver | integerArgument;
        pushInteger(integerResult);
    } else
        unPop(2);
}

void
primitiveBitXor()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        integerResult = integerReceiver ^ integerArgument;
        pushInteger(integerResult);
    } else
        unPop(2);
}

void
primitiveBitShift()
{
    int             integerReceiver;
    int             integerArgument;
    int             integerResult = 0;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    if (get_success()) {
        integerResult =
            integerArgument >= 0 ?
            integerReceiver << integerArgument :
            integerReceiver >> -integerArgument;
        success(OBJMEM_isIntegerValue(integerResult));
    }
    if (get_success())
        pushInteger(integerResult);
    else
        unPop(2);
}

void
primitiveMakePoint()
{
    int             integerReceiver;
    int             integerArgument;
    int             pointResult;
    integerArgument = popInteger();
    integerReceiver = popInteger();
    success(OBJMEM_isIntegerValue(integerReceiver));
    success(OBJMEM_isIntegerValue(integerArgument));
    if (get_success()) {
        pointResult = OBJMEM_instantiateClass_withPointers(ClassPointPointer,
                                                           ClassPointSize);
        Interp_StoreInteger_ofObject_withValue(XIndex, pointResult,
                                               integerReceiver);
        Interp_StoreInteger_ofObject_withValue(YIndex, pointResult,
                                               integerArgument);
        push(pointResult);
    } else
        unPop(2);
}

/*
 * --------------------- prim_LargeIntegerArithmetic.inc.h
 * -----------------------------
 */


/*
 * --------------------- prim_Private.inc.h -----------------------------
 */

void
primitiveBeSnapshotFile()
{
    int             fileObjectPointer = stackTop();
    int             fileNamePointer =
        OBJMEM_fetchPointer_ofObject(FileNameIndex,
                                     fileObjectPointer);        /* fileName */
    /* field */
    char           *fileName = Interp_StringFromObject(fileNamePointer);
    System_SetImageName(fileName);
    free(fileName);
}

/*
 * ----- Seb -----
 */
void
primitiveStdinInputSemaphore()
{
    int             semaphore = popStack();
    success(semaphore == NilPointer
            || OBJMEM_fetchClassOf(semaphore) == ClassSemaphorePointer);
    if (get_success())
        System_SetStdinInputSemaphore(semaphore);
    else
        unPop(1);
}

void
primitiveLoadedImageName()
{
    char           *name = strdup(System_GetImageName());
    int             suffix_index = strlen(name) - 3;
    name[suffix_index] = '\0';
    int             str = Interp_StringObjectFor(name);
    free(name);
    push(str);
}

void
primitiveImageArgv()
{
    int             rep;
    if (G_Options.imgArgv)
        rep = Interp_StringObjectFor(G_Options.imgArgv);
    else
        rep = NilPointer;
    push(rep);
}

void
primitiveStdinInputString()
{
    int             str = Interp_StringObjectFor(System_GetStdinLine());
    push(str);
}

void
primitiveShowStdout()
{
    int             stringArg = popStack();
    success(OBJMEM_fetchClassOf(stringArg) == ClassStringPointer);
    if (get_success()) {
        char           *str = Interp_StringFromObject(stringArg);
        fprintf(stderr, "%s\n", str);
        free(str);
    }
}
/*
 * ----- Seb -----
 */

#define PageSize 512
void
primitivePosixFileOperation()
{
    /* command id, name, page, file */
    /*
     * can get file descriptor from file ('fd' field) ID Return Remarks ----
     * ------- ---------- 0 true/false read page (stores size in bytesInPage) 1
     * true/false write page (uses bytesInPage) 2 true/false truncate after
     * page or entire file if page is nil 3 size of file/nil file size 4
     * true/false open file -- sets fd in file 5 true/false close file -- sets
     * fd to -1
     */
    int             page = popStack();
    int             name = popStack();
    int             code = popInteger();
    int             file = popStack();
    const int       DescriptorIndex = 8;        /* fd field of PosixFile */
    const int       PageNumberIndex = 3;        /* pageNumber field in */
    /* PosixFilePage */

    const int       PageInPageIndex = 1;        /* ByteArray contents in */
    /* PosixFilePage */
    const int       BytesInPageIndex = 4;       /* bytesInPage field in */
    /* PosixFilePage */

    /* const int PageSize = 512; // MUST match page size of PosixFilePage */

    static uint8_t  pageBuffer[PageSize];

    /* Code must be legit */
    success(code >= 0 && code <= 6);
    success(file != NilPointer);

    if (get_success()) {
        switch (code) {
        case 0:                /* Read page */
            {
                success(OBJMEM_fetchPointer_ofObject(DescriptorIndex, file) !=
                        NilPointer);
                success(page != NilPointer);
                if (get_success()) {
                    int             fd = (int)
                        positive32BitValueOf(OBJMEM_fetchPointer_ofObject
                                             (DescriptorIndex, file));
                    int             pageNumber =
                        Interp_FetchInteger_ofObject(PageNumberIndex, page);
                    assert(pageNumber >= 1);

                    int             byteArray =
                        OBJMEM_fetchPointer_ofObject(PageInPageIndex, page);

                    int             position = (pageNumber - 1) * PageSize;

                    if (System_SeekFileTo(fd, position) != position) {
                        push(FalsePointer);
                        return;
                    }
                    /* No direct pointer access! Read full page and store byte */
                    /* by byte */
                    int             bytesInPage =
                        System_ReadFile(fd, (char *) &pageBuffer, PageSize);

                    for (int i = 0; i < bytesInPage; i++)
                        OBJMEM_storeByte_ofObject_withValue(i, byteArray,
                                                            pageBuffer[i]);

                    Interp_StoreInteger_ofObject_withValue(BytesInPageIndex,
                                                           page, bytesInPage);
                    push(TruePointer);
                }
            }
            break;

        case 1:                /* Write page */
            {
                success(OBJMEM_fetchPointer_ofObject(DescriptorIndex, file) !=
                        NilPointer);
                success(page != NilPointer);
                if (get_success()) {
                    int             fd = (int)
                        positive32BitValueOf(OBJMEM_fetchPointer_ofObject
                                             (DescriptorIndex, file));
                    int             pageNumber =
                        Interp_FetchInteger_ofObject(PageNumberIndex, page);
                    assert(pageNumber >= 1);

                    int             byteArray =
                        OBJMEM_fetchPointer_ofObject(PageInPageIndex, page);

                    int             position = (pageNumber - 1) * PageSize;

                    if (System_SeekFileTo(fd, position) != position) {
                        push(FalsePointer);
                        return;
                    }
                    /* No direct pointer access! Read bytes by byte into */
                    /* staging buffer */
                    int             bytesInPage =
                        Interp_FetchInteger_ofObject(BytesInPageIndex, page);
                    /* Fill buffer with page data */
                    for (int i = 0; i < bytesInPage; i++)
                        pageBuffer[i] = OBJMEM_fetchByte_ofObject(i, byteArray);

                    /* blast it out */
                    if (System_WriteFile(fd, (char *) pageBuffer, bytesInPage)
                        == bytesInPage)
                        push(TruePointer);
                    else
                        push(FalsePointer);
                }
            }
            break;

        case 2:                /* truncate page (make it the last page). A nil */
            /* page means truncate to zero bytes */
            {
                success(OBJMEM_fetchPointer_ofObject(DescriptorIndex, file) !=
                        NilPointer);
                success(page != NilPointer);
                if (get_success()) {

                    int             result;
                    int             fd = (int)
                        positive32BitValueOf(OBJMEM_fetchPointer_ofObject
                                             (DescriptorIndex, file));
                    if (page != NilPointer) {
                        int             pageNumber =
                            Interp_FetchInteger_ofObject(PageNumberIndex, page);
                        assert(pageNumber >= 1);

                        int             bytesInPage =
                            Interp_FetchInteger_ofObject(BytesInPageIndex,
                                                         page);
                        int             newSize =
                            (pageNumber - 1) * PageSize + bytesInPage;
                        result = System_TruncateFileTo(fd, newSize);
                    } else
                        result = System_TruncateFileTo(fd, 0);
                    push(result != -1 ? TruePointer : FalsePointer);
                }

            }
            break;

        case 3:                /* File size */
            {
                success(OBJMEM_fetchPointer_ofObject(DescriptorIndex, file) !=
                        NilPointer);
                if (get_success()) {
                    int             fd = (int)
                        positive32BitValueOf(OBJMEM_fetchPointer_ofObject
                                             (DescriptorIndex, file));
                    int             size = System_FileSize(fd);
                    if (size >= 0)
                        push(positive32BitIntegerFor(size));
                    else
                        push(NilPointer);
                }
            }
            break;

        case 4:                /* Open File */
            {
                success(OBJMEM_fetchClassOf(name) == ClassStringPointer);
                if (get_success()) {
                    char           *fileName = Interp_StringFromObject(name);
                    int             fd = System_OpenFile(fileName);
                    free(fileName);
                    if (fd != -1) {
                        OBJMEM_storePointer_ofObject_withValue(DescriptorIndex,
                                                               file,
                                                               positive32BitIntegerFor
                                                               (fd));
                        push(TruePointer);
                    } else
                        push(FalsePointer);
                }
            }

            break;

        case 5:                /* Close file */
            {
                int             fd = (int)
                    positive32BitValueOf(OBJMEM_fetchPointer_ofObject
                                         (DescriptorIndex, file));
                int             result = System_CloseFile(fd);
                OBJMEM_storePointer_ofObject_withValue(DescriptorIndex, file,
                                                       NilPointer);
                push(result != -1 ? TruePointer : FalsePointer);
            }
            break;
        }
    }
    if (!get_success())
        unPop(4);
}

void
primitivePosixDirectoryOperation()
{
    /*
     * If you allocate an array and then allocate more objects to put into that
     * array, the array pointer has no reference and can go away if GC happens.
     * Be sure to push it on the stack after alloc to protect it.
     *
     */
    /*
     * ID ARG1 ARG2 Return Remarks ---- -------- ------- ------ ----------- 0
     * name fd/nil create file (replacing existing one) 1 name true/false
     * delete file 2 new name PosixFile true/false rename file 3 nil Array of
     * strings Array of file names in directory
     */
    const int       DescriptorIndex = 8;        /* fd field of PosixFile */

    int             arg2 = popStack();
    int             arg1 = popStack();
    int             code = popInteger();
    pop(1);                     /* remove receiver */

    success(code >= 0 && code <= 3);
    success(arg1 == NilPointer ||
            OBJMEM_fetchClassOf(arg1) == ClassStringPointer);

    if (get_success()) {
        switch (code) {
        case 0:                /* Create file */
            {
                char           *s = Interp_StringFromObject(arg1);
                int             fd = System_CreateFile(s);
                free(s);
                if (fd != -1) {
                    push(positive32BitIntegerFor(fd));
                } else
                    push(NilPointer);
            }
            break;

        case 1:                /* Delete file */
            {
                char           *s = Interp_StringFromObject(arg1);
                push(System_DeleteFile(s) ? TruePointer : FalsePointer);
                free(s);
            }
            break;

        case 2:                /* Rename file */
            {
                int             oldPos = 0;     /* silence uninitialized */
                /* variable warning */
                int             file = arg2;
                int             newNameObjectPointer = arg1;
                int             descriptorObjectPointer =
                    OBJMEM_fetchPointer_ofObject(DescriptorIndex, file);
                bool            wasOpen =
                    (descriptorObjectPointer != NilPointer);
                if (wasOpen) {
                    int             fd =
                        (int) positive32BitValueOf(descriptorObjectPointer);
                    oldPos = System_TellFile(fd);       /* Save position for */
                    /* restore after we */
                    /* open new file... */
                    /* Some operating systems don't let you rename an open */
                    /* file... */
                    System_CloseFile(fd);
                }
                char           *newName =
                    Interp_StringFromObject(newNameObjectPointer);
                char           *oldName =
                    Interp_StringFromObject(OBJMEM_fetchPointer_ofObject
                                            (FileNameIndex, file));
                if (System_RenameFile(oldName, newName)) {
                    /* Re-open and update descriptor of File object */
                    if (wasOpen) {
                        int             fd = System_OpenFile(newName);
                        if (fd != -1) {
                            OBJMEM_storePointer_ofObject_withValue
                                (DescriptorIndex, file,
                                 positive32BitIntegerFor(fd));
                            System_SeekFileTo(fd, oldPos);      /* Restore */
                            /* position.... */
                        } else
                            OBJMEM_storePointer_ofObject_withValue
                                (DescriptorIndex, file, NilPointer);
                    }
                    /* Update with new name */
                    OBJMEM_storePointer_ofObject_withValue(FileNameIndex, file,
                                                           newNameObjectPointer);
                    push(TruePointer);
                } else {
                    if (wasOpen) {
                        /* Something failed. Open the original file again...if */
                        /* possible */
                        int             fd = System_OpenFile(oldName);
                        if (fd != -1) {
                            OBJMEM_storePointer_ofObject_withValue
                                (DescriptorIndex, file,
                                 positive32BitIntegerFor(fd));
                            System_SeekFileTo(fd, oldPos);      /* Restore */
                            /* position.... */
                        } else
                            OBJMEM_storePointer_ofObject_withValue
                                (DescriptorIndex, file, NilPointer);
                    }
                    push(FalsePointer);
                }
                free(newName);
                free(oldName);
            }
            break;
        case 3:                /* Enumerate files */
            {
                int             length;
                char          **files = System_ListDir(&length);
                int             array =
                    OBJMEM_instantiateClass_withPointers(ClassArrayPointer,
                                                         length);
                /* Push the array first so there is a refernce on it just in */
                /* case GC takes */
                /* place when we add the */

                /* entries */
                push(array);

                for (int i = 0; i < length; i++) {
                    OBJMEM_storePointer_ofObject_withValue(i, array,
                                                           Interp_StringObjectFor
                                                           (files[i]));
                }
                System_FreeListDir(files, length);
            }
            break;
        }
    } else
        unPop(4);
}

void
primitivePosixLastErrorOperation()
{
    pop(1);
    pushInteger(System_LastError());
}

void
primitivePosixErrorStringOperation()
{
    int             code = popInteger();
    pop(1);                     /* pop receiver */
    if (get_success())
        push(Interp_StringObjectFor(System_ErrorText(code)));
    else
        unPop(2);
}

/*
 * --------------------- prim_StorageManagement.inc.h
 * -----------------------------
 */
void
primitiveInstVarAtPut()
{
    int             thisReceiver;
    int             index;
    int             newValue;
    /*
     * "source" "ERROR: realValue is not used in the method" [...]
     */
    newValue = popStack();
    index = popInteger();
    thisReceiver = popStack();
    checkIndexableBoundsOf_in(index, thisReceiver);
    if (get_success()) {
        subscript_with_storing(thisReceiver, index, newValue);
    }
    if (get_success())
        push(newValue);
    else
        unPop(3);
}

void
primitiveAsOop()
{
    int             thisReceiver;
    thisReceiver = popStack();
    /* fprintf(stderr, "oop val is for:%i is:%i\n", thisReceiver, (thisReceiver */
    /* | 1) >> 1); */
    success(OBJMEM_isIntegerObject(thisReceiver) == false);
    if (get_success())
        push(thisReceiver | 1);
    else
        unPop(1);
}

void
primitiveAsObject()
{
    int             thisReceiver;
    int             newOop;
    thisReceiver = popStack();
    newOop = thisReceiver & 0xFFFE;
    success(OBJMEM_hasObject(newOop));
    if (get_success())
        push(newOop);
    else
        unPop(1);
}

void
primitiveSomeInstance()
{
    int             cls;
    int             instance;
    cls = popStack();
    instance = OBJMEM_initialInstanceOf(cls);
    if (instance == NilPointer)
        primitiveFail();
    else
        push(instance);
}

void
primitiveNextInstance()
{
    int             object;
    int             instance;
    object = popStack();
    instance = OBJMEM_instanceAfter(object);
    if (instance == NilPointer)
        primitiveFail();
    else
        push(instance);
}

void
primitiveNewMethod()
{
    int             header;
    int             bytecodeCount;
    int             cls;
    int             size;
    header = popStack();
    bytecodeCount = popInteger();
    cls = popStack();
    int             literalCount = literalCountOfHeader(header);
    size = (literalCount + 1) * 2 + bytecodeCount;
    R_cls.newMethod = OBJMEM_instantiateClass_withBytes(cls, size);
    for (int i = 0; i < literalCount; i++) {
        OBJMEM_storeWord_ofObject_withValue(LiteralStart + i, R_cls.newMethod,
                                            NilPointer);
    }
    /* Note: using storeWord vs storePointer because it memory with */
    /* initialized with zeros and this break ref counting */
    OBJMEM_storeWord_ofObject_withValue(HeaderIndex, R_cls.newMethod, header);
    push(R_cls.newMethod);
}

void
primitiveObjectAt()
{
    int             thisReceiver;
    int             index;
    index = popInteger();
    thisReceiver = popStack();
    success(index > 0);
    success(index <= objectPointerCountOf(thisReceiver));
    if (get_success()) {
        push(OBJMEM_fetchPointer_ofObject(index - 1, thisReceiver));
    } else
        unPop(2);

}

void
primitiveObjectAtPut()
{
    int             thisReceiver;
    int             index;
    int             newValue;
    newValue = popStack();
    index = popInteger();
    thisReceiver = popStack();
    success(index > 0);
    success(index <= objectPointerCountOf(thisReceiver));
    if (get_success()) {
        OBJMEM_storePointer_ofObject_withValue(index - 1, thisReceiver,
                                               newValue);
        push(newValue);
    } else
        unPop(3);
}

void
primitiveNew()
{
    int             cls;
    int             size;
    cls = popStack();
    size = fixedFieldsOf(cls);
    success(isIndexable(cls) == false);
    if (get_success()) {
        if (isPointers(cls)) {
            push(OBJMEM_instantiateClass_withPointers(cls, size));
        } else {
            push(OBJMEM_instantiateClass_withWords(cls, size));
        }
    } else
        unPop(1);
}

void
primitiveNewWithArg()
{
    int             size;
    int             cls;
    /*
     * "source" [...] self success: size <= 65533. "dbanay: ERROR check max
     * size" [...]
     */
    size = positive16BitValueOf(popStack());
    success(size <= 65533);

    cls = popStack();
    success(isIndexable(cls));
    if (get_success()) {
        size = size + fixedFieldsOf(cls);
        if (isPointers(cls)) {
            push(OBJMEM_instantiateClass_withPointers(cls, size));
        } else {
            if (isWords(cls)) {
                push(OBJMEM_instantiateClass_withWords(cls, size));
            } else {
                push(OBJMEM_instantiateClass_withBytes(cls, size));
            }
        }
    } else
        unPop(2);
}

void
primitiveBecome()
{
    int             thisReceiver;
    int             otherPointer;
    otherPointer = popStack();
    thisReceiver = popStack();
    success(!OBJMEM_isIntegerObject(otherPointer));
    success(!OBJMEM_isIntegerObject(thisReceiver));
    if (get_success()) {
        OBJMEM_swapPointersOf_and(thisReceiver, otherPointer);
        push(thisReceiver);
    } else
        unPop(2);
}

void
primitiveInstVarAt()
{
    int             thisReceiver;
    int             index;
    int             value = 0;
    index = popInteger();
    thisReceiver = popStack();
    checkInstanceVariableBoundsOf_in(index, thisReceiver);
    if (get_success())
        value = subscript_with(thisReceiver, index);
    if (get_success())
        push(value);
    else
        unPop(2);

}

void
checkInstanceVariableBoundsOf_in(int index, int object)
{
    /* cls = OBJMEM_fetchClassOf(object); */
    success(index >= 1);
    success(index <= lengthOf(object));
}

/*
 * --------------------- prim_System.inc.h -----------------------------
 */
void
primitiveEquivalent()
{
    int             thisObject;
    int             otherObject;
    otherObject = popStack();
    thisObject = popStack();
    if (thisObject == otherObject)
        push(TruePointer);
    else
        push(FalsePointer);
}

void
primitiveClass()
{
    int             instance;
    instance = popStack();
    push(OBJMEM_fetchClassOf(instance));
}

void
primitiveCoreLeft()
{
    pop(1);                     /* remove receiver */
    push(positive32BitIntegerFor(OBJMEM_coreLeft()));
}
void
primitiveQuit()
{
    System_SignalQuit();
}

void
primitiveExitToDebugger()
{
    System_ExitToDebugger();
}

void
primitiveOopsLeft()
{
    pop(1);                     /* remove receiver */
    push(positive16BitIntegerFor(OBJMEM_oopsLeft()));
}

void
primitiveSignalAtOopsLeftWordsLeft()
{
    uint32_t        numWords = positive32BitValueOf(popStack());
    int             numOops = positive16BitValueOf(popStack());
    int             semaphore = popStack();
    /* sempahore must either be nil or an instance of Semaphore */
    success(semaphore == NilPointer
            || OBJMEM_fetchClassOf(semaphore) == ClassSemaphorePointer);
    if (get_success()) {
        /* Don't need to take reference since Smalltalk is waiting on it */
        /* someplace else */
        R_mem.lowSpaceSemaphore = semaphore;
        R_mem.oopsLeftLimit = numOops;
        R_mem.wordsLeftLimit = numWords;
    } else
        unPop(3);
}


/*
 * ------------------------------ utils ------------------------------
 */
float
extractFloat(int objectPointer)
{
    uint32_t        uint32 = (OBJMEM_fetchWord_ofObject(1, objectPointer) << 16)
        | OBJMEM_fetchWord_ofObject(0, objectPointer);
    float           f;
    memcpy(&f, &uint32, sizeof(uint32_t));
    /* return * (float *) &uint32; */
    return f;
}

/*
 * blue book
 */
void
success(bool successValue)
{
    G_successFlag = G_successFlag && successValue;
}

void
initPrimitive()
{
    G_successFlag = true;
}

int
primitiveFail()
{
    G_successFlag = false;
    return 0;                   /* invalid oop */
}

bool
get_success()
{
    return G_successFlag;
}

int
popInteger()
{
    int             integerPointer;
    integerPointer = popStack();
    success(OBJMEM_isIntegerObject(integerPointer));
    if (get_success())
        return OBJMEM_integerValueOf(integerPointer);
    return INT_MIN;
}

void
pushInteger(int integerValue)
{
    push(OBJMEM_integerObjectOf(integerValue));
}

int
positive16BitIntegerFor(int integerValue)
{
    int             newLargeInteger;
    if (integerValue < 0) {
        return primitiveFail();
    }
    if (OBJMEM_isIntegerValue(integerValue))
        return OBJMEM_integerObjectOf(integerValue);
    newLargeInteger =
        OBJMEM_instantiateClass_withBytes(ClassLargePositiveIntegerPointer, 2);
    OBJMEM_storeByte_ofObject_withValue(0, newLargeInteger,
                                        Interp_LowByteOf(integerValue));
    OBJMEM_storeByte_ofObject_withValue(1, newLargeInteger,
                                        Interp_HighByteOf(integerValue));
    return newLargeInteger;
}

int
positive32BitIntegerFor(int integerValue)
{
    if (OBJMEM_isIntegerValue(integerValue))
        return OBJMEM_integerObjectOf(integerValue);
    int             newLargeInteger;
    newLargeInteger =
        OBJMEM_instantiateClass_withBytes(ClassLargePositiveIntegerPointer, 4);
    OBJMEM_storeByte_ofObject_withValue(0, newLargeInteger,
                                        Interp_LowByteOf(integerValue));
    OBJMEM_storeByte_ofObject_withValue(1, newLargeInteger,
                                        Interp_HighByteOf(integerValue));
    OBJMEM_storeByte_ofObject_withValue(2, newLargeInteger,
                                        (integerValue >> 16) & 0xff);
    OBJMEM_storeByte_ofObject_withValue(3, newLargeInteger,
                                        (integerValue >> 24) & 0xff);
    return newLargeInteger;

}

int
positive16BitValueOf(int integerPointer)
{
    int             value;
    if (OBJMEM_isIntegerObject(integerPointer))
        return OBJMEM_integerValueOf(integerPointer);
    if (OBJMEM_fetchClassOf(integerPointer) != ClassLargePositiveIntegerPointer)
        return primitiveFail();
    if (OBJMEM_fetchByteLengthOf(integerPointer) != 2)
        return primitiveFail();
    value = OBJMEM_fetchByte_ofObject(1, integerPointer);
    value = value * 256 + OBJMEM_fetchByte_ofObject(0, integerPointer);
    return value;
}

uint32_t
positive32BitValueOf(int integerPointer)
{
    uint32_t        value;
    if (OBJMEM_isIntegerObject(integerPointer))
        return OBJMEM_integerValueOf(integerPointer);
    if (OBJMEM_fetchClassOf(integerPointer) != ClassLargePositiveIntegerPointer)
        return primitiveFail();
    int             bytes = OBJMEM_fetchByteLengthOf(integerPointer);
    value = OBJMEM_fetchByte_ofObject(0, integerPointer);
    if (bytes > 1)
        value |= OBJMEM_fetchByte_ofObject(1, integerPointer) << 8;
    if (bytes > 2)
        value |= OBJMEM_fetchByte_ofObject(2, integerPointer) << 16;
    if (bytes > 3)
        value |=
            ((uint32_t) OBJMEM_fetchByte_ofObject(3, integerPointer)) << 24;
    return value;
}

bool
primitiveResponse()
{
    int             flagValue;
    if (R_cls.primitiveIndex == 0) {
        flagValue = flagValueOf(R_cls.newMethod);
        if (flagValue == 5) {
            quickReturnSelf();
            return true;
        }
        if (flagValue == 6) {
            quickInstanceLoad();
            return true;
        }
        return false;
    }
    initPrimitive();
    dispatchPrimitives();
    return get_success();
}

void
quickReturnSelf()
{
    /* self is on stack top */
}

void
quickInstanceLoad()
{
    int             thisReceiver;
    int             fieldIndex;
    thisReceiver = popStack();
    fieldIndex = fieldIndexOf(R_cls.newMethod);
    push(OBJMEM_fetchPointer_ofObject(fieldIndex, thisReceiver));
}

/*
 * -------------------------- dispatch -------------------------
 */
int
specialSelectorPrimitiveResponse()
{
    initPrimitive();
    if (between_and(G_currentBytecode, 176, 191))
        arithmeticSelectorPrimitive();
    else if (between_and(G_currentBytecode, 192, 207))
        commonSelectorPrimitive();
    return get_success();
}

void
commonSelectorPrimitive()
{
    int             receiverClass;
    R_cls.argumentCount =
        Interp_FetchInteger_ofObject((G_currentBytecode - 176) * 2 + 1,
                                     SpecialSelectorsPointer);
    receiverClass = OBJMEM_fetchClassOf(stackValue(R_cls.argumentCount));
    switch (G_currentBytecode) {
    case 198:
        primitiveEquivalent();
        break;
    case 199:
        primitiveClass();
        break;
    case 200:
        {
            success(receiverClass == ClassMethodContextPointer
                    || receiverClass == ClassBlockContextPointer);
            if (get_success())
                primitiveBlockCopy();
        }
        break;
    case 201:
    case 202:
        {
            success(receiverClass == ClassBlockContextPointer);
            if (get_success())
                primitiveValue();
        }
        break;
    default:
        {
            primitiveFail();
            /* fprintf(stderr, "commonSelectorPrimitive noaction %i\n", */
            /* G_currentBytecode); */
        }
    }
}

void
arithmeticSelectorPrimitive()
{
    if (get_success()) {
        switch (G_currentBytecode) {
        case 176:
            primitiveAdd();
            break;
        case 177:
            primitiveSubtract();
            break;
        case 178:
            primitiveLessThan();
            break;
        case 179:
            primitiveGreaterThan();
            break;
        case 180:
            primitiveLessOrEqual();
            break;
        case 181:
            primitiveGreaterOrEqual();
            break;
        case 182:
            primitiveEqual();
            break;
        case 183:
            primitiveNotEqual();
            break;
        case 184:
            primitiveMultiply();
            break;
        case 185:
            primitiveDivide();
            break;
        case 186:
            primitiveMod();
            break;
        case 187:
            primitiveMakePoint();
            break;
        case 188:
            primitiveBitShift();
            break;
        case 189:
            primitiveDiv();
            break;
        case 190:
            primitiveBitAnd();
            break;
        case 191:
            primitiveBitOr();
            break;
        default:
            fprintf(stderr, "arithmeticSelectorPrimitive noaction %i\n",
                    G_currentBytecode);
            break;
        }
    }
}

void
dispatchPrimitives()
{
    switch (R_cls.primitiveIndex) {
        /* dispatchIntegerPrimitives */
    case 1:
        primitiveAdd();
        break;
    case 2:
        primitiveSubtract();
        break;
    case 3:
        primitiveLessThan();
        break;
    case 4:
        primitiveGreaterThan();
        break;
    case 5:
        primitiveLessOrEqual();
        break;
    case 6:
        primitiveGreaterOrEqual();
        break;
    case 7:
        primitiveEqual();
        break;
    case 8:
        primitiveNotEqual();
        break;
    case 9:
        primitiveMultiply();
        break;
    case 10:
        primitiveDivide();
        break;
    case 11:
        primitiveMod();
        break;
    case 12:
        primitiveDiv();
        break;
    case 13:
        primitiveQuo();
        break;
    case 14:
        primitiveBitAnd();
        break;
    case 15:
        primitiveBitOr();
        break;
    case 16:
        primitiveBitXor();
        break;
    case 17:
        primitiveBitShift();
        break;
    case 18:
        primitiveMakePoint();
        break;
    case 19:
        break;
        /* dispatchLargeIntegerPrimitives */
    case 20 ... 39:
        /* { fprintf(stderr, "dispatchLargeIntegerPrimitives fail %i\n", */
        /* R_cls.primitiveIndex); */
        primitiveFail();
        break;
        /* dispatchFloatPrimitives */
    case 40:
        primitiveAsFloat();
        break;
    case 41:
        primitiveFloatAdd();
        break;
    case 42:
        primitiveFloatSubtract();
        break;
    case 43:
        primitiveFloatLessThan();
        break;
    case 44:
        primitiveFloatGreaterThan();
        break;
    case 45:
        primitiveFloatLessOrEqual();
        break;
    case 46:
        primitiveFloatGreaterOrEqual();
        break;
    case 47:
        primitiveFloatEqual();
        break;
    case 48:
        primitiveFloatNotEqual();
        break;
    case 49:
        primitiveFloatMultiply();
        break;
    case 50:
        primitiveFloatDivide();
        break;
    case 51:
        primitiveTruncated();
        break;
    case 52:
        primitiveFractionalPart();
        break;
    case 53:
        primitiveExponent();
        break;
    case 54:
        primitiveTimesTwoPower();
        break;
    case 55 ... 59:
        fprintf(stderr, " dispatchFloatPrimitives noaction %i\n",
                R_cls.primitiveIndex);
        break;
        /* dispatchSubscriptAndStreamPrimitives */
    case 60:
        primitiveAt();
        break;
    case 61:
        primitiveAtPut();
        break;
    case 62:
        primitiveSize();
        break;
    case 63:
        primitiveStringAt();
        break;
    case 64:
        primitiveStringAtPut();
        break;
    case 65:
        primitiveNext();
        break;
    case 66:
        primitiveNextPut();
        break;
    case 67:
        primitiveAtEnd();
        break;
        /* dispatchStorageManagementPrimitives */
    case 68:
        primitiveObjectAt();
        break;
    case 69:
        primitiveObjectAtPut();
        break;
    case 70:
        primitiveNew();
        break;
    case 71:
        primitiveNewWithArg();
        break;
    case 72:
        primitiveBecome();
        break;
    case 73:
        primitiveInstVarAt();
        break;
    case 74:
        primitiveInstVarAtPut();
        break;
    case 75:
        primitiveAsOop();
        break;
    case 76:
        primitiveAsObject();
        break;
    case 77:
        primitiveSomeInstance();
        break;
    case 78:
        primitiveNextInstance();
        break;
    case 79:
        primitiveNewMethod();
        break;
        /* dispatchControlPrimitives */
    case 80:
        primitiveBlockCopy();
        break;
    case 81:
        primitiveValue();
        break;
    case 82:
        primitiveValueWithArgs();
        break;
    case 83:
        primitivePerform();
        break;
    case 84:
        primitivePerformWithArgs();
        break;
    case 85:
        primitiveSignal();
        break;
    case 86:
        primitiveWait();
        break;
    case 87:
        primitiveResume();
        break;
    case 88:
        primitiveSuspend();
        break;
    case 89:
        primitiveFlushCache();
        break;
        /* dispatchInputOutputPrimitives */
    case 90:
        primitiveMousePoint();
        break;
    case 91:
        primitiveCursorLocPut();
        break;
    case 92:
        primitiveCursorLink();
        break;
    case 93:
        primitiveInputSemaphore();
        break;
    case 94:
        primitiveSampleInterval();
        break;
    case 95:
        primitiveInputWord();
        break;
    case 96:
        primitiveCopyBits();
        break;
    case 97:
        primitiveSnapshot();
        break;
    case 98:
        primitiveTimeWordsInto();
        break;
    case 99:
        primitiveTickWordsInto();
        break;
    case 100:
        primitiveSignalAtTick();
        break;
    case 101:
        primitiveBeCursor();
        break;
    case 102:
        primitiveBeDisplay();
        break;
    case 103:
        primitiveScanCharacters();
        break;
    case 104:
        primitiveDrawLoop();
        break;
    case 105:
        primitiveStringReplace();
        break;
    case 106 ... 109:
        fprintf(stderr, "dispatchInputOutputPrimitives noaction %i\n",
                R_cls.primitiveIndex);
        break;
        /* dispatchSystemPrimitives */
    case 110:
        primitiveEquivalent();
        break;
    case 111:
        primitiveClass();
        break;
    case 112:
        primitiveCoreLeft();
        break;
    case 113:
        primitiveQuit();
        break;
    case 114:
        primitiveExitToDebugger();
        break;
    case 115:
        primitiveOopsLeft();
        break;
    case 116:
        primitiveSignalAtOopsLeftWordsLeft();
        break;
    case 117 ... 127:
        fprintf(stderr, "dispatchSystemPrimitives noaction %i\n",
                R_cls.primitiveIndex);
        break;
        /* dispatchPrivatePrimitives */
    case 128:
        primitiveBeSnapshotFile();
        break;
    case 130:
        primitivePosixFileOperation();
        break;
    case 131:
        primitivePosixDirectoryOperation();
        break;
    case 132:
        primitivePosixLastErrorOperation();
        break;
    case 133:
        primitivePosixErrorStringOperation();
        break;
    case 140:
        primitiveImageArgv();
        break;
    case 141:
        primitiveShowStdout();
        break;
    case 142:
        primitiveStdinInputSemaphore();
        break;
    case 143:
        primitiveStdinInputString();
        break;
    case 144:
        primitiveLoadedImageName();
        break;
    default:
        {
            fprintf(stderr, "dispatchPrivatePrimitives fail %i\n",
                    R_cls.primitiveIndex);
            primitiveFail();
        }
        break;
    }
}
