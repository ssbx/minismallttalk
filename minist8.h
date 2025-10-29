/*
 * Copyright © 2025 Sébastien Serre. All rights reserved.
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
#ifndef _MINIST8_H_
#define _MINIST8_H_

#include <stdbool.h>
#include <stdint.h>


void            error(const char *message);

#define between_and(v, min, max) (v >= min && v <= max)

/*
 * Registers
 */
typedef struct {
    int             activeContext;
    int             homeContext;
    int             method;
    int             receiver;
    int             instructionPointer;
    int             stackPointer;
} ContextRegisters;

typedef struct {
    int             messageSelector;
    int             argumentCount;
    int             newMethod;
    int             primitiveIndex;
} ClassRegisters;

typedef struct {
    bool            newProcessWaiting;
    int             newProcess;
    int             semaphoreList[4096];
    int             semaphoreIndex;
} SchedRegisters;

typedef struct {
    bool            checkLowMemory;
    bool            memoryIsLow;
    int             lowSpaceSemaphore;
    int             oopsLeftLimit;
    uint32_t        wordsLeftLimit;
} MemSignalRegister;

/*
 * Constants
 */
typedef enum {
    /*
     * SmallIntegers
     */
    MinusOnePointer = 65535,
    ZeroPointer = 1,
    OnePointer = 3,
    TwoPointer = 5
} SmallIntegers;

typedef enum {
    /*
     * UndefinedObject and Booleans
     */
    NilPointer = 2,
    FalsePointer = 4,
    TruePointer = 6,
    /*
     * Root
     */
    SchedulerAssociationPointer = 8,
    /*
     * Classes
     */
    ClassStringPointer = 14,
    ClassSmallInteger = 12,
    ClassArrayPointer = 16,
    ClassMethodContextPointer = 22,
    ClassBlockContextPointer = 24,
    ClassPointPointer = 26,
    ClassLargePositiveIntegerPointer = 28,
    ClassMessagePointer = 32,
    ClassCharacterPointer = 40,
    ClassCompiledMethod = 34,   /* la suite pas dans le blue book */
    ClassFloatPointer = 20,
    ClassSymbolPointer = 56,
    ClassSemaphorePointer = 38,
    ClassDisplayScreenPointer = 834,
    ClassUndefinedObject = 25728,
    SmalltalkPointer = 25286,   /* asOop retourne 12643 (depuis interp (v | 1)
                                 * // >> 1) */
    /*
     * Selectors
     */
    DoesNotUnderstandSelector = 42,
    CannotReturnSelector = 44,
    MustBeBooleanSelector = 52,
    SpecialSelectorsPointer = 48,
    /*
     * Tables
     */
    CharacterTablePointer = 50
} GaranteedPointers;


typedef enum {
    /*
     * Class CompiledMethod
     */
    HeaderIndex = 0,
    LiteralStart = 1
} MethodIndices;

typedef enum {
    /*
     * Class MethodContext
     */
    SenderIndex = 0,
    InstructionPointerIndex = 1,
    StackPointerIndex = 2,
    MethodIndex = 3,
    ReceiverIndex = 5,
    TempFrameStart = 6,
    /*
     * Class BlockContext
     */
    CallerIndex = 0,
    BlockArgumentCountIndex = 3,
    InitialIPIndex = 4,
    HomeIndex = 5
} ContextIndices;

typedef enum {
    /*
     * Class Class
     */
    SuperclassIndex = 0,
    MessageDictionaryIndex = 1,
    InstanceSpecificationIndex = 2,
    /*
     * Fields of a message dictionary
     */
    MethodArrayIndex = 1,
    SelectorStart = 2
} ClassIndices;

typedef enum {
    MessageSelectorIndex = 0,
    MessageArgumentsIndex = 1,
    MessageSize = 2
} MessageIndices;

typedef enum {
    XIndex = 0,
    YIndex = 1,
    ClassPointSize = 2
} PointIndices;


typedef enum {
    StreamArrayIndex = 0,
    StreamIndexIndex = 1,
    StreamReadLimitIndex = 2,
    StreamWriteLimitIndex = 3
} StreamIndices;

typedef enum {
    /*
     * Class ProcessorScheduler
     */
    ProcessListsIndex = 0,
    ActiveProcessIndex = 1,
    /*
     * Class LinkedList
     */
    FirstLinkIndex = 0,
    LastLinkIndex = 1,
    /*
     * Class Semaphore
     */
    ExcessSignalsIndex = 2,
    /*
     * Class Link
     */
    NextLinkIndex = 0,
    /*
     * Class Process
     */
    SuspendedContextIndex = 1,
    PriorityIndex = 2,
    MyListIndex = 3
} SchedulerIndices;


typedef enum {
    BitsInForm = 0,
    WidthInForm = 1,
    HeightInForm = 2,
    OffsetInForm = 3
} Form;

static const int ValueIndex = 1;        /* AssociationIndex */
static const int CharacterValueIndex = 0;       /* CharacterIndex */
static const int FileNameIndex = 1;

/*
 * input/output primitives
 */
typedef enum {
    KeyBackspace = 8,
    KeyTab = 9,
    KeyLineFeed = 10,
    KeyReturn = 13,
    KeyEscape = 27,
    KeySpace = 32,
    KeyDelete = 127,
    KeyLeftShift = 136,
    KeyRightShift = 137,
    KeyControl = 138,
    KeyAlphaLock = 139
} KeyParameters;

/*
 * argv options
 */
typedef struct {
    char           *rootDir;
    char           *currDir;
    char           *imgName;
    char           *imgArgv;
    bool            threeButtons;
    bool            autoAdapt;
    int             cyclesPerFrame;
    int             displayScale;
    bool            useVsync;
    bool            verbose;
    bool            debug;
    bool            trace;
    int             noVsyncDelay;
    bool            withLibui;
} Options;

/*
 * BItBlt
 */
typedef enum {
    DestFormIndex = 0,
    SourceFormIndex = 1,
    HalftoneFormIndex = 2,
    CombinationRuleIndex = 3,
    DestXIndex = 4,
    DestYIndex = 5,
    WidthIndex = 6,
    HeightIndex = 7,
    SourceXIndex = 8,
    SourceYIndex = 9,
    ClipXIndex = 10,
    ClipYIndex = 11,
    ClipWidthIndex = 12,
    ClipHeightIndex = 13
} BitBltParams;

/*
 * Character Scanner
 */
static const int LastIndexIndex = 14;
static const int XTableIndexIndex = 15;
static const int StopConditionsIndex = 16;
static const int TextIndex = 17;
static const int TextStyleIndex = 18;
static const int LeftMarginIndex = 19;
static const int RightMarginIndex = 20;
static const int FontIndex = 21;
static const int LineIndex = 22;
static const int RunStopIndexIndex = 23;
static const int SpaceCountIndex = 24;
static const int SpaceWidthIndex = 25;
static const int OutputMediumIndex = 26;

extern ContextRegisters R_ctx;
extern ClassRegisters R_cls;
extern SchedRegisters R_sch;
extern MemSignalRegister R_mem;
extern int      G_currentBytecode;
extern bool     G_successFlag;
extern int      G_methodCache[1024];
extern int      G_currentDisplay;
extern int      G_currentDisplayWidth;
extern int      G_currentDisplayHeight;
extern int      G_currentCursor;
extern Options  G_Options;
#endif                          /* _SMALLTALK80_H_ */
