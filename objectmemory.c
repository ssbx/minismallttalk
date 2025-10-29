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

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


#include "minist8.h"
#include "system.h"
#include "objectmemory.h"


#ifndef GC_REF_COUNT
#ifndef GC_MARK_SWEEP
#error "must define GC_REF_COUNT and/or GC_MARK_SWEEP"
#endif
#endif

/*
 * -------------------------------------------------------------------------
 */
/*
 * realwordmemory ----------------------------------------------------------
 */
/*
 * -------------------------------------------------------------------------
 */
// Segmented Memory Model as described in G&R pg. 656
uint16_t        ReamMem_memory[RM_SEGCOUNT][RM_SEGSIZE];

int
RealMem_segment_word(int s, int w)
{
    assert(s >= 0 && s < RealMem_SegmentCount);
    assert(w >= 0 && w < RealMem_SegmentSize);
    return ReamMem_memory[s][w];
}

int
RealMem_segment_word_put(int s, int w, int value)
{
    assert(s >= 0 && s < RealMem_SegmentCount);
    assert(w >= 0 && w < RealMem_SegmentSize);
    assert(value >= 0 && value < 65536);
    ReamMem_memory[s][w] = value;
    return value;
}

int
RealMem_segment_word_byte(int s, int w, int byteNumber)
{
    assert(s >= 0 && s < RealMem_SegmentCount);
    assert(w >= 0 && w < RealMem_SegmentSize);
    assert(byteNumber == 1 || byteNumber == 0);

    // byteNumber 0 is the byte of the word that appears first in member, 1 is
    // the next
    /*
     * big endian
     */
    // return (ReamMem_memory[s][w] >> (8*byteNumber)) & 0xff;
    return ((uint8_t *) & ReamMem_memory[s][w])[byteNumber];
    /*
     * little endian
     */
    // return ((std::uint8_t *) &ReamMem_memory[s][w])[1-byteNumber];
}

int
RealMem_segment_word_byte_put(int s, int w, int byteNumber, int value)
{
    assert(s >= 0 && s < RealMem_SegmentCount);
    assert(w >= 0 && w < RealMem_SegmentSize);
    assert(value >= 0 && value < 65536);
    assert(byteNumber == 1 || byteNumber == 0);

    // byteNumber 0 is the byte of the word that appears first in member, 1 is
    // the next
    /*
     * big endian
     */
    ((uint8_t *) & ReamMem_memory[s][w])[byteNumber] = value;
    /*
     * little endian
     */
    // ((uint8_t *) &ReamMem_memory[s][w])[1-byteNumber] = value;
    return value;
}

// The most significant bit in a word will be referred to with the index 0 and
// the least significant with the index 15. G&R 657
int
RealMem_segment_word_bits_to(int s, int w, int firstBitIndex, int lastBitIndex)
{
    assert(s >= 0 && s < RealMem_SegmentCount);
    assert(w >= 0 && w < RealMem_SegmentSize);

    uint16_t        shift = ReamMem_memory[s][w] >> (15 - lastBitIndex);
    uint16_t        mask = (1 << (lastBitIndex - firstBitIndex + 1)) - 1;

    return shift & mask;
}

int
RealMem_segment_word_bits_to_put(int s, int w, int firstBitIndex,
                                 int lastBitIndex, int value)
{
    assert(s >= 0 && s < RealMem_SegmentCount);
    assert(w >= 0 && w < RealMem_SegmentSize);
    assert(value >= 0 && value < 65536);

    uint16_t        mask = (1 << (lastBitIndex - firstBitIndex + 1)) - 1;
    assert((value & mask) == value);    // make sure it fits
    ReamMem_memory[s][w] =
        (ReamMem_memory[s][w] & ~(mask << (15 - lastBitIndex)))
        | (value << (15 - lastBitIndex));
    return value;
}

/*
 * -------------------------------------------------------------------------
 */
/*
 * objectmemory ----------------------------------------------------------
 */
/*
 * -------------------------------------------------------------------------
 */

// ObjectMemory *single = NULL;
// Special Register G&R pg. 667
int             currentSegment; // The index of the heap segment currently
                                // being used for allocation

int             freeWords;      // free words remaining (make primitiveFreeCore
                                // "fast")

// An a table entry with a free bit set OR that contains a reference to a free
// chunk
// (free bit clear but count field zero) of memory is counted as a free oop
int             freeOops;       // free OT entries (make primitiveFreeOops
                                // "fast")

// G&R pg. 664 - Object Table Related Constants
// Object Table Segment (last segment) contains the Object Table followed by
// the
// head of the OT free pointer list
// +-------------------------+
// | | <--- ObjectTableStart
// | |
// | |
// | Object Table |
// | |
// | |
// +-------------------------+
// | FreePointerList |
// +-------------------------+
// |////// UNUSED WORD //////|
// +-------------------------+

static const int ObjectTableSegment = RealMem_SegmentCount - 1;
static const int ObjectTableStart = 0;
static const int ObjectTableSize = RealMem_SegmentSize - 2;
// The smallest number that is too large to represent in an eight-bit count
// field; that is, 256.
static const int HugeSize = 256;        // G&R pg 661

// The location of the head of the linked list of free object table entries
static const int FreePointerList = ObjectTableStart + ObjectTableSize;  // G&R
                                                                        // pg.
                                                                        // 664

// G&R pg. 664 - Object Table Related Constants
// The smallest size of chunk that is not stored on a list whose chunk share
// the same size.
// (Theindex of the last free chunk list).
static const int BigSize = 20;
static const int FirstFreeChunkListSize = BigSize + 1;

// Heap Constants G&R pg. 658

// The number of heaps segments used in the implementation.
// We reserve the last segment for the Object Table and use the remaining for
// the heap
static const int HeapSegmentCount = RealMem_SegmentCount - 1;

// Each heap segment is organized as follows:
// +-------------------------+
// | |
// | |
// | Object Storage |
// | |
// | |<--- HeapSpaceStop (last word)
// +-------------------------+
// | Array of BigSize+1 |<--- FirstFreeChunkList
// | Free Chunks Linked |
// | List Heads |
// | |<--- LastFreeChunkList
// +-------------------------+

// The index of the first memory segmentused to store the heap
static const int FirstHeapSegment = 0;
static const int LastHeapSegment = FirstHeapSegment + HeapSegmentCount - 1;

// The address of the last location used in each heap segment.
static const int HeapSpaceStop = RealMem_SegmentSize - FirstFreeChunkListSize
    - 1;
static const int HeaderSize = 2;        // The number of words in an object
                                        // header(2).
// If HeaderSize changes, revisit forAllOtherObjectsAccessibleFrom_suchThat_do
// where we test if the offset passes the class field...

// The location of the head of the linked list of free chunks of size zero.
// Comes right
// after the last word for object storage.
static const int FirstFreeChunkList = HeapSpaceStop + 1;

// The bluebook incorrectly uses LastFreeChunkList in all places it is used!
// The
// headOfFreeChunkList:inSegment: and headOfFreeChunkList:inSegment:put methods
// take
// a SIZE as the first parameter not a location.
// The location of the head of the linked list of free chunks of size BigSize
// or larger.
// static const int LastFreeChunkList = FirstFreeChunkList + BigSize;

// Any sixteen-bit value that cannot be an object table index, e.g.,2**16~1.
static const int NonPointer = 65535;
// Last special oop
// (See SystemTracer in Smalltalk.sources)
static const int LastSpecialOop = 52;

/*
 * ==========================================================================
 */
/*
 * ==========================================================================
 */
/*
 * ==========================================================================
 */
bool            OBJMEM_loadSnapshot(const char *imageFileName);
bool            OBJMEM_saveSnapshot(const char *imageFileName);

// --- BCIInterface ---

int             OBJMEM_oopsLeft();

uint32_t        OBJMEM_coreLeft();

void            OBJMEM_garbageCollect();

// storePointer:ofObject:withValue:
int             OBJMEM_storePointer_ofObject_withValue(int fieldIndex,
                                                       int objectPointer,
                                                       int valuePointer);

// storeWord:ofObject:withValue:
int             OBJMEM_storeWord_ofObject_withValue(int wordIndex,
                                                    int objectPointer,
                                                    int valueWord);

// increaseReferencesTo:
void            OBJMEM_increaseReferencesTo(int objectPointer);

// initialInstanceOf:
int             OBJMEM_initialInstanceOf(int classPointer);

// decreaseReferencesTo:
void            OBJMEM_decreaseReferencesTo(int objectPointer);

// isIntegerValue:
bool            OBJMEM_isIntegerValue(int valueWord);

// fetchWord:ofObject:
int             OBJMEM_fetchWord_ofObject(int wordIndex, int objectPointer);

// integerValueOf:
int             OBJMEM_integerValueOf(int objectPointer);

// swapPointersOf:and:
void            OBJMEM_swapPointersOf_and(int firstPointer, int secondPointer);

// fetchWordLengthOf:
int             OBJMEM_fetchWordLengthOf(int objectPointer);

// instantiateClass:withWords:
int             OBJMEM_instantiateClass_withWords(int classPointer, int length);

// isIntegerObject:
bool            OBJMEM_isIntegerObject(int objectPointer);

// instantiateClass:withBytes:
int             OBJMEM_instantiateClass_withBytes(int classPointer, int length);

// hasObject:
bool            OBJMEM_hasObject(int objectPointer);

// instantiateClass:withPointers:
int             OBJMEM_instantiateClass_withPointers(int classPointer,
                                                     int length);

// fetchByte:ofObject:
int             OBJMEM_fetchByte_ofObject(int byteIndex, int objectPointer);

// fetchPointer:ofObject:
int             OBJMEM_fetchPointer_ofObject(int fieldIndex, int objectPointer);

// fetchClassOf:
int             OBJMEM_fetchClassOf(int objectPointer);

// integerObjectOf:
int             OBJMEM_integerObjectOf(int value);

// fetchByteLengthOf:
int             OBJMEM_fetchByteLengthOf(int objectPointer);

// instanceAfter:
int             OBJMEM_instanceAfter(int objectPointer);

// storeByte:ofObject:withValue:
int             OBJMEM_storeByte_ofObject_withValue(int byteIndex,
                                                    int objectPointer,
                                                    int valueByte);

// --- ObjectPointers ---

// cantBeIntegerObject:
void            OBJMEM_cantBeIntegerObject(int objectPointer);

#ifdef GC_MARK_SWEEP
void            OBJMEM_addRoot(int rootObjectPointer);
#endif

// --- Compaction ---

// sweepCurrentSegmentFrom:
int             OBJMEM_sweepCurrentSegmentFrom(int lowWaterMark);

// compactCurrentSegment
void            OBJMEM_compactCurrentSegment();

// releasePointer:
void            OBJMEM_releasePointer(int objectPointer);

// reverseHeapPointersAbove:
void            OBJMEM_reverseHeapPointersAbove(int lowWaterMark);

// abandonFreeChunksInSegment:
int             OBJMEM_abandonFreeChunksInSegment(int segment);

// allocateChunk:
int             OBJMEM_allocateChunk(int size);

#ifdef GC_MARK_SWEEP
// --- MarkingGarbage ---

// reclaimInaccessibleObjects
void            OBJMEM_reclaimInaccessibleObjects();

// markObjectsAccessibleFrom:
int             OBJMEM_markObjectsAccessibleFrom(int rootObjectPointer);

// markAccessibleObjects
void            OBJMEM_markAccessibleObjects();

// rectifyCountsAndDeallocateGarbage
void            OBJMEM_rectifyCountsAndDeallocateGarbage();

// zeroReferenceCounts
void            OBJMEM_zeroReferenceCounts();

#endif

