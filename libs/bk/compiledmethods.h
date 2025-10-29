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
#ifndef _COMPILED_METHODS_H_
#define _COMPILED_METHODS_H_
/*
 * Compiled Methods p576
 */

int headerOf(int methodPointer);
int literal_ofMethod(int offset, int methodPointer);
int temporaryCountOf(int methodPointer);
int largeContextFlagOf(int methodPointer);
int literalCountOf(int methodPointer);
int literalCountOfHeader(int headerPointer);
int objectPointerCountOf(int methodPointer);
int initialInstructionPointerOfMethod(int methodPointer);

/* flagvalue 
    0-4: no prim and 0 to 4 args
    5: prim return of self (0 args)
    6: primitive return of an inst var (0 args)
    7: header extension contains the number of args and prim index */
int flagValueOf(int methodPointer);
int fieldIndexOf(int methodPointer);
int headerExtensionOf(int methodPointer);
int argumentCountOf(int methodPointer);
int primitiveIndexOf(int methodPointer);
int methodClassOf(int methodPointer);


#endif // _COMPILED_METHODS_H_
