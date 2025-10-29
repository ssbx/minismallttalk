#ifndef _INTERPRETER_BYTECODES_H_
#define _INTERPRETER_BYTECODES_H_
/* stack bytecodes */
void pushReceiverVariableBytecode();
void pushReceiverVariable(int fieldIndex);
void pushTemporaryVariableBytecode();
void pushTemporaryVariable(int temporaryIndex);
void pushLiteralConstantBytecode();
void pushLiteralConstant(int literalIndex);
void pushLiteralVariableBytecode();
void pushLiteralVariable(int literalIndex);
void extendedPushBytecode();
void pushReceiverBytecode();
void duplicateTopBytecode();
void pushConstantBytecode();
void pushActiveContextBytecode();
void storeAndPopReceiverVariableBytecode();
void storeAndPopTemporaryVariableBytecode();
void extendedStoreAndPopBytecode();
void extendedStoreBytecode();
void popStackBytecode();
/* jump bytecodes */
void jump(int offset);
void shortUnconditionalJump();
void longUnconditionalJump();
void jumpIf_by(int condition, int offset);
void sendMustBeBoolean();
void shortConditionalJump();
void longConditionalJump();
/* send bytecodes */
void sendLiteralSelectorBytecode();
void sendSelector_argumentCount(int selector, int count);
void sendSelectorToClass(int classPointer);
void findNewMethodInClass(int cls);
void initializeMethodCache();
void executeNewMethod();
void activateNewMethod();
void singleExtendedSendBytecode();
void doubleExtendedSendBytecode();
void singleExtendedSuperBytecode();
void doubleExtendedSuperBytecode();
void sendBytecode();
void sendSpecialSelectorBytecode();
/* return bytecodes */
void returnBytecode();
void returnValue_to(int resultPointer, int contextPointer);
void returnToActiveContext(int aContext);
void nilContextFields();
/* dispatch */

#endif // _INTERPRETER_BYTECODES_H_

