/*
 * Copyright © 2025 Sébastien Serre. All rights reserved.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include "minist8.h"
#include "runtime.h"
#include "system.h"
#include "interpreter.h"

Options         G_Options;

void
__Run()
{
    int             i;

    for (;;) {
        System_ProcessEvents();
        System_CheckScheduledSemaphore();
        Interp_CheckLowMemoryConditions();

        for (i = 0; i < G_Options.cyclesPerFrame && !System_QuitSignalled();
             i++) {
            INTERP_Cycle(true);
        };

        if (System_ReadStdin()) {
            Interp_AsynchronousSignal(System_GetStdinInputSemaphore());
        };

        if (System_QuitSignalled())
            break;

        System_UpdateScreen();
    }
}


int
main(int argc, char *argv[])
{

    int ret = Runtime_Getopts(argc, argv);

    if (ret == GETOPT_STATUS_OK_QUIT)
      exit(0);
    else if (ret == GETOPT_STATUS_ERROR)
      exit(GETOPT_STATUS_ERROR);

    if (!System_Init())
        exit(EXIT_FAILURE);
    if (!INTERP_Init(System_GetImageName()))
        exit(EXIT_FAILURE);

    __Run();

    System_Quit();

    free(G_Options.imgName);
    free(G_Options.rootDir);

    return 0;
}
