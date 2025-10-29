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
#include "classes.h"
#include "routines.h"
#include "contexts.h"
#include "compiledmethods.h"
#include "objectmemory.h"


/*
 *
 * Classes p586
 *
 */
int hash(int objectPointer) {
  return objectPointer >> 1;
}
bool lookupMethodInDictionary(int dictionary) {
  int length;
  int index;
  int mask;
  bool wrapAround;
  int nextSelector;
  int methodArray;
  length = OBJMEM_fetchWordLengthOf(dictionary);
  mask = length - SelectorStart - 1;
  index = (mask & hash(R_cls.messageSelector)) + SelectorStart;
  wrapAround = false;
  for (;;) {
    nextSelector = OBJMEM_fetchPointer_ofObject(index, dictionary);
    if (nextSelector == NilPointer) return false;
    if (nextSelector == R_cls.messageSelector) {
      methodArray = OBJMEM_fetchPointer_ofObject(MethodArrayIndex, dictionary);
      R_cls.newMethod = OBJMEM_fetchPointer_ofObject(index - SelectorStart,
          methodArray);
      R_cls.primitiveIndex = primitiveIndexOf(R_cls.newMethod);
      return true;
    }
    index = index + 1;
    if (index == length) {
      if (wrapAround) return false;
      wrapAround = true;
      index = SelectorStart;
    }
  }
}
bool lookupMethodInClass(int cls) {
  int currentClass;
  int dictionary;
  currentClass = cls;
  while (currentClass != NilPointer) {
    dictionary = OBJMEM_fetchPointer_ofObject(MessageDictionaryIndex,
        currentClass);
    if (lookupMethodInDictionary(dictionary)) return true;
    currentClass = superclassOf(currentClass);
  }
  if (R_cls.messageSelector == DoesNotUnderstandSelector)
    error("Recursive not understood error encountered'");
  createActualMessage();
  R_cls.messageSelector = DoesNotUnderstandSelector;
  return lookupMethodInClass(cls);
}
int superclassOf(int classPointer) {
  return OBJMEM_fetchPointer_ofObject(SuperclassIndex, classPointer);
}
void createActualMessage() {
  int argumentArray;
  int message;
  argumentArray = OBJMEM_instantiateClass_withPointers(ClassArrayPointer,
      R_cls.argumentCount);
  message = OBJMEM_instantiateClass_withPointers(ClassMessagePointer,
      MessageSize);
  OBJMEM_storePointer_ofObject_withValue(MessageSelectorIndex, message,
      R_cls.messageSelector);
  OBJMEM_storePointer_ofObject_withValue(MessageArgumentsIndex, message,
      argumentArray);
  transfer_fromIndex_ofObject_toIndex_ofObject(R_cls.argumentCount,
      R_ctx.stackPointer - (R_cls.argumentCount - 1), R_ctx.activeContext, 0, argumentArray);
  pop(R_cls.argumentCount);
  push(message);
  R_cls.argumentCount = 1;
}
int instanceSpecificationOf(int classPointer) {
  return OBJMEM_fetchPointer_ofObject(InstanceSpecificationIndex, classPointer);
}

bool isPointers(int classPointer) {
  int pointersFlag;
  pointersFlag = extractBits_to_of(0, 0, instanceSpecificationOf(classPointer));
  return pointersFlag == 1;
}
bool isWords(int classPointer) {
  int wordsFlag;
  wordsFlag = extractBits_to_of(1, 1, instanceSpecificationOf(classPointer));
  return wordsFlag == 1;
}
bool isIndexable(int classPointer) {
  int indexableFlag;
  indexableFlag = extractBits_to_of(2, 2,
      instanceSpecificationOf(classPointer));
  return indexableFlag == 1;
}
int fixedFieldsOf(int classPointer) {
  return extractBits_to_of(4, 14, instanceSpecificationOf(classPointer));
}

