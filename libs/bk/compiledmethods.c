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
 * You should have recei
ved a copy of the GNU Lesser General Public License
 * along with Minist8. If not, see <https://www.gnu.org/licenses/>.
 */
#include "config.h"

#include "minist8.h"
#include "compiledmethods.h"
#include "routines.h"
#include "objectmemory.h"

int headerOf(int methodPointer) {
  return OBJMEM_fetchPointer_ofObject(HeaderIndex, methodPointer);
}
int literal_ofMethod(int offset, int methodPointer) {
  return OBJMEM_fetchPointer_ofObject(offset + LiteralStart, methodPointer);
}
int temporaryCountOf(int methodPointer) {
  return extractBits_to_of(3, 7, headerOf(methodPointer));
}
int largeContextFlagOf(int methodPointer) {
  return extractBits_to_of(8, 8, headerOf(methodPointer));
}
int literalCountOf(int methodPointer) {
  return literalCountOfHeader(headerOf(methodPointer));
}
int literalCountOfHeader(int headerPointer) {
  return extractBits_to_of(9, 14, headerPointer);
}
int objectPointerCountOf(int methodPointer) {
  return literalCountOf(methodPointer) + LiteralStart;
}

int initialInstructionPointerOfMethod(int methodPointer) {
  return (literalCountOf(methodPointer) + LiteralStart) * 2 + 1;
}

int flagValueOf(int methodPointer) {
  return extractBits_to_of(0, 2, headerOf(methodPointer));
}
int fieldIndexOf(int methodPointer) {
  return extractBits_to_of(3, 7, headerOf(methodPointer));
}
int headerExtensionOf(int methodPointer) {
  int literalCount;
  literalCount = literalCountOf(methodPointer);
  return literal_ofMethod(literalCount - 2, methodPointer);
}
int argumentCountOf(int methodPointer) {
  int flagValue;
  flagValue = flagValueOf(methodPointer);
  if (flagValue < 5) return flagValue;
  if (flagValue < 7) return 0;
  else return extractBits_to_of(2, 6, headerExtensionOf(methodPointer));
}
int primitiveIndexOf(int methodPointer) {
  int flagValue;
  flagValue = flagValueOf(methodPointer);
  if (flagValue == 7) {
    return extractBits_to_of(7, 14, headerExtensionOf(methodPointer));
  }
  return 0;
}

int methodClassOf(int methodPointer) {
  int literalCount;
  int association;
  literalCount = literalCountOf(methodPointer);
  association = literal_ofMethod(literalCount - 1, methodPointer);
  return OBJMEM_fetchPointer_ofObject(ValueIndex, association);
}

