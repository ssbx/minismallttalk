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
#ifndef _CLASSES_H_
#define _CLASSES_H_
#include <stdbool.h>
/*
 *
 * Classes p586
 *
 */

int hash(int objectPointer);
bool lookupMethodInDictionary(int dictionary);
bool lookupMethodInClass(int cls);
int superclassOf(int classPointer);
void createActualMessage();
int instanceSpecificationOf(int classPointer);
bool isPointers(int classPointer);
bool isWords(int classPointer);
bool isIndexable(int classPointer);
int fixedFieldsOf(int classPointer);

#endif // _CLASSES_H_
