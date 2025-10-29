#include "config.h"

#include "minist8.h"
#include "bytecodes.h"
#include "objectmemory.h"
#include "routines.h"
#include "contexts.h"
#include "classes.h"
#include "primitives.h"


/* stack bytecodes */

void pushReceiverVariableBytecode() {
  int fieldIndex;
  fieldIndex = extractBits_to_of(12, 15, G_currentBytecode);
  pushReceiverVariable(fieldIndex);
}

void pushReceiverVariable(int fieldIndex) {
  push(OBJMEM_fetchPointer_ofObject(fieldIndex, R_ctx.receiver));
}

void pushTemporaryVariableBytecode() {
  int fieldIndex;
  fieldIndex = extractBits_to_of(12, 15, G_currentBytecode);
  pushTemporaryVariable(fieldIndex);
}
void pushTemporaryVariable(int temporaryIndex) {
  push(temporary(temporaryIndex));
}
void pushLiteralConstantBytecode() {
  int fieldIndex;
  fieldIndex = extractBits_to_of(11, 15, G_currentBytecode);
  pushLiteralConstant(fieldIndex);
}
void pushLiteralConstant(int literalIndex) {
  push(literal(literalIndex));
}
void pushLiteralVariableBytecode() {
  int fieldIndex;
  fieldIndex = extractBits_to_of(11, 15, G_currentBytecode);
  pushLiteralVariable(fieldIndex);
}
void pushLiteralVariable(int literalIndex) {
  int association;
  association = literal(literalIndex);
  push(OBJMEM_fetchPointer_ofObject(ValueIndex, association));
}

void pushReceiverBytecode() {
  push(R_ctx.receiver);
}
void duplicateTopBytecode() {
  push(stackTop());
}

void pushActiveContextBytecode() {
  push(R_ctx.activeContext);
}
void storeAndPopReceiverVariableBytecode() {
  int variableIndex;
  variableIndex = extractBits_to_of(13, 15, G_currentBytecode);
  OBJMEM_storePointer_ofObject_withValue(variableIndex, R_ctx.receiver, popStack());
}

void storeAndPopTemporaryVariableBytecode() {
  int variableIndex;
  variableIndex = extractBits_to_of(13, 15, G_currentBytecode);
  OBJMEM_storePointer_ofObject_withValue(variableIndex + TempFrameStart,
      R_ctx.homeContext, popStack());
}

void extendedStoreAndPopBytecode() {
  extendedStoreBytecode();
  popStackBytecode();
}

void extendedStoreBytecode() {
  int descriptor;
  int variableType;
  int variableIndex;
  int association;
  descriptor = fetchByte();
  variableType = extractBits_to_of(8, 9, descriptor);
  variableIndex = extractBits_to_of(10, 15, descriptor);
  switch (variableType) {
  case 0:
    OBJMEM_storePointer_ofObject_withValue(variableIndex, R_ctx.receiver, stackTop());
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
    OBJMEM_storePointer_ofObject_withValue(ValueIndex, association, stackTop());
    break;
  }
}

void popStackBytecode() {
  popStack();
}

void jump(int offset) {
  R_ctx.instructionPointer = R_ctx.instructionPointer + offset;
}
void shortUnconditionalJump() {
  int offset;
  offset = extractBits_to_of(13, 15, G_currentBytecode);
  jump(offset + 1);
}

void longUnconditionalJump() {
  int offset;
  offset = extractBits_to_of(13, 15, G_currentBytecode);
  jump((offset - 4) * 256 + fetchByte());
}

