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
#ifndef _BITBLT_H_
#define _BITBLT_H_

#include <assert.h>
#include <stdint.h>

#include "objectmemory.h"
#include "minist8.h"


void            BB_init(int destForm, int sourceForm, int halftoneForm,
                        int combinationRule, int destX, int destY, int width,
                        int height, int sourceX, int sourceY, int clipX,
                        int clipY, int clipWidth, int clipHeight);

bool            BB_copyBits();
void            BB_getUpdatedBounds(int *, int *, int *, int *);
void            BB_free();

void            CHSCAN_init(int destForm, int sourceForm, int halftoneForm,
                            int combinationRule, int destX, int destY,
                            int width, int height, int sourceX, int sourceY,
                            int clipX, int clipY, int clipWidth, int clipHeight,
                            int xTable, int intlastIndex, int stopCondition);

int             CHSCAN_getUpdateDestX();
int             CHSCAN_getUpdatedWidth();
int             CHSCAN_getUpdatedSourceX();
int             CHSCAN_getUpdatedLastIndex();
int
















CHSCAN_scanCharactersFrom_to_in_rightX_stopConditions_displaying(int startIndex,
                                                                 int stopIndex,
                                                                 int
                                                                 sourceString,
                                                                 int rightX,
                                                                 int stop, bool
                                                                 displaying);

void            CHSCAN_free();

#endif                          /* _BITBLT_H_ */