// --- NonpointerObjs ---

// lastPointerOf:
int             OBJMEM_lastPointerOf(int objectPointer);

// spaceOccupiedBy:
int             OBJMEM_spaceOccupiedBy(int objectPointer);

// allocate:odd:pointer:extra:class:
int             OBJMEM_allocate_odd_pointer_extra_class(int size, int oddBit,
                                                        int pointerBit,
                                                        int extraWord,
                                                        int classPointer);

// --- UnallocatedSpc ---

// headOfFreePointerList
int             OBJMEM_headOfFreePointerList();

// toFreeChunkList:add:
void            OBJMEM_toFreeChunkList_add(int size, int objectPointer);

// headOfFreeChunkList:inSegment:put:
int             OBJMEM_headOfFreeChunkList_inSegment_put(int size, int segment,
                                                         int objectPointer);

// removeFromFreePointerList
int             OBJMEM_removeFromFreePointerList();

// toFreePointerListAdd:
void            OBJMEM_toFreePointerListAdd(int objectPointer);

// removeFromFreeChunkList:
int             OBJMEM_removeFromFreeChunkList(int size);

// resetFreeChunkList:inSegment:
void            OBJMEM_resetFreeChunkList_inSegment(int size, int segment);

// headOfFreeChunkList:inSegment:
int             OBJMEM_headOfFreeChunkList_inSegment(int size, int segment);

// headOfFreePointerListPut:
int             OBJMEM_headOfFreePointerListPut(int objectPointer);

// --- RefCntGarbage ---

// countDown:
int             OBJMEM_countDown(int rootObjectPointer);

// countUp:
int             OBJMEM_countUp(int objectPointer);

// deallocate:
void            OBJMEM_deallocate(int objectPointer);

/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
/*
 * =========================================================================
 */

/* *INDENT-OFF* */
int             OBJMEM_forAllOtherObjectsAccessibleFrom_suchThat_do(
                                                        int objectPointer,
                                                        bool (*predicate) (int),
                                                        void(*action)(int));
// OBJMEM_forAllObjectsAccessibleFrom:suchThat:do:
int             forAllObjectsAccessibleFrom_suchThat_do(int objectPointer,
                                                        bool (*predicate)(int),
                                                        void(*action)(int));
/* *INDENT-ON* */

/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
/*
 * =========================================================================
 */

// --- ObjectTableEnt ---
// segmentBitsOf:
int             OBJMEM_segmentBitsOf(int objectPointer);

// heapChunkOf:byte:put:
int             OBJMEM_heapChunkOf_byte_put(int objectPointer, int offset,
                                            int value);

// pointerBitOf:put:
int             OBJMEM_pointerBitOf_put(int objectPointer, int value);

// heapChunkOf:word:
int             OBJMEM_heapChunkOf_word(int objectPointer, int offset);

// segmentBitsOf:put:
int             OBJMEM_segmentBitsOf_put(int objectPointer, int value);

// heapChunkOf:word:put:
int             OBJMEM_heapChunkOf_word_put(int objectPointer, int offset,
                                            int value);

// oddBitOf:
int             OBJMEM_oddBitOf(int objectPointer);

// freeBitOf:
int             OBJMEM_freeBitOf(int objectPointer);

// locationBitsOf:
int             OBJMEM_locationBitsOf(int objectPointer);

// ot:
int             OBJMEM_ot(int objectPointer);

// freeBitOf:put:
int             OBJMEM_freeBitOf_put(int objectPointer, int value);

// classBitsOf:put:
int             OBJMEM_classBitsOf_put(int objectPointer, int value);

// heapChunkOf:byte:
int             OBJMEM_heapChunkOf_byte(int objectPointer, int offset);

// locationBitsOf:put:
int             OBJMEM_locationBitsOf_put(int objectPointer, int value);

// sizeBitsOf:
int             OBJMEM_sizeBitsOf(int objectPointer);

// oddBitOf:put:
int             OBJMEM_oddBitOf_put(int objectPointer, int value);

// ot:put:
int             OBJMEM_ot_put(int objectPointer, int value);

// countBitsOf:put:
int             OBJMEM_countBitsOf_put(int objectPointer, int value);

// classBitsOf:
int             OBJMEM_classBitsOf(int objectPointer);

// countBitsOf:
int             OBJMEM_countBitsOf(int objectPointer);

// ot:bits:to:put:
int             OBJMEM_ot_bits_to_put(int objectPointer, int firstBitIndex,
                                      int lastBitIndex, int value);

// sizeBitsOf:put:
int             OBJMEM_sizeBitsOf_put(int objectPointer, int value);

// ot:bits:to:
int             OBJMEM_ot_bits_to(int objectPointer, int firstBitIndex,
                                  int lastBitIndex);

// pointerBitOf:
int             OBJMEM_pointerBitOf(int objectPointer);

// --- Allocation ---

// obtainPointer:location:
int             OBJMEM_obtainPointer_location(int size, int location);

// attemptToAllocateChunk:
int             OBJMEM_attemptToAllocateChunk(int size);

// attemptToAllocateChunkInCurrentSegment:
int             OBJMEM_attemptToAllocateChunkInCurrentSegment(int size);

void            OBJMEM_outOfMemoryError();

int             OBJMEM_auditFreeOops();

#ifdef RUNTIME_CHECKING
void            OBJMEM_runtime_check(bool condition, const char *errorMessage);
#endif

bool            OBJMEM_loadObjectTable(int fd);
static bool     OBJMEM_padToPage(int fd);
bool            OBJMEM_loadObjects(int fd);
bool            OBJMEM_saveObjects(int fd);

/*
 * ==========================================================================
 */
/*
 * ==========================================================================
 */
/*
 * ==========================================================================
 */
/*
 * ==========================================================================
 */

// Snapshots
// Object space starts at offset 512 in the image
static const int ObjectSpaceBaseInImage = 512;

#ifdef GC_MARK_SWEEP
struct gc_notif_i gcNotification;
bool            haveGcNotification = false;
#endif

#ifdef GC_MARK_SWEEP
void
OBJMEM_init(struct gc_notif_i notif)
{
    currentSegment = -1;
    freeWords = 0;
    freeOops = 0;
    gcNotification = notif;
    haveGcNotification = true;
}
#else
void
OBJMEM_init()
{
    currentSegment = -1;
    freeWords = 0;
    freeOops = 0;
}
#endif

/* *INDENT-OFF* */
int
OBJMEM_forAllObjectsAccessibleFrom_suchThat_do(int objectPointer,
                                               bool (*predicate)(int),
                                               void (*action)(int));
/* * INDENT-ON* */

bool
OBJMEM_loadObjectTable(int fd)
{
    // First two 32-bit values have the object space length and object table
    // lengths in words
    int32_t         objectTableLength;

    if (System_SeekFileTo(fd, 4) == -1) // Skip over object space length
        return false;
    if (System_ReadFile
        (fd, (char *) &objectTableLength, sizeof(objectTableLength))
        != sizeof(objectTableLength))
        return false;
    int             fileSize = System_FileSize(fd);

    if (System_SeekFileTo(fd, fileSize - objectTableLength * 2) == -1)  // Reposition
                                                                        //
        // to
        // start
        // of
        // object
        // table
        return false;

    for (int objectPointer = 0; objectPointer < objectTableLength;
         objectPointer += 2) {
        uint16_t        words[2];
        if (System_ReadFile(fd, (char *) &words, sizeof(words)) !=
            sizeof(words))
            return false;
        OBJMEM_ot_put(objectPointer, words[0]);
        OBJMEM_locationBitsOf_put(objectPointer, words[1]);
    }

    OBJMEM_headOfFreePointerListPut(NonPointer);

    // Initialize the remaining entries as free
    for (int objectPointer = objectTableLength; objectPointer < ObjectTableSize;
         objectPointer += 2) {
        OBJMEM_ot_put(objectPointer, 0);
        OBJMEM_freeBitOf_put(objectPointer, 1);
        OBJMEM_locationBitsOf_put(objectPointer, 0);
    }

    // Build the OT entry free list, Go backwards so we have lower entries
    // first on free list.
    // Why? Makes the OT easier to see in debugger.
    // Note we skip oop 0, which is considered reserved and invalid. (page 2,
    // Xerox Virtual Image booklet)
    for (int objectPointer = ObjectTableSize - 2; objectPointer >= 2;
         objectPointer -= 2)
        if (OBJMEM_freeBitOf(objectPointer))
            OBJMEM_toFreePointerListAdd(objectPointer);

    freeOops = OBJMEM_auditFreeOops();

    return true;
}

bool
OBJMEM_loadObjects(int fd)
{
    static const int SegmentHeapSpaceSize = HeapSpaceStop + 1;

    // Track amount of free space available for objects in each segment
    int             heapSpaceRemaining[HeapSegmentCount];

    for (int segment = FirstHeapSegment; segment <= LastHeapSegment; segment++)
        heapSpaceRemaining[segment - FirstHeapSegment] = SegmentHeapSpaceSize;

    // Load objects from the virtual image into the heap segments
    // being careful to not split an object across a segment boundary
    int             destinationSegment = FirstHeapSegment,
        destinationWord = 0;

    for (int objectPointer = 2; objectPointer < ObjectTableSize;
         objectPointer += 2) {
        if (OBJMEM_freeBitOf(objectPointer))
            continue;
        // A free chunk has it's COUNT field set to zero but the free bit is
        // clear
        assert(OBJMEM_countBitsOf(objectPointer) != 0); // SANITY Make sure a
        // freeChunk wasn't
        // saved!

        // On disk objects are stored contiguously as if a large 20-bit WORD
        // addressed space
        // In this scheme, the OT segment and locations combine to form a WORD
        // address
        const int       objectImageWordAddress =
            (OBJMEM_segmentBitsOf(objectPointer)
             << 16) + OBJMEM_locationBitsOf(objectPointer);

        System_SeekFileTo(fd,
                          ObjectSpaceBaseInImage +
                          objectImageWordAddress * sizeof(uint16_t));

        uint16_t        objectSize;
        System_ReadFile(fd, (char *) &objectSize, sizeof(objectSize));

        // Account for the extra word used by HugeSize objects
        int             extraSpace =
            objectSize < HugeSize || OBJMEM_pointerBitOf(objectPointer) == 0 ?
            0 : 1;
        int             space = objectSize + extraSpace;        // space in
        // memory

        if (space > heapSpaceRemaining[destinationSegment - FirstHeapSegment]) {
            // No room left in the current segment, move to next
            destinationSegment++;
            if (destinationSegment == HeapSegmentCount)
                return false;   // Full
            destinationWord = 0;
        }
        // Update OT entry so that it references the object location in
        // ObjectMemory vs the disk image
        OBJMEM_segmentBitsOf_put(objectPointer, destinationSegment);
        OBJMEM_locationBitsOf_put(objectPointer, destinationWord);

        // Store the object in the image into word memory
        // First comes the size...

        OBJMEM_sizeBitsOf_put(objectPointer, objectSize);

        // Next is the class...
        uint16_t        classBits;
        System_ReadFile(fd, (char *) &classBits, sizeof(classBits));

        OBJMEM_classBitsOf_put(objectPointer, classBits);

        // Followed by the fields...
        for (int wordIndex = 0; wordIndex < objectSize - HeaderSize;
             wordIndex++) {
            uint16_t        word;
            System_ReadFile(fd, (char *) &word, sizeof(word));
            // use heap chunk
            OBJMEM_storeWord_ofObject_withValue(wordIndex, objectPointer, word);
        }

        destinationWord += space;

        heapSpaceRemaining[destinationSegment - FirstHeapSegment] -= space;
    }

    // Initialize the free chunk lists for each heap segment with the sentinel
    for (int segment = FirstHeapSegment; segment <= LastHeapSegment; segment++) {
        for (int size = HeaderSize; size <= BigSize; size++)
            OBJMEM_resetFreeChunkList_inSegment(size, segment);
    }

    freeWords = 0;
    // Place any remaining space in each segment onto it's free chunk list,
    // which is
    // is a linked list of object pointers.
    // The chunks are linked using the class field of an object. The size field
    //
    // of
    // the object contains the actual size of the free chunk.
    for (int segment = FirstHeapSegment; segment <= LastHeapSegment; segment++) {
        int             freeChunkSize =
            heapSpaceRemaining[segment - FirstHeapSegment];
        freeWords += freeChunkSize;
        if (freeChunkSize >= HeaderSize) {
            int             freeChunkLocation =
                SegmentHeapSpaceSize - freeChunkSize;
            // G&R pg 665 - each free chunk has an OT entry
            currentSegment = segment;   // Set special segment register
            int             objectPointer =
                OBJMEM_obtainPointer_location(freeChunkSize,
                                              freeChunkLocation);
            OBJMEM_toFreeChunkList_add(((freeChunkSize <
                                         (int) BigSize) ? freeChunkSize :
                                        BigSize), objectPointer);
        }
    }

    currentSegment = FirstHeapSegment;

    return true;

}

