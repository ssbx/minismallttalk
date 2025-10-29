#ifndef _PRIM_H_
#define _PRIM_H_
#include <stdbool.h>
/*
 * #include "Hardware.h" 
 */

void            error(const char *message);

/*
 * --- ArrayStrmPrims --- 
 */
void            primitiveTestSeb();
void            primitiveAtEnd();
void            checkIndexableBoundsOf_in(int index, int array);
void            primitiveNextPut();
int             lengthOf(int array);
void            primitiveNext();
void            dispatchSubscriptAndStreamPrimitives();
void            primitiveStringAt();
void            primitiveAt();
void            primitiveSize();
void            primitiveStringAtPut();
int             subscript_with(int array, int index);
void            primitiveAtPut();
void            subscript_with_storing(int array, int index, int value);

/*
 * --- Contexts --- 
 */
void            storeContextRegisters();
void            unPop(int number);
bool            isBlockContext(int contextPointer);
void            push(int object);
int             instructionPointerOfContext(int contextPointer);
void            newActiveContext(int aContext);
int             argumentCountOfBlock(int blockPointer);
int             literal(int offset);
int             sender();
int             temporary(int offset);
int             caller();
void            pop(int number);
void            storeStackPointerValue_inContext(int value, int contextPointer);
int             stackValue(int offset);
int             stackTop();
int             popStack();
void            fetchContextRegisters();
void            storeInstructionPointerValue_inContext(int value,
                                                       int contextPointer);
int             stackPointerOfContext(int contextPointer);
/*
 * --- Classes --- 
 */
bool            lookupMethodInDictionary(int dictionary);
bool            isPointers(int classPointer);
int             superclassOf(int classPointer);
int             fixedFieldsOf(int classPointer);
bool            isWords(int classPointer);
int             hash(int objectPointer);
bool            isIndexable(int classPointer);
int             instanceSpecificationOf(int classPointer);
void            createActualMessage();
bool            lookupMethodInClass(int cls);

/*
 * --- IOPrims --- 
 */
void            dispatchInputOutputPrimitives();
void            primitiveMousePoint();
void            primitiveCursorLocPut();
void            primitiveCursorLink();
void            primitiveInputSemaphore();
void            primitiveSampleInterval();
void            primitiveInputWord();
void            updateDisplay(int destForm, int updatedHeight, int updatedWidth,
                              int updatedX, int updatedY);
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
/*
 * --- ReturnBytecode --- 
 */
void            returnToActiveContext(int aContext);
void            returnBytecode();
void            nilContextFields();
void            returnValue_to(int resultPointer, int contextPointer);
/*
 * --- ControlPrims --- 
 */
void            synchronousSignal(int aSemaphore);
void            asynchronousSignal(int aSemaphore);
void            primitiveBlockCopy();
void            primitiveResume();
void            primitivePerformWithArgs();
int             wakeHighestPriority();
void            primitivePerform();
void            primitiveValueWithArgs();
int             removeFirstLinkOfList(int aLinkedList);
void            primitiveWait();
void            primitiveFlushCache();
void            suspendActive();
int             activeProcess();
int             schedulerPointer();
void            addLastLink_toList(int aLink, int aLinkedList);
void            dispatchControlPrimitives();
void            checkProcessSwitch();
void            primitiveSignal();
int             isEmptyList(int aLinkedList);
void            primitiveSuspend();
void            primitiveValue();
int             firstContext();
void            transferTo(int aProcess);
void            resume(int aProcess);
void            sleep(int aProcess);
/*
 * --- SystemPrims --- 
 */
void            primitiveClass();
void            dispatchSystemPrimitives();
void            primitiveEquivalent();
void            primitiveCoreLeft();
void            primitiveQuit();
void            primitiveExitToDebugger();
void            primitiveOopsLeft();
void            primitiveSignalAtOopsLeftWordsLeft();
void            dispatchPrivatePrimitives();
/*
 * Posix filesystem primitives -- dbanay 
 */
void            primitiveBeSnapshotFile();
void            primitivePosixFileOperation();
void            primitivePosixDirectoryOperation();
void            primitivePosixLastErrorOperation();
void            primitivePosixErrorStringOperation();
/*
 * --- PrimitiveTest --- 
 */
