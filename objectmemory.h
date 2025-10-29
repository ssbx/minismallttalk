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
#ifndef _OBJMEMORY_H_
#define _OBJMEMORY_H_

/*
 * Segmented Memory Model as described in G&R pg. 656 
 */
/*
 * realworldmem 
 */
#define RM_SEGCOUNT 16
#define RM_SEGSIZE 65536
static const int RealMem_SegmentCount = RM_SEGCOUNT;
static const int RealMem_SegmentSize = RM_SEGSIZE;

/*
 * uint16_t memory[RealMem_SegmentCount][RealMem_SegmentSize]; 
 */

int             RealMem_segment_word(int s, int w);
int             RealMem_segment_word_put(int s, int w, int value);
int             RealMem_segment_word_byte(int s, int w, int byteNumber);
int             RealMem_segment_word_byte_put(int s, int w, int byteNumber,
                                              int value);
int             RealMem_segment_word_bits_to(int s, int w, int firstBitIndex,
                                             int lastBitIndex);
int             RealMem_segment_word_bits_to_put(int s, int w,
                                                 int firstBitIndex,
                                                 int lastBitIndex, int value);

#include <stdint.h>
#include <stdbool.h>



#ifdef GC_MARK_SWEEP
struct gc_notif_i {
    void            (*prepareForCollection)();
    void            (*collectionCompleted)();
};

void            OBJMEM_init(struct gc_notif_i);
void            OBJMEM_addRoot(int);
#else
void            OBJMEM_init();
#endif

uint32_t        OBJMEM_coreLeft();
void            OBJMEM_decreaseReferencesTo(int);
int             OBJMEM_fetchByteLengthOf(int);
int             OBJMEM_fetchByte_ofObject(int, int);
int             OBJMEM_fetchClassOf(int);
int             OBJMEM_fetchPointer_ofObject(int, int);
int             OBJMEM_fetchWordLengthOf(int);
int             OBJMEM_fetchWord_ofObject(int, int);
void            OBJMEM_garbageCollect();
bool            OBJMEM_hasObject(int);
void            OBJMEM_increaseReferencesTo(int);
int             OBJMEM_initialInstanceOf(int);
int             OBJMEM_instanceAfter(int);
int             OBJMEM_instantiateClass_withBytes(int, int);
int             OBJMEM_instantiateClass_withPointers(int, int);
int             OBJMEM_instantiateClass_withWords(int, int);
int             OBJMEM_integerObjectOf(int);
int             OBJMEM_integerValueOf(int);
bool            OBJMEM_isIntegerObject(int);
bool            OBJMEM_isIntegerValue(int);
bool            OBJMEM_loadSnapshot(const char *);
int             OBJMEM_oopsLeft();
bool            OBJMEM_saveSnapshot(const char *);
int             OBJMEM_storeByte_ofObject_withValue(int, int, int);
int             OBJMEM_storePointer_ofObject_withValue(int, int, int);
int             OBJMEM_storeWord_ofObject_withValue(int, int, int);
void            OBJMEM_swapPointersOf_and(int, int);

#endif                          /* _OBJMEMORY_H_ */