bool
OBJMEM_loadSnapshot(const char *fileName)
{
    int             fd = System_OpenFile(fileName);
    if (fd == -1)
        return false;

    bool            succeeded = OBJMEM_loadObjectTable(fd) &&
        OBJMEM_loadObjects(fd);

    System_CloseFile(fd);

    return succeeded;
}

bool
OBJMEM_padToPage(int fd)
{
    int             pos = System_TellFile(fd);
    int             desired = ((pos + 512 - 1) / 512) * 512;
    uint16_t        word = 0;
    int             pad = (desired - pos) / sizeof(word);
    while (pad-- > 0) {
        if (System_WriteFile(fd, (char *) &word, sizeof(word)) != sizeof(word))
            return false;
    }

    return true;
}

bool
OBJMEM_saveSnapshot(const char *imageFileName)
{

    int             fd = System_CreateFile(imageFileName);
    if (fd == -1)
        return false;
    bool            success = OBJMEM_saveObjects(fd);

    System_CloseFile(fd);
    return success;

}

bool
OBJMEM_saveObjects(int fd)
{
    // Avoid dumping out the entire object table -- we only need to write
    // entries up until
    // the last OT entry that references an object
    int             lastUsedObjectPointer = NonPointer;
    for (int objectPointer = 2; objectPointer < ObjectTableSize;
         objectPointer += 2) {
        if (OBJMEM_hasObject(objectPointer))
            lastUsedObjectPointer = objectPointer;
    }

    int             storedObjectTableLength = lastUsedObjectPointer + 2;

    int32_t         placeHolder[2] = { 0 };

    // Write place holder value for object space length and object table length
    if (System_WriteFile(fd, (char *) &placeHolder, sizeof(placeHolder))
        != sizeof(placeHolder))
        return false;

    // Write two zero bytes indicating interchange format
    uint8_t         interchange[2] = { 0 };
    if (System_WriteFile(fd, (char *) &interchange, sizeof(interchange))
        != sizeof(interchange))
        return false;

    if (!OBJMEM_padToPage(fd))  // Advance to next page before writing objects
        return false;

    // Write objects
    int32_t         objectSpaceLength = 0;
    for (int objectPointer = 2; objectPointer < storedObjectTableLength;
         objectPointer += 2) {
        if (!OBJMEM_hasObject(objectPointer))
            continue;

        // Write object to file... N.B. we do not store the extra word for
        // HugeSize objects
        uint16_t        header[2];
        uint16_t        objectSize = OBJMEM_sizeBitsOf(objectPointer);
        header[0] = objectSize;
        header[1] = (uint16_t) OBJMEM_fetchClassOf(objectPointer);
        if (System_WriteFile(fd, (char *) &header, sizeof(header)) !=
            sizeof(header))
            return false;
        int             wordLengthOfObject =
            OBJMEM_fetchWordLengthOf(objectPointer);
        for (int wordIndex = 0; wordIndex < wordLengthOfObject; wordIndex++) {
            uint16_t        word =
                (uint16_t) OBJMEM_fetchWord_ofObject(wordIndex,
                                                     objectPointer);
            if (System_WriteFile(fd, (char *) &word, sizeof(word)) !=
                sizeof(word))
                return false;
        }

        objectSpaceLength += objectSize;
    }


    if (!OBJMEM_padToPage(fd))  // Advance to next page before writing object
        // table
        return false;

    // Write object table
    int             objectImageWordAddress = 0;
    for (int objectPointer = 0; objectPointer < storedObjectTableLength;
         objectPointer += 2) {
        uint16_t        oldOTValue = OBJMEM_ot(objectPointer);
        uint16_t        oldOTLocation = OBJMEM_locationBitsOf(objectPointer);

        if (objectPointer >= 2) {
            if (!OBJMEM_freeBitOf(objectPointer)
                && OBJMEM_countBitsOf(objectPointer) == 0) {
                // This entry was for a free chunk of memory, but we don't save
                // free space in the image. Store as a
                // an available OT entry by setting free bit
                OBJMEM_freeBitOf_put(objectPointer, 1);
            }

            if (OBJMEM_freeBitOf(objectPointer)) {
                // manual.pdf - page 3: free entries have freeBit set and other
                //
                // bits in both
                // words are 0.
                OBJMEM_ot_put(objectPointer, 0);
                OBJMEM_freeBitOf_put(objectPointer, 1);
                OBJMEM_locationBitsOf_put(objectPointer, 0);
            } else {
                uint16_t        objectSize =
                    (uint16_t) OBJMEM_sizeBitsOf(objectPointer);

                // Modify the location of the object table entry... we do this
                // once we no longer
                // process this object table entry
                OBJMEM_segmentBitsOf_put(objectPointer,
                                         objectImageWordAddress >> 16);
                OBJMEM_locationBitsOf_put(objectPointer,
                                          objectImageWordAddress & 0xffff);
                objectImageWordAddress += objectSize;
            }
        }
        // Assemble object table entry
        uint16_t        words[2];
        words[0] = OBJMEM_ot(objectPointer);
        words[1] = OBJMEM_locationBitsOf(objectPointer);

        // Restore OT entry
        OBJMEM_ot_put(objectPointer, oldOTValue);
        OBJMEM_locationBitsOf_put(objectPointer, oldOTLocation);

        // Write this entry
        if (System_WriteFile(fd, (char *) &words, sizeof(words)) !=
            sizeof(words))
            return false;

    }

    // Now we can go back fill in the values for the image header.
    System_SeekFileTo(fd, 0);
    System_WriteFile(fd, (char *) &objectSpaceLength,
                     sizeof(objectSpaceLength));
    System_WriteFile(fd, (char *) &storedObjectTableLength,
                     sizeof(storedObjectTableLength));

    return true;
}

// sweepCurrentSegmentFrom:
int
OBJMEM_sweepCurrentSegmentFrom(int lowWaterMark)
{
    int             si;
    int             di;
    int             objectPointer;
    int             size;

    /*
     * "source" "ERROR dbanay - unused local 'space'" si <- di <- lowWaterMark.
     * [si < HeapSpaceStop] whileTrue: "for each object, si" [(wordMemory
     * segment: currentSegment word: si + 1) = NonPointer ifTrue: "unallocated,
     * so skip it" [size <- wordMemory segment: currentSegment word: si. si <-
     * si + size] ifFalse: "allocated, so keep it, but move it to compact
     * storage" [objectPointer <- wordMemory segment: currentSegment word: si.
     * size <- self locationBitsOf: objectPointer. "the reversed size" self
     * locationBitsOf: objectPointer put: di. "point object table at new
     * location" self sizeBitsOf: objectPointer put: size. "restore the size to
     * its proper place" si <- si + 1. "skip the size" di <- di + 1. "skip the
     * size" 2 to: (self spaceOccupiedBy: objectPointer) do: "move the rest of
     * the object" [ :i | wordMemory segment: currentSegment word: di put:
     * (wordMemory segment: currentSegment word: si). si <- si + 1. di <- di +
     * 1]]]. ^di
     */

    si = di = lowWaterMark;
    while (si < HeapSpaceStop)  // for each object, si
    {
        if (RealMem_segment_word(currentSegment, si + 1) == NonPointer) {
            // Unallocated, so skip it (see abandonFreeChunksInSegment)
            size = RealMem_segment_word(currentSegment, si);
            si = si + size;
        } else {
            // allocated, so keep it, but move it to compact storage
            objectPointer = RealMem_segment_word(currentSegment, si);   // reversed
                                                                        //
            // pointer!
            size = OBJMEM_locationBitsOf(objectPointer);        // the reversed
                                                                //
            // size (stored
            // during
            // pointer
            // reversal
            // step)
            OBJMEM_locationBitsOf_put(objectPointer, di);       // point object
                                                                //
            // table at new
            // location
            // (di)
            OBJMEM_sizeBitsOf_put(objectPointer, size); // restore the size to
            // its proper place
            si++;               // skip the size header
            di++;               // skip the size header (size we wrote to di
            // via sizeBitsOf_put)
            int             limit = OBJMEM_spaceOccupiedBy(objectPointer);
            for (int i = 2; i <= limit; i++)    // move the rest of the object
                // (already wrote size via
                // sizeBitsOf_put)
            {
                RealMem_segment_word_put(currentSegment, di,
                                         RealMem_segment_word(currentSegment,
                                                              si));
                si++;
                di++;

            }
        }
    }

    return di;

}

// compactCurrentSegment
void
OBJMEM_compactCurrentSegment()
{
    int             lowWaterMark;
    int             bigSpace;

    /*
     * "source" lowWaterMark <- self abandonFreeChunksInSegment:
     * currentSegment. lowWaterMark < HeapSpaceStop ifTrue: [self
     * reverseHeapPointersAbove: lowWaterMark. bigSpace <- self
     * sweepCurrentSegmentFrom: lowWaterMark. self deallocate: (self
     * obtainPointer: (HeapSpaceStop + 1 - bigSpace) location: bigSpace)]
     */

    RUNTIME_CHECK(currentSegment >= FirstHeapSegment &&
                  currentSegment <= LastHeapSegment);

    lowWaterMark = OBJMEM_abandonFreeChunksInSegment(currentSegment);
    if (lowWaterMark < HeapSpaceStop) {
        OBJMEM_reverseHeapPointersAbove(lowWaterMark);
        bigSpace = OBJMEM_sweepCurrentSegmentFrom(lowWaterMark);
        OBJMEM_deallocate(OBJMEM_obtainPointer_location
                          (HeapSpaceStop + 1 - bigSpace, bigSpace));
    }
}

// Force a garbage collection
void
OBJMEM_garbageCollect()
{
#ifdef GC_MARK_SWEEP
    OBJMEM_reclaimInaccessibleObjects();
#endif
}

// releasePointer:
void
OBJMEM_releasePointer(int objectPointer)
{
    /*
     * "source" self freeBitOf: objectPointer put: 1. self
     * toFreePointerListAdd: objectPointer
     */
    OBJMEM_freeBitOf_put(objectPointer, 1);
    OBJMEM_toFreePointerListAdd(objectPointer);
}