void            success(bool successValue);
void            dispatchPrimitives();
int             positive16BitValueOf(int integerPointer);
uint32_t        positive32BitValueOf(int integerPointer);
void            initPrimitive();
bool            get_success();
bool            primitiveResponse();
void            quickInstanceLoad();
void            arithmeticSelectorPrimitive();
int             primitiveFail();
void            pushInteger(int integerValue);
void            quickReturnSelf();
int             positive16BitIntegerFor(int integerValue);
int             positive32BitIntegerFor(int integerValue);
int             popInteger();
int             specialSelectorPrimitiveResponse();
void            commonSelectorPrimitive();
/*
 * --- Initialization --- 
 */
void            initializeMethodCache();
/*
 * --- ArithmeticPrim --- 
 */
void            primitiveMod();
void            dispatchArithmeticPrimitives();
void            primitiveEqual();
void            primitiveBitOr();
void            primitiveDivide();
void            primitiveMultiply();
void            dispatchLargeIntegerPrimitives();
void            primitiveBitAnd();
void            primitiveSubtract();
void            dispatchIntegerPrimitives();
void            primitiveGreaterOrEqual();
void            primitiveAdd();
void            primitiveNotEqual();
void            primitiveQuo();
void            dispatchFloatPrimitives();
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
void            primitiveLessOrEqual();
void            primitiveMakePoint();
void            primitiveBitXor();
void            primitiveLessThan();
void            primitiveBitShift();
void            primitiveGreaterThan();
void            primitiveDiv();
/*
 * --- SendBytecodes --- 
 */
void            sendSelector_argumentCount(int selector, int count);
void            findNewMethodInClass(int cls);
void            activateNewMethod();
void            sendSpecialSelectorBytecode();
void            doubleExtendedSuperBytecode();
void            sendBytecode();
void            doubleExtendedSendBytecode();
void            sendSelectorToClass(int classPointer);
void            sendLiteralSelectorBytecode();
void            singleExtendedSuperBytecode();
void            singleExtendedSendBytecode();
void            extendedSendBytecode();
void            executeNewMethod();


/*
 * --- MainLoop --- 
 */
void            dispatchOnThisBytecode();
int             fetchByte();
/*
 * static void interpret(); 
 */
/*
 * --- CompiledMethod --- 
 */
int             headerOf(int methodPointer);
int             literalCountOf(int methodPointer);
int             primitiveIndexOf(int methodPointer);
int             argumentCountOf(int methodPointer);
int             literalCountOfHeader(int headerPointer);
int             fieldIndexOf(int methodPointer);
int             methodClassOf(int methodPointer);
int             literal_ofMethod(int offset, int methodPointer);
int             temporaryCountOf(int methodPointer);
int             largeContextFlagOf(int methodPointer);
int             objectPointerCountOf(int methodPointer);
int             headerExtensionOf(int methodPointer);
int             flagValueOf(int methodPointer);
int             initialInstructionPointerOfMethod(int methodPointer);
/*
 * --- IntegerAccess --- 
 */
void            Interp_StoreInteger_ofObject_withValue(int fieldIndex,
                                                       int objectPointer,
                                                       int integerValue);
void            Interp_Transfer_fromIndex_ofObject_toIndex_ofObject(int count,
                                                                    int
                                                                    firstFrom,
                                                                    int fromOop,
                                                                    int firstTo,
                                                                    int toOop);
int             Interp_LowByteOf(int anInteger);
int             Interp_FetchInteger_ofObject(int fieldIndex, int objectPointer);
int             Interp_HighByteOf(int anInteger);
/*
 * --- StoreMgmtPrims --- 
 */
void            checkInstanceVariableBoundsOf_in(int index, int object);
void            primitiveNewMethod();
void            primitiveAsOop();
void            primitiveSomeInstance();
void            primitiveObjectAt();
void            primitiveNextInstance();
void            primitiveNew();
void            primitiveAsObject();
void            primitiveNewWithArg();
void            primitiveInstVarAtPut();
void            primitiveObjectAtPut();
void            primitiveInstVarAt();
void            primitiveBecome();
void            dispatchStorageManagementPrimitives();
void            pushFloat(float f);
float           extractFloat(int objectPointer);
float           popFloat();
#ifdef GC_MARK_SWEEP
void            prepareForCollection();
void            collectionCompleted();
#endif

#endif                          /* _PRIM_H_ */