void jumpIf_by(int condition, int offset) {
  int boolean;
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

void sendMustBeBoolean() {
  sendSelector_argumentCount(MustBeBooleanSelector, 0);
}

void shortConditionalJump() {
  int offset;
  offset = extractBits_to_of(13, 15, G_currentBytecode);
  jumpIf_by(FalsePointer, offset + 1);
}


/* send bytecodes */
void sendLiteralSelectorBytecode() {
  int selector;
  selector = literal(extractBits_to_of(12, 15, G_currentBytecode));
  sendSelector_argumentCount(selector,
      extractBits_to_of(10, 11, G_currentBytecode) - 1);
}


// sendSelector:argumentCount:
void sendSelector_argumentCount(int selector, int count) {
  int newReceiver;
  R_cls.messageSelector = selector;
  R_cls.argumentCount = count;
  newReceiver = stackValue(R_cls.argumentCount);

#if 0
#ifdef DEBUGGING_SUPPORT
  // Debugging aid that helped me figure out why stuff
  // wasn't working - intercept show: and error: messages and
  // display them
  const int showSelector = 6640; // #show:
  const int errorSelector = 282; // #error:
#if 0
  bool showSend = true;
  if (selector == showSelector)
  {
    int s = stackValue(argumentCount-1);
    assert(OBJMEM_fetchClassOf(s) == ClassStringPointer);
    char* text = stringFromObject(s);
    printf("Transcript: %s\n", text);
    free(text);
    showSend = false;
  }
  else if (selector == errorSelector)
  {
    int s = stackValue(argumentCount-1);
    assert(OBJMEM_fetchClassOf(s) == ClassStringPointer);
    char* text = stringFromObject(s);
    printf("ERROR: %s\n", text);
    free(text);
    showSend = false;
  }
  if (showSend)
  {
    int cls = OBJMEM_fetchClassOf(newReceiver);
    char*  className = classNameOfObject(newReceiver);
    char* sel = selectorName(selector);
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

void sendSelectorToClass(int classPointer) {
  findNewMethodInClass(classPointer);
  executeNewMethod();
}

void findNewMethodInClass(int cls) {
  int hash;
  // This is not a great hash function
  // Bits of History, pg.244 has a discussion and a better one
  // hash = ((messageSelector ^ cls) & 0xff) << 2;
  hash = (R_cls.messageSelector & cls & 0xff) << 2; // removed +1 since C arrays are zero based
  if (G_methodCache[hash] == R_cls.messageSelector && G_methodCache[hash + 1] == cls) {
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

void initializeMethodCache() {
  for (int i = 0; i < sizeof(G_methodCache) / sizeof(G_methodCache[0]); i++)
    G_methodCache[i] = NilPointer;
}

void executeNewMethod() {
  if (!primitiveResponse()) activateNewMethod();
}

void activateNewMethod() {
  int contextSize;
  int newContext;
  if (largeContextFlagOf(R_cls.newMethod) == 1)
    contextSize = 32 + TempFrameStart;
  else
    contextSize = 12 + TempFrameStart;
  newContext = OBJMEM_instantiateClass_withPointers(ClassMethodContextPointer,
      contextSize);
  OBJMEM_storePointer_ofObject_withValue(SenderIndex, newContext,
      R_ctx.activeContext);
  storeInstructionPointerValue_inContext(
      initialInstructionPointerOfMethod(R_cls.newMethod), newContext);
  storeStackPointerValue_inContext(temporaryCountOf(R_cls.newMethod), newContext);
  OBJMEM_storePointer_ofObject_withValue(MethodIndex, newContext, R_cls.newMethod);
  transfer_fromIndex_ofObject_toIndex_ofObject(R_cls.argumentCount + 1,
      R_ctx.stackPointer - R_cls.argumentCount, R_ctx.activeContext, ReceiverIndex, newContext);
  pop(R_cls.argumentCount + 1);
  newActiveContext(newContext);
}

void singleExtendedSendBytecode() {
  int descriptor;
  int selectorIndex;
  descriptor = fetchByte();
  selectorIndex = extractBits_to_of(11, 15, descriptor);
  sendSelector_argumentCount(literal(selectorIndex),
      extractBits_to_of(8, 10, descriptor));
}

void doubleExtendedSendBytecode() {
  int count;
  int selector;
  count = fetchByte();
  selector = literal(fetchByte());
  sendSelector_argumentCount(selector, count);
}

void singleExtendedSuperBytecode() {
  int descriptor;
  int selectorIndex;
  int methodClass;
  descriptor = fetchByte();
  R_cls.argumentCount = extractBits_to_of(8, 10, descriptor);
  selectorIndex = extractBits_to_of(11, 15, descriptor);
  R_cls.messageSelector = literal(selectorIndex);
  methodClass = methodClassOf(R_ctx.method);
  sendSelectorToClass(superclassOf(methodClass));
}

void doubleExtendedSuperBytecode() {
  int methodClass;
  R_cls.argumentCount = fetchByte();
  R_cls.messageSelector = literal(fetchByte());
  methodClass = methodClassOf(R_ctx.method);
  sendSelectorToClass(superclassOf(methodClass));
}

void sendSpecialSelectorBytecode() {
  int selectorIndex;
  int selector;
  int count;
  if (!specialSelectorPrimitiveResponse()) {
    selectorIndex = (G_currentBytecode - 176) * 2;
    selector = OBJMEM_fetchPointer_ofObject(selectorIndex,
        SpecialSelectorsPointer);
    count = fetchInteger_ofObject(selectorIndex + 1, SpecialSelectorsPointer);
    sendSelector_argumentCount(selector, count);
  }
}

void returnValue_to(int resultPointer, int contextPointer) {
  int sendersIP;
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

void returnToActiveContext(int aContext) {
  OBJMEM_increaseReferencesTo(aContext);
  nilContextFields();
  OBJMEM_decreaseReferencesTo(R_ctx.activeContext);
  R_ctx.activeContext = aContext;
  fetchContextRegisters();
}

void nilContextFields() {
  OBJMEM_storePointer_ofObject_withValue(SenderIndex, R_ctx.activeContext,
      NilPointer);
  OBJMEM_storePointer_ofObject_withValue(InstructionPointerIndex, R_ctx.activeContext,
      NilPointer);
}

void dispatchOnThisBytecode() {
  switch (G_currentBytecode) {
    case 0 ... 15:
      // Push Receiver Variable #iiii
      pushReceiverVariableBytecode();
      break;
    case 16 ... 31:
      // Push Temporary Location #iiii
      pushTemporaryVariableBytecode();
      break;
    case 32 ... 63:
      // Push Literal Constant #iiiii
      pushLiteralConstantBytecode();
      break;
    case 64 ... 95:
      // Push Literal Variable #iiiii
      pushLiteralVariableBytecode();
      break;
    case 96 ... 103:
      // Pop and Store Receiver Variable #iii
      storeAndPopReceiverVariableBytecode();
      break;
    case 104 ... 111:
      // Pop and Store Temporary Location #iii
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
      // Returnreceiver [ii] From Message
      returnValue_to(R_ctx.receiver, sender());
      break;
    case 121:
      // Return True [ii] From Message
      returnValue_to(TruePointer, sender());
      break;
    case 122:
      // Return False [ii] From Message
      returnValue_to(FalsePointer, sender());
      break;
    case 123:
      // Return Nil [ii] From Message
      returnValue_to(NilPointer, sender());
      break;
    case 124:
      returnValue_to(popStack(), sender());
      // Return Stack Top From Message [i]
      break;
    case 125:
      // Return Stack Top From Block [i]
      returnValue_to(popStack(), caller());
      break;
    case 126 ... 127:
      // unused
      break;
    case 128:
      // Push (Receiver Variable, Temporary Location, Lit eral Constant,
      // Literal Variable) [jj] #kkkkkk
      { int descriptor;
        int variableType;
        int variableIndex;
        descriptor = fetchByte();
        variableType = extractBits_to_of(8, 9, descriptor);
        variableIndex = extractBits_to_of(10, 15, descriptor);
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
      // Store (Receiver Variable, Temporary Location, Ille gal, Literal
      // Variable) [jj] #kkkkkk
      extendedStoreBytecode();
      break;
    case 130:
      // Pop and Store (Receiver Variable, Temporary Location, Illegal,
      // Literal Variable) [jj] #kkkkkk
      extendedStoreAndPopBytecode();
      break;
    case 131:
      // 131 Send Literal Selector #kkkkk With #jjj Arguments
      singleExtendedSendBytecode();
      break;
    case 132:
      // 132 Send Literal Selector #kkkkkkkk With #jjjjjjjj Arguments
      doubleExtendedSendBytecode();
      break;
    case 133:
      // 133 Send Literal Selector #kkkkk To Superclass With jjj Arguments
      singleExtendedSuperBytecode();
      break;
    case 134:
      break;
      // 134 Send Literal Selector # kkkkkkkk To Superclass With jjjjjjjj Arguments
      doubleExtendedSuperBytecode();
      break;
    case 135:
      // Pop Stack Top
      popStackBytecode();
      break;
    case 136:
      // Duplicate Stack Top
      duplicateTopBytecode();
      break;
    case 137:
      // Push Active Context
      pushActiveContextBytecode();
      break;
    case 138 ... 143:
      // unused
      break;
    case 144 ... 151:
      // Jump iii+ 1 (i.e., 1 through 8)
      shortUnconditionalJump();
      break;
    case 152 ... 159:
      // Pop and Jump On False iii+ 1 (i.e., 1 through 8)
      shortConditionalJump();
      break;
    case 160 ... 167:
      // Jump (iii .. 4).256 + jjjjjjjj
      longUnconditionalJump();
      break;
    case 168 ... 171:
      // 168 ... 171 Pop and Jump On True i i*256 .. I-jjjjjjjj
        { int offset;
          offset = extractBits_to_of(14, 15, G_currentBytecode);
          offset = offset * 256 + fetchByte();
          jumpIf_by(TruePointer, offset); }
      break;
    case 172 ... 175:
      // 172 ... 175 Pop and Jump On False ii.256 + jjjjjjjj
      { int offset;
        offset = extractBits_to_of(14, 15, G_currentBytecode);
        offset = offset * 256 + fetchByte();
        jumpIf_by(FalsePointer, offset); }
      break;
    case 176 ... 207:
      // 176 ... 191 Send Arthmetic Message #iiii
      // 192 ... 207 Send Special Message #iiii
      sendSpecialSelectorBytecode();
      break;
    case 208 ... 255:
      // 208 ... 223 Send Literal Selector #iiii With No Arguments
      // 224 ... 239 end Literal Selector #iiii With 1 Argument
      // 240 ... 255 Send Literal Selector #iiii With 2 Arguments
      sendLiteralSelectorBytecode();
      break;
    default: {}
  }
}