// reverseHeapPointersAbove:
void
OBJMEM_reverseHeapPointersAbove(int lowWaterMark)
{
    int             size;

    /*
     * "source" 0 to: ObjectTableSize-2 by: 2 do: [ :objectPointer | (self
     * freeBitOf: objectPointer) = 0 ifTrue: "the Object Table entry is in use"
     * [(self segmentBitsOf: objectPointer) = currentSegment ifTrue: "the object
     * is in this segment" [(self locationBitsOf: objectPointer) < lowWaterMark
     * ifFalse: "the object will be swept" [size <- self sizeBitsOf:
     * objectPointer. "rescue the size" self sizeBitsOf: objectPointer put:
     * objectPointer. "reverse the pointer" self locationBitsOf: objectPointer
     * put: size "save the size"]]]]
     */

    /*
     * pg. 673 G&R
     */
    for (int objectPointer = 0; objectPointer <= ObjectTableSize - 2;
         objectPointer += 2) {
        if (OBJMEM_freeBitOf(objectPointer) == 0)       // the Object Table
            // entry is in use
        {
            // the object is in this segment
            if (OBJMEM_segmentBitsOf(objectPointer) == currentSegment) {
                // the object will be swept
                if (OBJMEM_locationBitsOf(objectPointer) >= lowWaterMark) {
                    size = OBJMEM_sizeBitsOf(objectPointer);    // rescue the
                    // size
                    OBJMEM_sizeBitsOf_put(objectPointer, objectPointer);        // reverse
                                                                                //
                    // pointer
                    OBJMEM_locationBitsOf_put(objectPointer, size);     // save
                                                                        //
                    // the
                    // size
                }
            }
        }
    }
}

// abandonFreeChunksInSegment:
int
OBJMEM_abandonFreeChunksInSegment(int segment)
{
    int             lowWaterMark;       // Location in the segment of the first
                                        //
    // free chunk
    int             objectPointer;
    int             nextPointer;

    /*
     * "source" lowWaterMark <- HeapSpaceStop. "first assume that no chunk is
     * free" HeaderSize to: BigSize do: "for each free-chunk list" [ :size |
     * objectPointer <- self headOfFreeChunkList: size inSegment: segment.
     * [objectPointer = NonPointer] whileFalse: [lowWaterMark <- lowWaterMark
     * min: (self locationBitsOf: objectPointer). nextPointer <- self
     * classBitsOf: objectPointer. "link to next free chunk" self classBitsOf:
     * objectPointer put: NonPointer. "distinguish for sweep" self
     * releasePointer: objectPointer. "add entry to free-pointer list"
     * objectPointer <- nextPointer]. self resetFreeChunkList: size inSegment:
     * segment]. ^lowWaterMark
     */

    // This computes the low water mark and sets the class field of free chunks
    //
    // to NonPointer
    // so that are identified as such and can be combined into one free block
    // after compaction
    lowWaterMark = HeapSpaceStop;       // first assume that no chunk is free
    for (int size = HeaderSize; size <= BigSize; size++) {
        objectPointer = OBJMEM_headOfFreeChunkList_inSegment(size, segment);
        while (objectPointer != NonPointer) {
            int             loc = OBJMEM_locationBitsOf(objectPointer);
            lowWaterMark = (lowWaterMark < loc) ? lowWaterMark : loc;
            nextPointer = OBJMEM_classBitsOf(objectPointer);    // link to next
                                                                //
            // free chunk
            OBJMEM_classBitsOf_put(objectPointer, NonPointer);  // distinguish
            // for sweep
            OBJMEM_releasePointer(objectPointer);       // Add entry to free
            // (object table)
            // pointer list
            objectPointer = nextPointer;
        }
        OBJMEM_resetFreeChunkList_inSegment(size, segment);
    }
    return lowWaterMark;
}

#ifdef GC_MARK_SWEEP

// reclaimInaccessibleObjects
void
OBJMEM_reclaimInaccessibleObjects()
{

    /*
     * "source" self zeroReferenceCounts. self markAccessibleObjects. self
     * rectifyCountsAndDeallocateGarbage
     */

    OBJMEM_zeroReferenceCounts();
    OBJMEM_markAccessibleObjects();
    OBJMEM_rectifyCountsAndDeallocateGarbage();
}

/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
static bool
pred1(int objectPointer)
{
    // the predicate tests for an unmarked object and marks it
    bool            unmarked = OBJMEM_countBitsOf(objectPointer) == 0;
    if (unmarked) {
        OBJMEM_countBitsOf_put(objectPointer, 1);
    }
    return unmarked;
}
static void
act1(int objectPointer)
{                               // the action restores the mark to count=1
    OBJMEM_countBitsOf_put(objectPointer, 1);
}

// markObjectsAccessibleFrom:
int
OBJMEM_markObjectsAccessibleFrom(int rootObjectPointer)
{
    /*
     * "source" ^self forAllObjectsAccessibleFrom: rootObjectPointer suchThat:
     * "the predicate tests for an unmarked object and marks it" [
     * :objectPointer | unmarked <- (self countBitsOf: objectPointer) = 0.
     * unmarked ifTrue: [self countBitsOf: objectPointer put: 1]. unmarked] do:
     * "the action restores the mark to count=1" [ :objectPointer | self
     * countBitsOf: objectPointer put: 1]
     */
    return OBJMEM_forAllObjectsAccessibleFrom_suchThat_do(rootObjectPointer,
                                                          pred1, act1);
}

/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
/*
 * =========================================================================
 */

// markAccessibleObjects
void
OBJMEM_markAccessibleObjects()
{
    /*
     * "source" "ERROR: rootObjectPointers not defined" rootObjectPointers do:
     * [ :rootObjectPointer | self markObjectsAccessibleFrom:
     * rootObjectPointer]
     */

    for (int i = 0; i <= LastSpecialOop; i += 2) {
        OBJMEM_addRoot(i);
    }

    if (haveGcNotification)
        gcNotification.prepareForCollection();

}
#endif

void
OBJMEM_outOfMemoryError()
{
    assert(0);
    abort();
}

// allocateChunk:
int
OBJMEM_allocateChunk(int size)
{
    int             objectPointer;

    /*
     * "source" "marking collector" objectPointer <- self
     * attemptToAllocateChunk: size. objectPointer isNil ifFalse:
     * [^objectPointer]. self reclaimInaccessibleObjects. "garbage collect and
     * try again" objectPointer <- self attemptToAllocateChunk: size.
     * objectPointer isNil ifFalse: [^objectPointer]. self outOfMemoryError
     * "give up"
     */

    objectPointer = OBJMEM_attemptToAllocateChunk(size);
#ifdef GC_MARK_SWEEP
    if (objectPointer == NilPointer) {
        OBJMEM_reclaimInaccessibleObjects();
        objectPointer = OBJMEM_attemptToAllocateChunk(size);
    }
#endif
    if (objectPointer != NilPointer) {
        if (freeWords >= size)
            freeWords -= size;

        return objectPointer;
    }
    OBJMEM_outOfMemoryError();  // give up
    return NilPointer;
}

#ifdef GC_MARK_SWEEP

// rectifyCountsAndDeallocateGarbage
void
OBJMEM_rectifyCountsAndDeallocateGarbage()
{
    int             count;

    /*
     * "source" "reset heads of free-chunk lists" FirstHeapSegment to:
     * LastHeapSegment do: "for every segment" [ :segment | HeaderSize to:
     * BigSize do: "for every free chunk list" [ :size | "reset the list head"
     * self resetFreeChunkList: size inSegment: segment]]. "rectify counts, and
     * deallocate garbage" 0 to: ObjectTableSize-2 by: 2 do: "for every object
     * table entry" [ :objectPointer | (self freeBitOf: objectPointer) = 0
     * ifTrue: "if it is not a free entry" [(count <- self countBitsOf:
     * objectPointer) = 0 ifTrue: "it is unmarked so deallocate it" [self
     * deallocate: objectPointer] ifFalse: "it is marked so rectify reference
     * counts" [count < 128 ifTrue: "subtract 1 to compensate for the mark"
     * [self countBitsOf: objectPointer put: count-1]. 1 to: (self
     * lastPointerOf: objectPointer)-1 do: [ :offset | "increment the reference
     * count of each pointer" self countUp: (self heapChunkOf: objectPointer
     * word: offset)]]]]. "be sure the root objects don't disappear" "ERROR:
     * rootObjectPointers not defined" rootObjectPointers do: [
     * :rootObjectPointer | self countUp: rootObjectPointer]. self countBitsOf:
     * NilPointer put: 128
     */
    // reset heads of free-chunk lists
    for (int segment = FirstHeapSegment; segment <= LastHeapSegment; segment++) {
        // for every free chunk list
        for (int size = HeaderSize; size <= BigSize; size++) {
            // reset the list head
            OBJMEM_resetFreeChunkList_inSegment(size, segment);
        }
    }

    // rectify counts, and deallocate garbage
    for (int objectPointer = 0; objectPointer <= ObjectTableSize - 2;
         objectPointer += 2) {
        if (OBJMEM_freeBitOf(objectPointer) == 0)       // if it is not a free
            // entry
        {
            count = OBJMEM_countBitsOf(objectPointer);
            if (count == 0) {
                // unmarked, so deallocate it
                freeWords += OBJMEM_spaceOccupiedBy(objectPointer);     // dbanay
                OBJMEM_deallocate(objectPointer);
            } else {
                // it is marked so rectify reference counts
                if (count < 128) {
                    // subtract 1 to compensate for the mark
                    OBJMEM_countBitsOf_put(objectPointer, count - 1);
                }

                int             limit = OBJMEM_lastPointerOf(objectPointer) - 1;
                // increment the reference count of each pointer
                // NB start at offset 1, which is the class
                for (int offset = 1; offset <= limit; offset++) {
                    OBJMEM_countUp(OBJMEM_heapChunkOf_word
                                   (objectPointer, offset));
                }
            }
        }
    }

    OBJMEM_countBitsOf_put(NilPointer, 128);

    freeOops = OBJMEM_auditFreeOops();

    if (haveGcNotification)
        gcNotification.collectionCompleted();

}

// zeroReferenceCounts
void
OBJMEM_zeroReferenceCounts()
{
    /*
     * "source" 0 to: ObjectTableSize-2 by: 2 do: [ :objectPointer | self
     * countBitsOf: objectPointer put: 0]
     */

    for (int objectPointer = 0; objectPointer <= ObjectTableSize - 2;
         objectPointer += 2) {
        OBJMEM_countBitsOf_put(objectPointer, 0);
    }
}
#endif

// lastPointerOf:
// This returns the size of object up to the last pointer in it
int
OBJMEM_lastPointerOf(int objectPointer)
{
    // MethodClass is the object table index of CompiledMethod.
    static const int MethodClass = ClassCompiledMethod;
    int             methodHeader;

    /*
     * "source" "ERROR - dbanay this is WRONG." (self pointerBitOf:
     * objectPointer) = 0 ifTrue: [^HeaderSize] "ERROR: MethodClass not
     * defined" ifFalse: [(self classBitsOf: objectPointer) = MethodClass
     * ifTrue: [methodHeader <- self heapChunkOf: objectPointer word:
     * HeaderSize. ^HeaderSize + 1 + ((methodHeader bitAnd: 126) bitShift: -1)]
     * ifFalse: [^self sizeBitsOf: objectPointer]]
     */

    /*
     * "source" "Fixed in July 1985 Ed of G&R" (self pointerBitOf:
     * objectPointer) = 0 ifTrue: [(self classBitsOf: objectPointer) =
     * MethodClass ifTrue: [methodHeader <- self heapChunkOf: objectPointer
     * word: HeaderSize. ^HeaderSize + 1 + ((methodHeader bitAnd: 126)
     * bitShift: -1)] ifFalse: [^HeaderSize]] ifFalse: [^self sizeBitsOf:
     * objectPointer]
     *
     */

    if (OBJMEM_pointerBitOf(objectPointer) == 0) {      // Not pointer object
        // CompiledMethods are special in that they are marked as having no
        // pointers but actually do
        if (OBJMEM_classBitsOf(objectPointer) == MethodClass) {
            methodHeader = OBJMEM_heapChunkOf_word(objectPointer, HeaderSize);
            // Header Size + Method Header Size + Literal Count
            return HeaderSize + 1 + ((methodHeader & 126) >> 1);
        } else
            return HeaderSize;  // Class field is last pointer in the object
    }

    return OBJMEM_sizeBitsOf(objectPointer);

}

