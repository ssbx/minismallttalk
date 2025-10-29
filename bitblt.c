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

#include "bitblt.h"
#include <stdlib.h>
#include <stdint.h>

static const uint16_t AllOnes = 0xFFFF;

const int       _WidthInForm = 1;
const int       _HeightInForm = 2;

/*
 * static const int DestFormIndex = 0; static const int SourceFormIndex = 1;
 * static const int HalftoneFormIndex = 2; static const int
 * CombinationRuleIndex = 3; static const int DestXIndex = 4; static const int
 * DestYIndex = 5; static const int WidthIndex = 6; static const int
 * HeightIndex = 7; static const int SourceXIndex = 8; static const int
 * SourceYIndex = 9; static const int ClipXIndex = 10; static const int
 * ClipYIndex = 11; static const int ClipWidthIndex = 12; static const int
 * ClipHeightIndex = 13;
 */
/*
 * "source" "initialize a table of bit masks ... p.356" RightMasks <- #(0 16r1
 * 16r3 16r7 16rF 16r1F 16r3F 16r7F 16rFF 16r1FF 16r3FF 16r7FF 16rFFF 16r1FFF
 * 16r3FFF 16r7FFF 16rFFFF). AllOnes <- 16rFFFF
 */

int             destX;
int             width;
int             sourceX;

int             destForm;
int             sourceForm;
int             halftoneForm;
int             combinationRule;
int             destX;
int             destY;
int             width;
int             height;
int             sourceX;
int             sourceY;
int             clipX;
int             clipY;
int             clipWidth;
int             clipHeight;

/*
 * charscan 
 */
int             stopConditions;
int             xTable;
int             lastIndex;

/*
 * instance variables (pg. 356 G&R) 
 */
int             sourceBits;
int             sourceRaster;
int             destBits;
int             destRaster;
int             halftoneBits;
int             skew;
int             preload;
int             nWords;
int             hDir;
int             vDir;
int             sourceIndex;
int             sourceDelta;
int             destIndex;
int             destDelta;
int             sx;
int             sy;
int             dx;
int             dy;
int             w;
int             h;

int             sourceBitsWordLength;
int             destBitsWordLength;

/*
 * sourceRaster is the source pitch, and destRaster the destination pitch. // I 
 * guess these terms were unknown to the implementors. 
 */

uint16_t        mask1;
uint16_t        mask2;
uint16_t        skewMask;

/*
 * Additional info 
 */
int             sourceFormWidth;
int             sourceFormHeight;
int             destFormWidth;
int             destFormHeight;

/*
 * Actual area affected by last copyBots 
 */
int             updatedX,
                updatedY,
                updatedWidth,
                updatedHeight;