// spaceOccupiedBy:
int
OBJMEM_spaceOccupiedBy(int objectPointer)
{
    int             size;

    /*
     * "source" size <- self sizeBitsOf: objectPointer. (size < HugeSize or:
     * [(self pointerBitOf: objectPointer) = 0]) ifTrue: [^size] ifFalse:
     * [^size + 1]
     */
    size = OBJMEM_sizeBitsOf(objectPointer);
    if (size < HugeSize || OBJMEM_pointerBitOf(objectPointer) == 0)
        return size;
    return size + 1;            // Account for extra word used for traversal
    // algorithm (G&R pg. 679)
}

// allocate:odd:pointer:extra:class:
int
OBJMEM_allocate_odd_pointer_extra_class(int size, int oddBit,
                                        int pointerBit, int extraWord,
                                        int classPointer)
{
    int             objectPointer;
    int             defaultValue;

    /*
     * "source" self countUp: classPointer. objectPointer <- self
     * allocateChunk: size + extraWord. self oddBitOf: objectPointer put:
     * oddBit. self pointerBitOf: objectPointer put: pointerBit. self
     * classBitsOf: objectPointer put: classPointer. default <- pointerBit=0
     * ifTrue: [0] ifFalse: [NilPointer]. HeaderSize to: size-1 do: [ :i | self
     * heapChunkOf: objectPointer word: i put: default]. self sizeBitsOf:
     * objectPointer put: size. ^objectPointer
     */
    OBJMEM_countUp(classPointer);
    objectPointer = OBJMEM_allocateChunk(size + extraWord);
    OBJMEM_oddBitOf_put(objectPointer, oddBit);
    OBJMEM_pointerBitOf_put(objectPointer, pointerBit);
    OBJMEM_classBitsOf_put(objectPointer, classPointer);
    defaultValue = (pointerBit == 0) ? 0 : NilPointer;
    for (int i = HeaderSize; i <= size - 1; i++) {
        OBJMEM_heapChunkOf_word_put(objectPointer, i, defaultValue);
    }
    OBJMEM_sizeBitsOf_put(objectPointer, size);
    freeOops--;                 // dbanay
    return objectPointer;
}

// headOfFreePointerList
int
OBJMEM_headOfFreePointerList()
{
    /*
     * "source" ^wordMemory segment: ObjectTableSegment word: FreePointerList
     */

    return RealMem_segment_word(ObjectTableSegment, FreePointerList);
}

// toFreeChunkList:add:
void
OBJMEM_toFreeChunkList_add(int size, int objectPointer)
{
    int             segment;

    /*
     * "source" segment <- self segmentBitsOf: objectPointer. self classBitsOf:
     * objectPointer put: (self headOfFreeChunkList: size inSegment: segment).
     * self headOfFreeChunkList: size inSegment: segment put: objectPointer
     */

    segment = OBJMEM_segmentBitsOf(objectPointer);
    OBJMEM_classBitsOf_put(objectPointer,
                           OBJMEM_headOfFreeChunkList_inSegment(size, segment));
    OBJMEM_headOfFreeChunkList_inSegment_put(size, segment, objectPointer);

}

// headOfFreePointerListPut:
int
OBJMEM_headOfFreePointerListPut(int objectPointer)
{
    /*
     * "source" ^wordMemory segment: ObjectTableSegment word: FreePointerList
     * put: objectPointer
     */

    return RealMem_segment_word_put(ObjectTableSegment, FreePointerList,
                                    objectPointer);
}

// headOfFreeChunkList:inSegment:put:
int
OBJMEM_headOfFreeChunkList_inSegment_put(int size, int segment,
                                         int objectPointer)
{
    /*
     * "source" ^wordMemory segment: segment word: FirstFreeChunkList + size
     * put: objectPointer
     */

    RUNTIME_CHECK(size >= HeaderSize && size <= BigSize);

    return RealMem_segment_word_put(segment, FirstFreeChunkList + size,
                                    objectPointer);
}

// removeFromFreePointerList
int
OBJMEM_removeFromFreePointerList()
{
    int             objectPointer;

    /*
     * "source" objectPointer <- self headOfFreePointerList. objectPointer =
     * NonPointer ifTrue: [^nil]. self headOfFreePointerListPut: (self
     * locationBitsOf: objectPointer). ^objectPointer
     */

    objectPointer = OBJMEM_headOfFreePointerList();
    if (objectPointer == NonPointer)
        return NilPointer;
    OBJMEM_headOfFreePointerListPut(OBJMEM_locationBitsOf(objectPointer));
    return objectPointer;
}

// toFreePointerListAdd:
void
OBJMEM_toFreePointerListAdd(int objectPointer)
{
    /*
     * "source" self locationBitsOf: objectPointer put: (self
     * headOfFreePointerList). self headOfFreePointerListPut: objectPointer
     */

    OBJMEM_locationBitsOf_put(objectPointer, OBJMEM_headOfFreePointerList());
    OBJMEM_headOfFreePointerListPut(objectPointer);

}

// removeFromFreeChunkList:
int
OBJMEM_removeFromFreeChunkList(int size)
{
    int             objectPointer;
    int             secondChunk;

    /*
     * "source" objectPointer <- self headOfFreeChunkList: size inSegment:
     * currentSegment. objectPointer = NonPointer ifTrue: [^nil]. secondChunk
     * <- self classBitsOf: objectPointer. self headOfFreeChunkList: size
     * inSegment: currentSegment put: secondChunk. ^objectPointer
     */

    RUNTIME_CHECK(currentSegment >= FirstHeapSegment &&
                  currentSegment <= LastHeapSegment);

    objectPointer = OBJMEM_headOfFreeChunkList_inSegment(size, currentSegment);
    if (objectPointer == NonPointer)
        return NilPointer;
    secondChunk = OBJMEM_classBitsOf(objectPointer);
    OBJMEM_headOfFreeChunkList_inSegment_put(size, currentSegment, secondChunk);
    return objectPointer;

}

// resetFreeChunkList:inSegment:
void
OBJMEM_resetFreeChunkList_inSegment(int size, int segment)
{
    /*
     * "source" self headOfFreeChunkList: size inSegment: segment put:
     * NonPointer
     */
    RUNTIME_CHECK(size >= 2 && size <= BigSize);
    OBJMEM_headOfFreeChunkList_inSegment_put(size, segment, NonPointer);
}

// headOfFreeChunkList:inSegment:
int
OBJMEM_headOfFreeChunkList_inSegment(int size, int segment)
{
    /*
     * "source" ^wordMemory segment: segment word: FirstFreeChunkList + size
     */
    RUNTIME_CHECK(size >= 2 && size <= BigSize);
    return RealMem_segment_word(segment, FirstFreeChunkList + size);
}

/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
// this is a pointer, so decrement its reference count
static bool
pred0(int objectPointer)
{                               // predicate
    int             count = OBJMEM_countBitsOf(objectPointer) - 1;
    RUNTIME_CHECK(count >= 0);
    if (count < 127)
        OBJMEM_countBitsOf_put(objectPointer, count);
    return count == 0;
}
static void
act0(int objectPointer)
{                               // action
    // std::cout << "reference count zero. freeing " << objectPointer << " ("
    // << classNameOfObject(fetchClassOf(objectPointer)) << ") free oops = " <<
    //
    // freeOops << "\n";
    OBJMEM_countBitsOf_put(objectPointer, 0);
    freeWords += OBJMEM_spaceOccupiedBy(objectPointer); // dbanay
    freeOops++;
    OBJMEM_deallocate(objectPointer);
}

// countDown:
int
OBJMEM_countDown(int rootObjectPointer)
{

    /*
     * "source" (self isIntegerObject: rootObjectPointer) ifTrue:
     * [^rootObjectPointer] ifFalse: "this is a pointer, so decrement its
     * reference count" [^self forAllObjectsAccessibleFrom: rootObjectPointer
     * suchThat: "the predicate decrements the count and tests for zero" [
     * :objectPointer | count <- (self countBitsOf: objectPointer) - 1. count <
     * 127 ifTrue: [self countBitsOf: objectPointer put: count]. count = 0] do:
     * "the action zeroes the count and deallocates the object" [
     * :objectPointer | self countBitsOf: objectPointer put: 0. self
     * deallocate: objectPointer]]
     */

    if (OBJMEM_isIntegerObject(rootObjectPointer))
        return rootObjectPointer;

    RUNTIME_CHECK(OBJMEM_countBitsOf(rootObjectPointer) > 0);

    // this is a pointer, so decrement its reference count
    return OBJMEM_forAllObjectsAccessibleFrom_suchThat_do(rootObjectPointer,
                                                          pred0, act0);

}

/*
 * =========================================================================
 */
/*
 * =========================================================================
 */
/*
 * =========================================================================
 */

// countUp:
int
OBJMEM_countUp(int objectPointer)
{
    int             count;

    /*
     * "source" (self isIntegerObject: objectPointer) ifFalse: [count <- (self
     * countBitsOf: objectPointer) + 1. count < 129 ifTrue: [self countBitsOf:
     * objectPointer put: count]]. ^objectPointer
     */
    if (!OBJMEM_isIntegerObject(objectPointer)) {
        count = OBJMEM_countBitsOf(objectPointer) + 1;
        if (count < 129)        // Count sticks at 128
            OBJMEM_countBitsOf_put(objectPointer, count);
    }

    return objectPointer;
}

// deallocate:
void
OBJMEM_deallocate(int objectPointer)
{
    int             space;

    /*
     * "source" space <- self spaceOccupiedBy: objectPointer. self sizeBitsOf:
     * objectPointer put: space. self toFreeChunkList: (space min: BigSize)
     * add: objectPointer
     */

    space = OBJMEM_spaceOccupiedBy(objectPointer);
    OBJMEM_sizeBitsOf_put(objectPointer, space);
    OBJMEM_toFreeChunkList_add(((space < (int) BigSize) ? space : BigSize),
                               objectPointer);
}

#ifdef RECURSIVE_MARKING
// recursive version -- stack hungry
// forAllOtherObjectsAccessibleFrom:suchThat:do:
int
OBJMEM_forAllOtherObjectsAccessibleFrom_suchThat_do(int objectPointer,
                                                    bool (*predicate)(int),
                                                    void (*action)(int)
    )
{
    int             next;

    /*
     * "source" 1 to: (self lastPointerOf: objectPointer) - 1 do: [ :offset |
     * next <- self heapChunkOf: objectPointer word: offset. ((self
     * isIntegerObject: next)==false and: [predicate value: next]) ifTrue:
     * "it's a non-immediate object and it should be processed" [self
     * forAllOtherObjectsAccessibleFrom: next suchThat: predicate do: action]].
     * "all pointers have been followed; now perform the action" action value:
     * objectPointer. ^objectPointer
     */

    int             limit = OBJMEM_lastPointerOf(objectPointer) - 1;
    // N.B. start at offset 1, which is class field
    for (int offset = 1; offset <= limit; offset++) {
        next = OBJMEM_heapChunkOf_word(objectPointer, offset);
        if (!OBJMEM_isIntegerObject(next) && predicate(next)) {
            // it's a non-immediate object and it should be processed
            OBJMEM_forAllOtherObjectsAccessibleFrom_suchThat_do(next,
                                                                predicate,
                                                                action);
        }
    }

    // all pointers have been followed; now perform the action
    action(objectPointer);
    return objectPointer;
}
#else
// forAllOtherObjectsAccessibleFrom:suchThat:do:
int
OBJMEM_forAllOtherObjectsAccessibleFrom_suchThat_do(int objectPointer,
                                                    bool (*predicate)(int),
                                                    void (*action)(int))
{
    int             prior;
    int             current;
    int             offset;
    int             size;
    int             next;

    /*
     * "source" "compute prior, current, offset, and size to begin processing
     * objectPointer" prior <- NonPointer. current <- objectPointer. offset <-
     * size <- self lastPointerOf: objectPointer. [true] whileTrue: "for all
     * pointers in all objects traversed" [(offset <- offset - 1) > 0
     * "decrement the field index" ifTrue: "the class hasn't been passed yet"
     * [next <- self heapChunkOf: current word: offset. "one of the pointers"
     * ((self isIntegerObject: next)==false and: [predicate value: next])
     * ifTrue: "it's a non-immediate object and it should be processed"
     * ["reverse the pointer chain" self heapChunkOf: current word: offset put:
     * prior. "save the offset either in the count field or in the extra word"
     * size < HugeSize ifTrue: [self countBitsOf: current put: offset] ifFalse:
     * [self heapChunkOf: current word: size+1 put: offset]. "ERROR dbanay size
     * not size + 1" "compute prior, current, offset, and size to begin
     * processing next" prior <- current. current <- next. offset <- size <-
     * self lastPointerOf: current]] ifFalse: ["all pointers have been
     * followed; now perform the action" action value: objectPointer. "ERROR
     * dbanay - should be current" "did we get here from another object?" prior
     * = NonPointer ifTrue: "this was the root object, so we are done"
     * [^objectPointer]. "restore next, current and size to resume processing
     * prior" next <- current. current <- prior. size <- self lastPointerOf:
     * current. "restore offset either from the count field or from the extra
     * word" size < HugeSize ifTrue: [offset <- self countBitsOf: current]
     * ifFalse: [offset <- self heapChunkOf: current word: size+1]. "ERROR
     * dbanay size not size + 1" "restore prior from the reversed pointer
     * chain" prior <- self heapChunkOf: current word: offset. "restore
     * (un-reverse) the pointer chain" self heapChunkOf: current word: offset
     * put: next]]
     */

    prior = NonPointer;
    current = objectPointer;
    offset = size = OBJMEM_lastPointerOf(objectPointer);
    for (;;) {
        // for all pointers in all objects traversed
        offset--;               // decrement the field index
        if (offset > 0)         // the class hasn't been passed yet
        {
            next = OBJMEM_heapChunkOf_word(current, offset);    // one of the
            // pointers
            if (!OBJMEM_isIntegerObject(next) && predicate(next)) {
                // it's a non-immediate object and it should be processed
                // reverse pointer chain
                OBJMEM_heapChunkOf_word_put(current, offset, prior);
                // save the offset either in the count field or in the extra
                // word
                if (size < HugeSize)
                    OBJMEM_countBitsOf_put(current, offset);
                else
                    OBJMEM_heapChunkOf_word_put(current, size, offset);
                // compute prior, current, offset, and size to begin processing
                //
                // next
                prior = current;
                current = next;
                offset = size = OBJMEM_lastPointerOf(current);
            }
        } else {
            // all pointers have been followed; now perform the action
            action(current);
            // did we get here from another object?
            if (prior == NonPointer)    // this was the root object, so we are
                // done
                return objectPointer;
            // restore next, current and size to resume processing prior
            next = current;
            current = prior;
            size = OBJMEM_lastPointerOf(current);
            // restore offset either from the count field or from the extra
            // word
            if (size < HugeSize)
                offset = OBJMEM_countBitsOf(current);
            else
                offset = OBJMEM_heapChunkOf_word(current, size);
            // restore prior from the reversed pointer chain
            prior = OBJMEM_heapChunkOf_word(current, offset);
            // restore (un-reverse) the pointer chain
            OBJMEM_heapChunkOf_word_put(current, offset, next);
        }
    }
}
#endif

// forAllObjectsAccessibleFrom:suchThat:do:
int
OBJMEM_forAllObjectsAccessibleFrom_suchThat_do(int objectPointer,
                                               bool (*predicate)(int),
                                               void (*action)(int))
{
    /*
     * "source" (predicate value: objectPointer) ifTrue: [^self
     * forAllOtherObjectsAccessibleFrom: objectPointer suchThat: predicate do:
     * action]
     */

    if (predicate(objectPointer)) {
        return
            OBJMEM_forAllOtherObjectsAccessibleFrom_suchThat_do(objectPointer,
                                                                predicate,
                                                                action);
    }

    return NilPointer;

}

// storePointer:ofObject:withValue:
int
OBJMEM_storePointer_ofObject_withValue(int fieldIndex, int objectPointer,
                                       int valuePointer)
{
    int             chunkIndex;

    /*
     * "source" chunkIndex <- HeaderSize + fieldIndex. self countUp:
     * valuePointer. self countDown: (self heapChunkOf: objectPointer word:
     * chunkIndex). ^self heapChunkOf: objectPointer word: chunkIndex put:
     * valuePointer
     */

    RUNTIME_CHECK(fieldIndex >= 0 &&
                  fieldIndex < OBJMEM_fetchWordLengthOf(objectPointer));
    RUNTIME_CHECK(valuePointer > 0);

    chunkIndex = HeaderSize + fieldIndex;

#ifdef GC_REF_COUNT
    OBJMEM_countUp(valuePointer);
    OBJMEM_countDown(OBJMEM_heapChunkOf_word(objectPointer, chunkIndex));
#endif
    return OBJMEM_heapChunkOf_word_put(objectPointer, chunkIndex, valuePointer);
}

// storeWord:ofObject:withValue:
int
OBJMEM_storeWord_ofObject_withValue(int wordIndex, int objectPointer,
                                    int valueWord)
{
    /*
     * "source" ^self heapChunkOf: objectPointer word: HeaderSize + wordIndex
     * put: valueWord
     */
    RUNTIME_CHECK(wordIndex >= 0 &&
                  wordIndex < OBJMEM_fetchWordLengthOf(objectPointer));
    return OBJMEM_heapChunkOf_word_put(objectPointer, HeaderSize + wordIndex,
                                       valueWord);
}

// initialInstanceOf:
int
OBJMEM_initialInstanceOf(int classPointer)
{
    // Mario checks for the count bit not being zero. This is necessary
    // because an OT entry with a clear free bit but a ZERO count marks
    // an allocated chunk of memory available for use
    /*
     * "source" 0 to: ObjectTableSize-2 by: 2 do: [ :pointer | "ERROR in next
     * line, second part of test omitted" ((self freeBitOf: pointer) = 0 and:
     * [(self countBitOf: pointer) ~= 0]) ifTrue: [(self fetchClassOf: pointer)
     * = classPointer ifTrue: [^pointer]]]. ^NilPointer
     */

    for (int pointer = 0; pointer <= ObjectTableSize - 2; pointer += 2) {
        // Only consider non-free entries that are not free chunks
        if (OBJMEM_freeBitOf(pointer) == 0 && OBJMEM_countBitsOf(pointer) != 0) {
            if (OBJMEM_fetchClassOf(pointer) == classPointer)
                return pointer;
        }
    }
    return NilPointer;
}

// swapPointersOf:and:
void
OBJMEM_swapPointersOf_and(int firstPointer, int secondPointer)
{

    int             firstSegment;
    int             firstLocation;
    int             firstPointerBit;    /* dbanay - ERROR was named
                                         * firstPointer */
    int             firstOdd;

    /*
     * "source" "ERROR in next line, firstPointer redefined" "dbanay - should
     * have been called firstPointerBit (FYI - NOT FIXED IN July 1985 ed)"
     * firstSegment <- self segmentBitsOf: firstPointer. firstLocation <- self
     * locationBitsOf: firstPointer. "dbanay ERROR - next should be:
     * firstPointerBit <- self pointerBitOf: firstPointer. firstPointer <- self
     * pointerBitOf: firstPointer. firstOdd <- self oddBitOf: firstPointer.
     * self segmentBitsOf: firstPointer put: (self segmentBitsOf:
     * secondPointer). self locationBitsOf: firstPointer put: (self
     * locationBitsOf: secondPointer). self pointerBitOf: firstPointer put:
     * (self pointerBitOf: secondPointer). self oddBitOf: firstPointer put:
     * (self oddBitOf: secondPointer). self segmentBitsOf: secondPointer put:
     * firstSegment. self locationBitsOf: secondPointer put: firstLocation.
     * "ERROR dbanay - should be firstPointerBit" self pointerBitOf:
     * secondPointer put: firstPointer. self oddBitOf: secondPointer put:
     * firstOdd
     */

    firstSegment = OBJMEM_segmentBitsOf(firstPointer);
    firstLocation = OBJMEM_locationBitsOf(firstPointer);
    firstPointerBit = OBJMEM_pointerBitOf(firstPointer);
    firstOdd = OBJMEM_oddBitOf(firstPointer);

    OBJMEM_segmentBitsOf_put(firstPointer, OBJMEM_segmentBitsOf(secondPointer));
    OBJMEM_locationBitsOf_put(firstPointer,
                              OBJMEM_locationBitsOf(secondPointer));
    OBJMEM_pointerBitOf_put(firstPointer, OBJMEM_pointerBitOf(secondPointer));
    OBJMEM_oddBitOf_put(firstPointer, OBJMEM_oddBitOf(secondPointer));

    OBJMEM_segmentBitsOf_put(secondPointer, firstSegment);
    OBJMEM_locationBitsOf_put(secondPointer, firstLocation);
    OBJMEM_pointerBitOf_put(secondPointer, firstPointerBit);
    OBJMEM_oddBitOf_put(secondPointer, firstOdd);
}

// instantiateClass:withWords:
int
OBJMEM_instantiateClass_withWords(int classPointer, int length)
{
    int             size;

    /*
     * "source" size <- HeaderSize + length. ^self allocate: size odd: 0
     * pointer: 0 extra: 0 class: classPointer
     */
    size = HeaderSize + length;
    return OBJMEM_allocate_odd_pointer_extra_class(size, 0, 0, 0, classPointer);
}

// instantiateClass:withBytes:
int
OBJMEM_instantiateClass_withBytes(int classPointer, int length)
{
    int             size;

    /*
     * "source" size <- HeaderSize + ((length + 1)/2). ^self allocate: size
     * odd: length\\2 pointer: 0 extra: 0 class: classPointer
     */
    size = HeaderSize + ((length + 1) / 2);
    return OBJMEM_allocate_odd_pointer_extra_class(size, length % 2, 0, 0,
                                                   classPointer);
}

// hasObject:
bool
OBJMEM_hasObject(int objectPointer)
{
    /*
     * "source" "is objectPointer pointing to a valid object ?" ^(self
     * freeBitOf: objectPointer) = 0 "not sure that the first clause is
     * necessary" and: [(self countBitsOf: objectPointer) ~= 0]
     */
    OBJMEM_cantBeIntegerObject(objectPointer);  // dbanay
    return OBJMEM_freeBitOf(objectPointer) == 0
        && OBJMEM_countBitsOf(objectPointer) != 0;
}