static uint16_t RightMasks[] = { 0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF,
    0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

/*
 * BitBlt *BB_single = NULL; 
 */

void
BB_init(int adestForm, int asourceForm, int ahalftoneForm,
        int acombinationRule, int adestX, int adestY, int awidth, int aheight,
        int asourceX, int asourceY, int aclipX, int aclipY, int aclipWidth,
        int aclipHeight)
{
    halftoneForm = ahalftoneForm;
    combinationRule = acombinationRule;
    destX = adestX;
    destY = adestY;
    width = awidth;
    height = aheight;
    sourceX = asourceX;
    sourceY = asourceY;
    clipX = aclipX;
    clipY = aclipY;
    clipWidth = aclipWidth;
    clipHeight = aclipHeight;
    destForm = adestForm;
    sourceForm = asourceForm;

    if (sourceForm != NilPointer) {
        sourceFormWidth =
            OBJMEM_integerValueOf(OBJMEM_fetchWord_ofObject
                                  (_WidthInForm, sourceForm));
        sourceFormHeight =
            OBJMEM_integerValueOf(OBJMEM_fetchWord_ofObject
                                  (_HeightInForm, sourceForm));
    } else {
        sourceX = sourceY = 0;
        sourceFormHeight = sourceFormWidth = 0;
    }
    destFormWidth =
        OBJMEM_integerValueOf(OBJMEM_fetchWord_ofObject
                              (_WidthInForm, destForm));
    destFormHeight =
        OBJMEM_integerValueOf(OBJMEM_fetchWord_ofObject
                              (_HeightInForm, destForm));
    updatedX = 0;
    updatedY = 0;
    updatedWidth = 0;
    updatedHeight = 0;
}

void
CHSCAN_init(int adestForm, int asourceForm, int ahalftoneForm,
            int acombinationRule, int adestX, int adestY, int awidth,
            int aheight, int asourceX, int asourceY, int aclipX, int aclipY,
            int aclipWidth, int aclipHeight, int axTable, int alastIndex,
            int astopCondition)
{
    BB_init(adestForm, asourceForm, ahalftoneForm, acombinationRule, adestX,
            adestY, awidth, aheight, asourceX, asourceY, aclipX, aclipY,
            aclipWidth, aclipHeight);
    xTable = axTable;
    lastIndex = alastIndex;
    stopConditions = astopCondition;
}

void
BB_free()
{
    return;
}

/*
 * clipRange 
 */
void            BB_clipRange();
/*
 * merge:with: 
 */
uint16_t        BB_merge_with(uint16_t sourceWord, uint16_t destinationWord);

/*
 * copyLoop 
 */
void            BB_copyLoop();

/*
 * calculateOffsets 
 */
void            BB_calculateOffsets();

/*
 * computeMasks 
 */
void            BB_computeMasks();

/*
 * checkOverlap 
 */
void            BB_checkOverlap();
int             BB_formWordCount(int width, int height);

/*
 * clipRange 
 */
void
BB_clipRange()
{
    /*
     * "source" "clip and adjust source origin and extent appropriately" "first
     * in x" destX >= clipX ifTrue: [sx <- sourceX. dx <- destX. w <- width]
     * ifFalse: [sx <- sourceX + (clipX - destX). w <- width - (clipX - destX).
     * dx <- clipX]. (dx + w) > (clipX + clipWidth) ifTrue: [w <- w - ((dx + w)
     * - (clipX + clipWidth))]. "then in y" destY >= clipY ifTrue: [sy <-
     * sourceY. dy <- destY. h <- height] ifFalse: [sy <- sourceY + clipY -
     * destY. h <- height - (clipY - destY). dy <- clipY]. (dy + h) > (clipY +
     * clipHeight) ifTrue: [h <- h - ((dy + h) - (clipY + clipHeight))]. sx < 0
     * ifTrue: [dx <- dx - sx. w <- w + sx. sx <- 0]. "ERROR dbanay need to
     * check if sourceForm is nil."
     *
     * sx + w > sourceForm width ifTrue: [w <- w - (sx + w - sourceForm width)].
     * sy < 0 ifTrue: [dy <- dy - sy. h <- h + sy. sy <- 0]. sy + h > sourceForm
     * height ifTrue: [h <- h - (sy + h - sourceForm height)]
     */

    if (clipX < 0) {
        clipWidth += clipX;
        clipX = 0;
    }
    if (clipY < 0) {
        clipHeight += clipY;
        clipY = 0;
    }

    if ((clipX + clipWidth) > destFormWidth) {
        clipWidth = destFormWidth - clipX;
    }

    if ((clipY + clipHeight) > destFormHeight) {
        clipHeight = destFormHeight - clipY;
    }
    /*
     * clip and adjust source origin and extent appropriately // first in x 
     */
    if (destX >= clipX) {
        sx = sourceX;
        dx = destX;
        w = width;
    } else {
        sx = sourceX + (clipX - destX);
        w = width - (clipX - destX);
        dx = clipX;
    }
    if ((dx + w) > (clipX + clipWidth)) {
        w = w - ((dx + w) - (clipX + clipWidth));
    }
    /*
     * then in y 
     */
    if (destY >= clipY) {
        sy = sourceY;
        dy = destY;
        h = height;
    } else {
        sy = sourceY + clipY - destY;
        h = height - (clipY - destY);
        dy = clipY;
    }
    if ((dy + h) > (clipY + clipHeight)) {
        h = h - ((dy + h) - (clipY + clipHeight));
    }

    if (sourceForm == NilPointer)
        return;

    if (sx < 0) {
        dx = dx - sx;
        w = w + sx;
        sx = 0;
    }

    if (sx + w > sourceFormWidth) {
        w = w - (sx + w - sourceFormWidth);
    }

    if (sy < 0) {
        dy = dy - sy;
        h = h + sy;
        sy = 0;
    }

    if (sy + h > sourceFormHeight) {
        h = h - (sy + h - sourceFormHeight);
    }
}

/*
 * // WordArray class>>maxSize // maxSize // "The maximum size of a WordArray
 * is 64640 elements." // ^64640 // // When a Form is allocated, the number of
 * words required to hold the bits is // (width + 15)/16 * height. If the form
 * is large enough this will exceed // maxSize // and rather than fail,
 * Smalltalk clamps the array size to 64640! This mucks // things up if such a
 * form is used as a source or target. 
 */
/*
 * copyBits 
 */
bool
BB_copyBits()
{
    /*
     * "source" "from copyBits, p.356" "sets w and h" self clipRange. (w <= 0
     * or: [h <= 0]) ifTrue: [^self ]. "null range" self computeMasks. self
     * checkOverlap. self calculateOffsets. self copyLoop
     */

    /*
     * from copyBits, p.356 
     */
    BB_clipRange();
    if (w > 0 && h > 0) {
        updatedX = dx;
        updatedY = dy;
        updatedWidth = w;
        updatedHeight = h;
        BB_computeMasks();
        /*
         * Check if source or dest is "bad" 
         */
        if (sourceForm != NilPointer
            && BB_formWordCount(sourceFormWidth, sourceFormHeight)
            != sourceBitsWordLength)
            return false;
        if (BB_formWordCount(destFormWidth, destFormHeight) !=
            destBitsWordLength)
            return false;

        BB_checkOverlap();
        BB_calculateOffsets();
        BB_copyLoop();
    } else {
        updatedX = 0;
        updatedY = 0;
        updatedWidth = 0;
        updatedHeight = 0;

    }

    return true;

}

/*
 * copyLoop 
 */
void
BB_copyLoop()
{
    uint16_t        prevWord;
    uint16_t        thisWord = 0;       /* eliminate warning */
    uint16_t        skewWord;
    uint16_t        mergeMask;
    uint16_t        halftoneWord;
    uint16_t        mergeWord;
    uint16_t        destWord;
    int             word;

    /*
     * "source" 1 to: h do: "here is the vertical loop" [ :i | (halftoneForm
     * notNil) ifTrue: [halftoneWord <- halftoneBits at: (1 + (dy bitAnd: 15)).
     * dy <- dy + vDir] ifFalse: [halftoneWord <- AllOnes]. skewWord <-
     * halftoneWord. preload ifTrue: [prevWord <- sourceBits at: sourceIndex +
     * 1. "load the 32-bit shifter" sourceIndex <- sourceIndex + hDir] ifFalse:
     * [prevWord <- 0]. mergeMask <- mask1. "ERROR: extra to on next line" 1 to:
     * nWords do: "here is the inner horizontal loop" [ :word | sourceForm
     * notNil "if source used" ifTrue: [prevWord <- prevWord bitAnd: skewMask.
     * "ERROR: dbanay need to check if out of range. use 0 if that is the case"
     * thisWord <- sourceBits at: sourceIndex + 1. "pick up next word" skewWord
     * <- prevWord bitOr: (thisWord bitAnd: skewMask bitInvert). prevWord <-
     * thisWord. skewWord <- (skewWord bitShift: skew) bitOr: (skewWord
     * bitShift: skew - 16)]. "16-bit rotate" mergeWord <- self merge: (skewWord
     * bitAnd: halftoneWord) with: (destBits at: destIndex + 1). destBits at:
     * destIndex + 1 put: ((mergeMask bitAnd: mergeWord) bitOr: (mergeMask
     * bitInvert bitAnd: (destBits at: destIndex + 1))). sourceIndex <-
     * sourceIndex + hDir. destIndex <- destIndex + hDir. word = (nWords - 1)
     * ifTrue: [mergeMask <- mask2] ifFalse: [mergeMask <- AllOnes]].
     * sourceIndex <- sourceIndex + sourceDelta. destIndex <- destIndex +
     * destDelta]
     */

    int             i;
    for (i = 1; i <= h; i++) {
        /*
         * here is the vertical loop 
         */
        if (halftoneForm != NilPointer) {
            halftoneWord = OBJMEM_fetchWord_ofObject((dy & 15), halftoneBits);
            dy = dy + vDir;
        } else {
            halftoneWord = AllOnes;
        }
        skewWord = halftoneWord;
        if (preload) {
            /*
             * load the 32-bit shifter 
             */

            prevWord = OBJMEM_fetchWord_ofObject(sourceIndex, sourceBits);
            sourceIndex = sourceIndex + hDir;
        } else {
            prevWord = 0;
        }
        mergeMask = mask1;

        /*
         * here is the inner horizontal loop 
         */
        for (word = 1; word <= nWords; word++) {
            if (sourceForm != NilPointer) {     /* if source used */
                prevWord = prevWord & skewMask;
                if (word <= sourceRaster && sourceIndex >= 0
                    && sourceIndex < sourceBitsWordLength)
                    thisWord =
                        OBJMEM_fetchWord_ofObject(sourceIndex, sourceBits);
                /*
                 * else 
                 */
                /*
                 * thisWord = 0; 
                 */
                skewWord = prevWord | (thisWord & ~skewMask);

                prevWord = thisWord;
                /*
                 * Note: replaced left shift by negative value with a right 
                 */
                /*
                 * shift of complement 
                 */
                skewWord = (skewWord << skew) | (skewWord >> (16 - skew));
            }

            if (destIndex >= destBitsWordLength)
                return;

            destWord = OBJMEM_fetchWord_ofObject(destIndex, destBits);
            /*
             * 16-bit rotate 
             */
            mergeWord = BB_merge_with(skewWord & halftoneWord, destWord);

            OBJMEM_storeWord_ofObject_withValue(destIndex, destBits,
                                                (mergeMask & mergeWord) |
                                                (~mergeMask & destWord));
            sourceIndex = sourceIndex + hDir;
            destIndex = destIndex + hDir;

            if (word == (nWords - 1))
                mergeMask = mask2;
            else
                mergeMask = AllOnes;
        }

        sourceIndex = sourceIndex + sourceDelta;
        destIndex = destIndex + destDelta;

    }
}

/*
 * calculateOffsets 
 */
void
BB_calculateOffsets()
{
    /*
     * "source" "check if need to preload buffer (i.e., two words of source
     * needed for first word of destination)" preload <- (sourceForm notNil)
     * and: [skew ~= 0 and: [skew <= (sx bitAnd: 15)]]. hDir < 0 ifTrue:
     * [preload <- preload == false]. "calculate starting offsets" sourceIndex
     * <- sy * sourceRaster + (sx // 16). destIndex <- dy * destRaster + (dx //
     * 16). "calculate increments from end of 1 line to start of next"
     * sourceDelta <- (sourceRaster * vDir) - (nWords + (preload ifTrue: [1]
     * ifFalse: [0]) * hDir). destDelta <- (destRaster * vDir) - (nWords *
     * hDir)
     */

    /*
     * check if need to preload buffer 
     */
    /*
     * (i.e., two words of source needed for first word of destination) 
     */
    preload = (sourceForm != NilPointer) && skew != 0 && skew <= (sx & 15);
    if (hDir < 0)
        preload = !preload;

    /*
     * calculate starting offsets 
     */
    sourceIndex = sy * sourceRaster + (sx / 16);
    destIndex = dy * destRaster + (dx / 16);
    /*
     * calculate increments from end of 1 line to start of next 
     */
    /*
     * dbanay - note operator precedence change! 
     */
    sourceDelta = (sourceRaster * vDir) - ((nWords + (preload ? 1 : 0)) * hDir);
    destDelta = (destRaster * vDir) - (nWords * hDir);
}

/*
 * computeMasks 
 */
void
BB_computeMasks()
{
    int             startBits;
    int             endBits;

    /*
     * "source" "calculate skew and edge masks" destBits <- destForm bits.
     * destRaster <- destForm width - 1 // 16 + 1. sourceForm notNil ifTrue:
     * [sourceBits <- sourceForm bits. sourceRaster <- sourceForm width - 1 //
     * 16 + 1]. halftoneForm notNil ifTrue: [halftoneBits <- halftoneForm
     * bits]. skew <- (sx - dx) bitAnd: 15. "how many bits source gets skewed
     * to right" startBits <- 16 - (dx bitAnd: 15). " how many bits in first
     * word" mask1 <- RightMasks at: startBits + 1. endBits <- 15 - ((dx + w -
     * 1) bitAnd: 15). "how many bits in last word" mask2 <- (RightMasks at:
     * endBits + 1) bitInvert. skewMask <- (skew = 0 ifTrue: [0] ifFalse:
     * [RightMasks at: 16 - skew + 1]). "determine number of words stored per
     * line; merge masks if necessary" "ERROR dbanay : nWords <- (w - startBits
     * + 15) // 16 + 1 for False case" w < startBits ifTrue: [mask1 <- mask1
     * bitAnd: mask2. mask2 <- 0. nWords <- 1] ifFalse: [nWords <- (w -
     * startBits - 1) // 16 + 2]
     */

    const int       BitsInForm = 0;

    /*
     * calculate skew and edge masks 
     */
    destBits = OBJMEM_fetchPointer_ofObject(BitsInForm, destForm);
    destBitsWordLength = OBJMEM_fetchWordLengthOf(destBits);

    destRaster = (destFormWidth - 1) / 16 + 1;
    if (sourceForm != NilPointer) {
        sourceBits = OBJMEM_fetchPointer_ofObject(BitsInForm, sourceForm);
        sourceBitsWordLength = OBJMEM_fetchWordLengthOf(sourceBits);
        sourceRaster = (sourceFormWidth - 1) / 16 + 1;
    } else
        sourceBitsWordLength = 0;

    if (halftoneForm != NilPointer) {
        halftoneBits = OBJMEM_fetchPointer_ofObject(BitsInForm, halftoneForm);
    }
    /*
     * how many bits source gets skewed to right 
     */
    skew = (sx - dx) & 15;

    /*
     * how many bits in first word 
     */
    startBits = 16 - (dx & 15);

    mask1 = RightMasks[startBits];      /* +1 removed - dbanay -- C/C++ arrays */
    /*
     * start at 0 
     */

    /*
     * how many bits in last word 
     */
    endBits = 15 - ((dx + w - 1) & 15);

    mask2 = ~RightMasks[endBits];       /* +1 removed - dbanay -- C/C++ arrays */
    /*
     * start at 0 
     */
    skewMask = skew == 0 ? 0 : RightMasks[16 - skew];
    /*
     * determine number of words stored per line; merge masks if necessary 
     */
    /*
     * Bluebook had the nWords calculation wrong when w == startBits 
     */
    if (w < startBits) {
        mask1 = mask1 & mask2;
        mask2 = 0;
        nWords = 1;
    } else {
        nWords = (w - startBits + 15) / 16 + 1;
    }
}

/*
 * checkOverlap 
 */
void
BB_checkOverlap()
{
    int             t;

    /*
     * "source" "check for possible overlap of source and destination" hDir <-
     * vDir <- 1. "defaults for no overlap" (sourceForm == destForm and: [dy >=
     * sy]) ifTrue: [dy > sy "have to start at bottom" ifTrue: [vDir <- -1. sy
     * <- sy + h - 1. dy <- dy + h - 1] ifFalse: [dx > sx "y's are equal, but
     * x's are backward" ifTrue: [hDir <- -1. sx <- sx + w - 1. "start at
     * right" dx <- dx + w - 1. "and fix up masks" skewMask <- skewMask
     * bitInvert. t <- mask1. mask1 <- mask2. mask2 <- t]]]
     */

    /*
     * check for possible overlap of source and destination 
     */
    hDir = vDir = 1;            /* defaults for no overlap */
    if (sourceForm == destForm && (dy >= sy)) {
        if (dy > sy) {          /* have to start at bottom */
            vDir = -1;
            sy = sy + h - 1;
            dy = dy + h - 1;
        } else {
            if (dx > sx) {
                /*
                 * y's are equal, but x's are backward 
                 */
                hDir = -1;
                /*
                 * start at right 
                 */
                sx = sx + w - 1;
                dx = dx + w - 1;
                /*
                 * and fix up masks 
                 */
                skewMask = ~skewMask;
                t = mask1;
                mask1 = mask2;
                mask2 = t;
            }
        }
    }
}

void
BB_getUpdatedBounds(int *boundsX, int *boundsY, int *boundsWidth,
                    int *boundsHeight)
{
    *boundsX = updatedX;
    *boundsY = updatedY;
    *boundsWidth = updatedWidth;
    *boundsHeight = updatedHeight;
}

uint16_t
BB_merge_with(uint16_t sourceWord, uint16_t destinationWord)
{
    /*
     * "source" "These are the 16 combination rules:" combinationRule = 0
     * ifTrue: [^0]. combinationRule = 1 ifTrue: [^sourceWord bitAnd:
     * destinationWord]. combinationRule = 2 ifTrue: [^sourceWord bitAnd:
     * destinationWord bitInvert]. combinationRule = 3 ifTrue: [^sourceWord].
     * combinationRule = 4 ifTrue: [^sourceWord bitInvert bitAnd:
     * destinationWord]. combinationRule = 5 ifTrue: [^destinationWord].
     * combinationRule = 6 ifTrue: [^sourceWord bitXor: destinationWord].
     * combinationRule = 7 ifTrue: [^sourceWord bitOr: destinationWord].
     * combinationRule = 8 ifTrue: [^sourceWord bitInvert bitAnd:
     * destinationWord bitInvert]. combinationRule = 9 ifTrue: [^sourceWord
     * bitInvert bitXor: destinationWord]. combinationRule = 10 ifTrue:
     * [^destinationWord bitInvert]. combinationRule = 11 ifTrue: [^sourceWord
     * bitOr: destinationWord bitInvert]. combinationRule = 12 ifTrue:
     * [^sourceWord bitInvert]. combinationRule = 13 ifTrue: [^sourceWord
     * bitInvert bitOr: destinationWord]. combinationRule = 14 ifTrue:
     * [^sourceWord bitInvert bitOr: destinationWord bitInvert].
     * combinationRule = 15 ifTrue: [^AllOnes]
     */

    /*
     * These are the 16 combination rules: 
     */
    switch (combinationRule) {
    case 0:
        return 0;
        case 1                  /* [^sourceWord bitAnd: destinationWord] */
    :
        return sourceWord & destinationWord;

        case 2                  /* [^sourceWord bitAnd: destinationWord */
            /*
             * bitInvert] 
             */
    :
        return sourceWord & (~destinationWord);

        case 3                  /* [^sourceWord] */
    :
        return sourceWord;

        case 4                  /* [^sourceWord bitInvert bitAnd: */
            /*
             * destinationWord] 
             */
    :
        return (~sourceWord) & destinationWord;

        case 5                  /* [^destinationWord] */
    :
        return destinationWord;

        case 6                  /* [^sourceWord bitXor: destinationWord] */
    :
        return sourceWord ^ destinationWord;

        case 7                  /* [^sourceWord bitOr: destinationWord] */
    :
        return sourceWord | destinationWord;

        case 8                  /* [^sourceWord bitInvert bitAnd: */
            /*
             * destinationWord bitInvert]. 
             */
    :
        return (~sourceWord) & (~destinationWord);

        case 9                  /* [^sourceWord bitInvert bitXor: */
            /*
             * destinationWord]. 
             */
    :
        return (~sourceWord) ^ destinationWord;

        case 10                 /* [^destinationWord bitInvert] */
    :
        return ~destinationWord;

        case 11                 /* [^sourceWord bitOr: destinationWord */
            /*
             * bitInvert]. 
             */
    :
        return sourceWord | (~destinationWord);

        case 12                 /* [^sourceWord bitInvert] */
    :
        return ~sourceWord;

        case 13                 /* [^sourceWord bitInvert bitOr: */
            /*
             * destinationWord] 
             */
    :
        return (~sourceWord) | destinationWord;

        case 14                 /* [^sourceWord bitInvert bitOr: */
            /*
             * destinationWord bitInvert]. 
             */
    :
        return (~sourceWord) | (~destinationWord);

    case 15:
        return AllOnes;
    default:
        assert(0);
    }

    return 0;
}

int
BB_formWordCount(int width, int height)
{
    return (width + 15) / 16 * height;
}

int
CHSCAN_scanCharactersFrom_to_in_rightX_stopConditions_displaying(int startIndex,
                                                                 int stopIndex,
                                                                 int
                                                                 sourceString,
                                                                 int rightX,
                                                                 int stops, bool
                                                                 displaying)
{
    const int       EndOfRun = 257;     /* TextConstants at: #EndOfRun put: */
    /*
     * 257. 
     */
    const int       CrossedX = 258;     /* TextConstants at: #CrossedX put: */
    /*
     * 258. 
     */

    /*
     * "source" | ascii nextDestX | lastIndex <- startIndex. [lastIndex <=
     * stopIndex] whileTrue: [ascii <- (sourceString at: lastIndex) asciiValue.
     * (stopConditions at: ascii + 1) ~~ nil ifTrue: [^stops at: ascii + 1].
     * sourceX <- xTable at: ascii + 1. nextDestX <- destX + (width _ (xTable
     * at: ascii + 2) - sourceX). nextDestX > rightX ifTrue: [^stops at:
     * CrossedX]. display ifTrue: [self copyBits]. destX <- nextDestX. lastIndex
     * <- lastIndex + 1]. lastIndex <- stopIndex. ^stops at: EndOfRun
     *
     *
     */
    int             ascii;
    int             nextDestX;

    lastIndex = startIndex;
    while (lastIndex <= stopIndex) {
        ascii = OBJMEM_fetchByte_ofObject(lastIndex - 1, sourceString);

        if (OBJMEM_fetchPointer_ofObject(ascii, stopConditions) != NilPointer) {
            return OBJMEM_fetchPointer_ofObject(ascii, stops);
        }

        sourceX =
            OBJMEM_integerValueOf(OBJMEM_fetchPointer_ofObject(ascii, xTable));
        width =
            OBJMEM_integerValueOf(OBJMEM_fetchPointer_ofObject
                                  (ascii + 1, xTable)) - sourceX;
        nextDestX = destX + width;
        if (nextDestX > rightX) {
            return OBJMEM_fetchPointer_ofObject(CrossedX - 1, stops);
        }
        if (displaying)
            BB_copyBits();
        destX = nextDestX;
        lastIndex = lastIndex + 1;
    }

    lastIndex = stopIndex;
    return OBJMEM_fetchPointer_ofObject(EndOfRun - 1, stops);
}

int
CHSCAN_getUpdateDestX()
{
    return destX;
}
int
CHSCAN_getUpdatedWidth()
{
    return width;
}
int
CHSCAN_getUpdatedSourceX()
{
    return sourceX;
}
int
CHSCAN_getUpdatedLastIndex()
{
    return lastIndex;
}
void
CHSCAN_free()
{
    return;
}