int
OBJMEM_auditFreeOops()
{
    int             count = 0;
    for (int objectPointer = 2; objectPointer < ObjectTableSize;
         objectPointer += 2) {
        if (!OBJMEM_hasObject(objectPointer))
            count++;
    }

    return count;
}

// instantiateClass:withPointers:
int
OBJMEM_instantiateClass_withPointers(int classPointer, int length)
{
    int             size;
    int             extra;

    /*
     * "source" size <- HeaderSize + length. extra <- size < HugeSize ifTrue:
     * [0] ifFalse: [1]. ^self allocate: size odd: 0 pointer: 1 extra: extra
     * class: classPointer
     */

    size = HeaderSize + length;
    extra = size < HugeSize ? 0 : 1;
    return OBJMEM_allocate_odd_pointer_extra_class(size, 0, 1, extra,
                                                   classPointer);
}

// instanceAfter:
int
OBJMEM_instanceAfter(int objectPointer)
{
    int             classPointer;

    /*
     * "source" "July 1985 ed also got this wrong" "ERROR: next line omitted by
     * G&R" classPointer <- self fetchClassOf: objectPointer. "dbanay - ERROR -
     * was 'objectPointer to: ObjectTableSize-2 by: 2 do:' should start at
     * objectPointer+2'" "dbanay - ERROR - should also check count bits to skip
     * free Chunk entries" objectPointer to: ObjectTableSize-2 by: 2 do: [
     * :pointer | (self freeBitOf: pointer) = 0 ifTrue: [(self fetchClassOf:
     * pointer) = classPointer ifTrue: [^pointer]]]. ^NilPointer
     */

    classPointer = OBJMEM_fetchClassOf(objectPointer);

    for (int pointer = objectPointer + 2; pointer <= ObjectTableSize - 2;
         pointer += 2) {
        if (OBJMEM_hasObject(pointer)) {
            if (OBJMEM_fetchClassOf(pointer) == classPointer)
                return pointer;
        }
    }
    return NilPointer;

}

// cantBeIntegerObject:
void
OBJMEM_cantBeIntegerObject(int objectPointer)
{
    /*
     * "source" (self isIntegerObject: objectPointer) "ERROR: Sensor and
     * notify: not defined" ifTrue: [Sensor notify: 'A small integer has no
     * object table entry']
     */

    assert(!OBJMEM_isIntegerObject(objectPointer));
    if (OBJMEM_isIntegerObject(objectPointer)) {
        fprintf(stderr, "A small integer has no object table entry\n");
        abort();
    }
}

// obtainPointer:location:
int
OBJMEM_obtainPointer_location(int size, int location)
{
    int             objectPointer;

    /*
     * "source" objectPointer <- self removeFromFreePointerList. objectPointer
     * isNil ifTrue: [^nil]. self ot: objectPointer put: 0. self segmentBitsOf:
     * objectPointer put: currentSegment. self locationBitsOf: objectPointer
     * put: location. self sizeBitsOf: objectPointer put: size. ^objectPointer
     */
    RUNTIME_CHECK(currentSegment >= FirstHeapSegment &&
                  currentSegment <= LastHeapSegment);
    objectPointer = OBJMEM_removeFromFreePointerList();
    if (objectPointer == NilPointer)
        return NilPointer;
    OBJMEM_ot_put(objectPointer, 0);
    OBJMEM_segmentBitsOf_put(objectPointer, currentSegment);
    OBJMEM_locationBitsOf_put(objectPointer, location);
    OBJMEM_sizeBitsOf_put(objectPointer, size);
    return objectPointer;
}

// attemptToAllocateChunk:
int
OBJMEM_attemptToAllocateChunk(int size)
{
    int             objectPointer;

    /*
     * "source" objectPointer <- self attemptToAllocateChunkInCurrentSegment:
     * size. objectPointer isNil ifFalse: [^objectPointer]. 1 to:
     * HeapSegmentCount do: [ :i | currentSegment <- currentSegment + 1.
     * currentSegment > LastHeapSegment ifTrue: [currentSegment <-
     * FirstHeapSegment]. self compactCurrentSegment. objectPointer <- self
     * attemptToAllocateChunkInCurrentSegment: size. objectPointer isNil
     * ifFalse: [^objectPointer]]. ^nil
     */

    RUNTIME_CHECK(currentSegment >= FirstHeapSegment &&
                  currentSegment <= LastHeapSegment);
    objectPointer = OBJMEM_attemptToAllocateChunkInCurrentSegment(size);
    if (objectPointer != NilPointer)
        return objectPointer;
    for (int i = 1; i <= HeapSegmentCount; i++) {
        currentSegment++;
        if (currentSegment > LastHeapSegment)
            currentSegment = FirstHeapSegment;
        OBJMEM_compactCurrentSegment();
        objectPointer = OBJMEM_attemptToAllocateChunkInCurrentSegment(size);
        if (objectPointer != NilPointer)
            return objectPointer;

    }
    return NilPointer;
}

// attemptToAllocateChunkInCurrentSegment:
int
OBJMEM_attemptToAllocateChunkInCurrentSegment(int size)
{
    int             objectPointer = NilPointer;
    int             predecessor;
    int             next;
    int             availableSize;
    int             excessSize;
    int             newPointer;

    /*
     * "source" ERROR dbanay - LastFreeChunkList is a LOCATION and not a size.
     * Should use BigSize size < BigSize ifTrue: [objectPointer <- self
     * removeFromFreeChunkList: size]. objectPointer notNil ifTrue:
     * [^objectPointer]. "small chunk of exact size handy so use it"
     * predecessor <- NonPointer. "remember predecessor of chunk under
     * consideration" objectPointer <- self headOfFreeChunkList:
     * LastFreeChunkList inSegment: currentSegment. "the search loop stops when
     * the end of the linked list is encountered" [objectPointer = NonPointer]
     * whileFalse: [availableSize <- self sizeBitsOf: objectPointer.
     * availableSize = size ifTrue: "exact fit - remove from free chunk list
     * and return" [next <- self classBitsOf: objectPointer. "the link to the
     * next chunk" predecessor = NonPointer ifTrue: "it was the head of the
     * list; make the next item the head" [self headOfFreeChunkList:
     * LastFreeChunkList inSegment: currentSegment put: next] ifFalse: "it was
     * between two chunks; link them together" [self classBitsOf: predecessor
     * put: next]. ^objectPointer]. "this chunk was either too big or too
     * small; inspect the amount of variance" excessSize <- availableSize -
     * size. excessSize >= HeaderSize ifTrue: "can be broken into two usable
     * parts: return the second part" ["obtain an object table entry for the
     * second part" newPointer <- self obtainPointer: size location: (self
     * locationBitsOf: objectPointer) + excessSize. newPointer isNil ifTrue:
     * [^nil]. "correct the size of the first part (which remains on the free
     * list)" self sizeBitsOf: objectPointer put: excessSize. ^newPointer]
     * ifFalse: "not big enough to use; try the next chunk on the list"
     * [predecessor <- objectPointer. objectPointer <- self classBitsOf:
     * objectPointer]]. ^nil "the end of the linked list was reached and no fit
     * was found"
     */
    RUNTIME_CHECK(currentSegment >= FirstHeapSegment &&
                  currentSegment <= LastHeapSegment);
    if (size < BigSize)
        objectPointer = OBJMEM_removeFromFreeChunkList(size);

    if (objectPointer != NilPointer) {
        return objectPointer;   // small chunk of exact size handy so use it
    }
    predecessor = NonPointer;   // remember predecessor of chunk under
    // consideration
    objectPointer =
        OBJMEM_headOfFreeChunkList_inSegment(BigSize, currentSegment);

    // the search loop stops when the end of the linked list is encountered
    while (objectPointer != NonPointer) {
        availableSize = OBJMEM_sizeBitsOf(objectPointer);
        if (availableSize == size) {
            // exact fit - remove from free chunk list and return
            next = OBJMEM_classBitsOf(objectPointer);   // the link to the next
                                                        //
            // chunk
            if (predecessor == NonPointer) {
                // it was the head of the list; make the next item the head
                OBJMEM_headOfFreeChunkList_inSegment_put(BigSize,
                                                         currentSegment, next);
            } else {
                // it was between two chunks; link them together
                OBJMEM_classBitsOf_put(predecessor, next);
            }
            return objectPointer;
        }
        // this chunk was either too big or too small; inspect the amount of
        // variance
        excessSize = availableSize - size;
        if (excessSize >= HeaderSize) {
            // can be broken into two usable parts: return the second part
            // obtain an object table entry for the second part
            newPointer = OBJMEM_obtainPointer_location(size,
                                                       OBJMEM_locationBitsOf
                                                       (objectPointer) +
                                                       excessSize);
            if (newPointer == NilPointer)
                return NilPointer;
            // correct the size of the first part (which remains on the free
            // list)
            OBJMEM_sizeBitsOf_put(objectPointer, excessSize);
            return newPointer;
        } else {
            // not big enough to use; try the next chunk on the list
            predecessor = objectPointer;
            objectPointer = OBJMEM_classBitsOf(objectPointer);
        }
    }

    return NilPointer;          // the end of the linked list was reached and
    // no fit was found
}

/*
 * ========================================================================
 */

// --- BCIInterface ---
int
OBJMEM_oopsLeft()
{
    return freeOops;
}

uint32_t
OBJMEM_coreLeft()
{
    return freeWords;
}

// increaseReferencesTo:
void
OBJMEM_increaseReferencesTo(int objectPointer)
{
    /*
     * "source" self countUp: objectPointer
     */
#ifdef GC_REF_COUNT
    OBJMEM_countUp(objectPointer);
#endif
}

// decreaseReferencesTo:
void
OBJMEM_decreaseReferencesTo(int objectPointer)
{
    /*
     * "source" self countDown: objectPointer
     */
#ifdef GC_REF_COUNT
    OBJMEM_countDown(objectPointer);
#endif
}

// isIntegerValue:
bool
OBJMEM_isIntegerValue(int valueWord)
{
    /*
     * "source" "ERROR: G&R really cock this up" "dbanay - still broken in July
     * 1985 ed!" ^valueWord >= -16384 and: [valueWord <= 16383]
     */

    return valueWord >= -16384 && valueWord <= 16383;
}

// fetchWord:ofObject:
int
OBJMEM_fetchWord_ofObject(int wordIndex, int objectPointer)
{
    /*
     * "source" ^self heapChunkOf: objectPointer word: HeaderSize + wordIndex
     */

    RUNTIME_CHECK(wordIndex >= 0 &&
                  wordIndex < OBJMEM_fetchWordLengthOf(objectPointer));
    return OBJMEM_heapChunkOf_word(objectPointer, HeaderSize + wordIndex);
}

// integerValueOf:
int
OBJMEM_integerValueOf(int objectPointer)
{
    /*
     * "source" ^objectPointer/2
     */

    return (int16_t) (objectPointer & 0xfffe) / 2;
    // Right shifting a negative number is undefined according to the standard.
    // return ((int16_t) objectPointer) >> 1;
}

// fetchWordLengthOf:
int
OBJMEM_fetchWordLengthOf(int objectPointer)
{
    /*
     * "source" ^(self sizeBitsOf: objectPointer) - HeaderSize
     */

    return OBJMEM_sizeBitsOf(objectPointer) - HeaderSize;
}

// isIntegerObject:
bool
OBJMEM_isIntegerObject(int objectPointer)
{
    /*
     * "source" ^(objectPointer bitAnd: 1) = 1
     */

    return (objectPointer & 1) == 1;
}

// fetchByte:ofObject:
int
OBJMEM_fetchByte_ofObject(int byteIndex, int objectPointer)
{
    /*
     * "source" ^self heapChunkOf: objectPointer byte: (HeaderSize*2 +
     * byteIndex)
     */
    return OBJMEM_heapChunkOf_byte(objectPointer, (HeaderSize * 2 + byteIndex));
}

// fetchPointer:ofObject:
int
OBJMEM_fetchPointer_ofObject(int fieldIndex, int objectPointer)
{
    /*
     * "source" ^self heapChunkOf: objectPointer word: HeaderSize + fieldIndex
     */
    RUNTIME_CHECK(fieldIndex >= 0 &&
                  fieldIndex < OBJMEM_fetchWordLengthOf(objectPointer)
        );
    return OBJMEM_heapChunkOf_word(objectPointer, HeaderSize + fieldIndex);
}

// fetchClassOf:
int
OBJMEM_fetchClassOf(int objectPointer)
{
    /*
     * Note that fetchClassOf:objectPointer returns IntegerClass (the object
     * table index of SmallInteger) if its argument is an immediate integer.
     * G&R pg 686
     */
    /*
     * "source" (self isIntegerObject: objectPointer) ifTrue: [^IntegerClass]
     * "ERROR IntegerClass not defined" ifFalse: [^self classBitsOf:
     * objectPointer]
     */

    if (OBJMEM_isIntegerObject(objectPointer))
        return ClassSmallInteger;

    return OBJMEM_classBitsOf(objectPointer);

}

// integerObjectOf:
int
OBJMEM_integerObjectOf(int value)
{
    /*
     * "source" ^(value bitShift: 1) + 1
     */
    return (uint16_t) ((value << 1) | 1);
}

// fetchByteLengthOf:
int
OBJMEM_fetchByteLengthOf(int objectPointer)
{
    /*
     * "source" "ERROR in selector of next line" ^(self fetchWordLengthOf:
     * objectPointer)*2 - (self oddBitOf: objectPointer)
     */
    return OBJMEM_fetchWordLengthOf(objectPointer) * 2
        - OBJMEM_oddBitOf(objectPointer);
}

// storeByte:ofObject:withValue:
int
OBJMEM_storeByte_ofObject_withValue(int byteIndex, int objectPointer,
                                    int valueByte)
{
    /*
     * "source" ^self heapChunkOf: objectPointer byte: (HeaderSize*2 +
     * byteIndex) put: valueByte
     */

    return OBJMEM_heapChunkOf_byte_put(objectPointer,
                                       HeaderSize * 2 + byteIndex, valueByte);
}

// --- ObjectPointers ---

#ifdef GC_MARK_SWEEP
void
OBJMEM_addRoot(int rootObjectPointer)   // dbanay
{
    OBJMEM_markObjectsAccessibleFrom(rootObjectPointer);
}
#endif

// segmentBitsOf:
int
OBJMEM_segmentBitsOf(int objectPointer)
{
    /*
     * "source" ^self ot: objectPointer bits: 12 to: 15
     */

    return OBJMEM_ot_bits_to(objectPointer, 12, 15);
}

// heapChunkOf:byte:put:
int
OBJMEM_heapChunkOf_byte_put(int objectPointer, int offset, int value)
{
    /*
     * "source" ^wordMemory segment: (self segmentBitsOf: objectPointer) word:
     * ((self locationBitsOf: objectPointer) + (offset//2)) byte: (offset\\2)
     * put: value
     */

    return RealMem_segment_word_byte_put(OBJMEM_segmentBitsOf(objectPointer),
                                         OBJMEM_locationBitsOf(objectPointer) +
                                         (offset / 2), offset % 2, value);
}

// pointerBitOf:put:
int
OBJMEM_pointerBitOf_put(int objectPointer, int value)
{
    /*
     * "source" ^self ot: objectPointer bits: 9 to: 9 put: value
     */

    return OBJMEM_ot_bits_to_put(objectPointer, 9, 9, value);
}

// heapChunkOf:word:
int
OBJMEM_heapChunkOf_word(int objectPointer, int offset)
{
    /*
     * "source" ^wordMemory segment: (self segmentBitsOf: objectPointer) word:
     * ((self locationBitsOf: objectPointer) + offset)
     */
    return RealMem_segment_word(OBJMEM_segmentBitsOf(objectPointer),
                                OBJMEM_locationBitsOf(objectPointer) + offset);
}

// segmentBitsOf:put:
int
OBJMEM_segmentBitsOf_put(int objectPointer, int value)
{
    /*
     * "source" ^self ot: objectPointer bits: 12 to: 15 put: value
     */

    return OBJMEM_ot_bits_to_put(objectPointer, 12, 15, value);
}

// heapChunkOf:word:put:
int
OBJMEM_heapChunkOf_word_put(int objectPointer, int offset, int value)
{
    /*
     * "source" ^wordMemory segment: (self segmentBitsOf: objectPointer) word:
     * ((self locationBitsOf: objectPointer) + offset) put: value
     */
    return RealMem_segment_word_put(OBJMEM_segmentBitsOf(objectPointer),
                                    OBJMEM_locationBitsOf(objectPointer) +
                                    offset, value);
}

// oddBitOf:
int
OBJMEM_oddBitOf(int objectPointer)
{
    /*
     * "source" ^self ot: objectPointer bits: 8 to: 8
     */

    return OBJMEM_ot_bits_to(objectPointer, 8, 8);
}

// freeBitOf:
int
OBJMEM_freeBitOf(int objectPointer)
{
    /*
     * "source" ^self ot: objectPointer bits: 10 to: 10
     */

    return OBJMEM_ot_bits_to(objectPointer, 10, 10);
}

// locationBitsOf:
int
OBJMEM_locationBitsOf(int objectPointer)
{
    /*
     * "source" self cantBeIntegerObject: objectPointer. ^wordMemory segment:
     * ObjectTableSegment word: ObjectTableStart + objectPointer + 1
     */
    OBJMEM_cantBeIntegerObject(objectPointer);
    return RealMem_segment_word(ObjectTableSegment,
                                ObjectTableStart + objectPointer + 1);
}

// ot:
int
OBJMEM_ot(int objectPointer)
{

    /*
     * "source" self cantBeIntegerObject: objectPointer. ^wordMemory segment:
     * ObjectTableSegment word: ObjectTableStart + objectPointer
     */

    OBJMEM_cantBeIntegerObject(objectPointer);
    return RealMem_segment_word(ObjectTableSegment,
                                ObjectTableStart + objectPointer);
}

// freeBitOf:put:
int
OBJMEM_freeBitOf_put(int objectPointer, int value)
{
    /*
     * "source" ^self ot: objectPointer bits: 10 to: 10 put: value
     */

    return OBJMEM_ot_bits_to_put(objectPointer, 10, 10, value);
}

// classBitsOf:put:
int
OBJMEM_classBitsOf_put(int objectPointer, int value)
{
    /*
     * "source" ^self heapChunkOf: objectPointer word: 1 put: value
     */

    return OBJMEM_heapChunkOf_word_put(objectPointer, 1, value);
}

// heapChunkOf:byte:
int
OBJMEM_heapChunkOf_byte(int objectPointer, int offset)
{
    /*
     * "source" ^wordMemory segment: (self segmentBitsOf: objectPointer) word:
     * ((self locationBitsOf: objectPointer) + (offset//2)) byte: (offset\\2)
     */

    return RealMem_segment_word_byte(OBJMEM_segmentBitsOf(objectPointer),
                                     OBJMEM_locationBitsOf(objectPointer) +
                                     offset / 2, offset % 2);
}

// locationBitsOf:put:
int
OBJMEM_locationBitsOf_put(int objectPointer, int value)
{
    /*
     * "source" self cantBeIntegerObject: objectPointer. ^wordMemory segment:
     * ObjectTableSegment word: ObjectTableStart + objectPointer + 1 put: value
     */
    OBJMEM_cantBeIntegerObject(objectPointer);
    return RealMem_segment_word_put(ObjectTableSegment,
                                    ObjectTableStart + objectPointer + 1,
                                    value);
}

// sizeBitsOf:
int
OBJMEM_sizeBitsOf(int objectPointer)
{
    /*
     * "source" ^self heapChunkOf: objectPointer word: 0
     */

    return OBJMEM_heapChunkOf_word(objectPointer, 0);
}

// oddBitOf:put:
int
OBJMEM_oddBitOf_put(int objectPointer, int value)
{
    /*
     * "source" ^self ot: objectPointer bits: 8 to: 8 put: value
     */
    return OBJMEM_ot_bits_to_put(objectPointer, 8, 8, value);
}

// ot:put:
int
OBJMEM_ot_put(int objectPointer, int value)
{
    /*
     * "source" self cantBeIntegerObject: objectPointer. ^wordMemory segment:
     * ObjectTableSegment word: ObjectTableStart + objectPointer put: value
     */

    OBJMEM_cantBeIntegerObject(objectPointer);
    return RealMem_segment_word_put(ObjectTableSegment,
                                    ObjectTableStart + objectPointer, value);
}

// countBitsOf:put:
int
OBJMEM_countBitsOf_put(int objectPointer, int value)
{
    /*
     * "source" ^self ot: objectPointer bits: 0 to: 7 put: value
     */

    return OBJMEM_ot_bits_to_put(objectPointer, 0, 7, value);
}

// classBitsOf:
int
OBJMEM_classBitsOf(int objectPointer)
{
    /*
     * "source" ^self heapChunkOf: objectPointer word: 1
     */
    return OBJMEM_heapChunkOf_word(objectPointer, 1);

}

// countBitsOf:
int
OBJMEM_countBitsOf(int objectPointer)
{
    /*
     * "source" ^self ot: objectPointer bits: 0 to: 7
     */

    return OBJMEM_ot_bits_to(objectPointer, 0, 7);
}

// ot:bits:to:put:
int
OBJMEM_ot_bits_to_put(int objectPointer,
                      int firstBitIndex, int lastBitIndex, int value)
{
    /*
     * "source" self cantBeIntegerObject: objectPointer. ^wordMemory segment:
     * ObjectTableSegment word: ObjectTableStart + objectPointer bits:
     * firstBitIndex to: lastBitIndex put: value
     */

    OBJMEM_cantBeIntegerObject(objectPointer);
    return RealMem_segment_word_bits_to_put(ObjectTableSegment,
                                            ObjectTableStart + objectPointer,
                                            firstBitIndex, lastBitIndex, value);
}

// sizeBitsOf:put:
int
OBJMEM_sizeBitsOf_put(int objectPointer, int value)
{
    /*
     * "source" ^self heapChunkOf: objectPointer word: 0 put: value
     */

    return OBJMEM_heapChunkOf_word_put(objectPointer, 0, value);
}

// ot:bits:to:
int
OBJMEM_ot_bits_to(int objectPointer, int firstBitIndex, int lastBitIndex)
{
    /*
     * "source" self cantBeIntegerObject: objectPointer. ^wordMemory segment:
     * ObjectTableSegment word: ObjectTableStart + objectPointer bits:
     * firstBitIndex to: lastBitIndex
     */

    OBJMEM_cantBeIntegerObject(objectPointer);
    return RealMem_segment_word_bits_to(ObjectTableSegment,
                                        ObjectTableStart + objectPointer,
                                        firstBitIndex, lastBitIndex);

}

// pointerBitOf:
int
OBJMEM_pointerBitOf(int objectPointer)
{
    /*
     * "source" ^self ot: objectPointer bits: 9 to: 9
     */

    return OBJMEM_ot_bits_to(objectPointer, 9, 9);
}

// --- Allocation ---

#ifdef RUNTIME_CHECKING
void
OBJMEM_runtime_check(bool condition, const char *errorMessage)
{
    if (!condition) {
        assert(0);
        fprintf(stderr, "%s\n", errorMessage);
        abort();
    }
}
#endif
