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

extern Options      G_Options;

static void
__PrintHelp(char *name, FILE * out)
{
    fprintf(out, "usage: %s [-v] [-d ms] [-v | -d | -t ]"
            "[-c cycles-per-frame] [-x] [-i snapshot] [-t] [-g]\n", name);
    fprintf(out,
            "   -a               screen auto adapt. set -c and -w optimaly\n");
    fprintf(out, "   -s               use vsync. Default yes\n");
    fprintf(out, "   -w DELAY_MS      estimated refresh rate. default 60\n");
    fprintf(out, "   -c NUM           cycles per frame. Default 5000\n");
    fprintf(out, "   -x               2x scale. Default false\n");
    fprintf(out, "   -g               open gl dev window. Default false\n");
    fprintf(out,
            "   -i FILE_NAME     snapshot name. Default \"snapshot.im\"\n");
    fprintf(out, "   -t               emulate three button. Default false\n");
    fprintf(out, "   -v               verbose\n");
    fprintf(out, "   -d               debug\n");
    fprintf(out, "   -t               trace\n");
    fprintf(out, "   -o STRING        image options\n");
    fprintf(out, "\n");
    fprintf(out, "Version: %i.%i.%i\n", M8_VMAJ, M8_VMIN, M8_VPATCH);
}

int
Runtime_Getopts(int argc, char *argv[])
{
    char           *imgPath = NULL;
    int             opt;
    char           *cdir;
    char           *separator;
    int             len;

    G_Options.currDir = NULL;
    G_Options.rootDir = NULL;
    G_Options.imgName = NULL;
    G_Options.imgArgv = NULL;
    G_Options.threeButtons = false;
    G_Options.autoAdapt = false;
    G_Options.useVsync = false;
    G_Options.verbose = false;
    G_Options.debug = false;
    G_Options.trace = false;
    G_Options.withLibui = false;
    G_Options.noVsyncDelay = 1000 / 60; /* Try -delay 8 arg if your CPU is */

    /*
     * unhappy
     */
    G_Options.cyclesPerFrame = 5000;
    G_Options.displayScale = 1;

    while ((opt = getopt(argc, argv, "gamwhvsxti:d:c:o:")) != -1) {
        switch (opt) {
        case 's':
            G_Options.useVsync = true;
            break;
        case 'a':
            G_Options.autoAdapt = true;
            break;
        case 'v':
            G_Options.verbose = true;
            break;
        case 'd':
            G_Options.debug = true;
            break;
        case 'g':
            G_Options.withLibui = true;
            break;
        case 't':
            G_Options.trace = true;
            break;
        case 'x':
            G_Options.displayScale = 2;
            break;
        case 'm':
            G_Options.threeButtons = true;
            break;
        case 'w':
            G_Options.noVsyncDelay = atoi(optarg);
            break;
        case 'c':
            G_Options.cyclesPerFrame = atoi(optarg);
            break;
        case 'i':
            imgPath = strdup(optarg);
            break;
        case 'o':
            G_Options.imgArgv = strdup(optarg);
            break;
        case 'h':
            __PrintHelp(argv[0], stdout);
            return GETOPT_STATUS_OK_QUIT;
        default:
            __PrintHelp(argv[0], stderr);
            return GETOPT_STATUS_ERROR;
        }
    }

    cdir = malloc(sizeof(char) * PATH_MAX);
    if (getcwd(cdir, PATH_MAX) == NULL) {
        perror("cwd error");
        abort();
    }
    G_Options.currDir = realloc(cdir, strlen(cdir) + 1);

    if (imgPath == NULL)
        imgPath = strdup("snapshot.im");

    separator = strrchr(imgPath, '/');
    if (separator == NULL) {
        len = strlen(imgPath) + strlen(G_Options.currDir) + 2;
        G_Options.rootDir = malloc(sizeof(char) * len);
        snprintf(G_Options.rootDir, len, "%s/%s", G_Options.currDir, imgPath);
        G_Options.imgName = imgPath;
    } else {
        G_Options.imgName = strdup(++separator);
        *separator = '\0';
        len = strlen(G_Options.currDir) + strlen(imgPath) + 2;
        G_Options.rootDir = malloc(sizeof(char) * len);
        snprintf(G_Options.rootDir, len, "%s/%s", G_Options.currDir, imgPath);
    }

    if (G_Options.verbose) {
        printf("snapname: %s\n", G_Options.imgName);
        printf("rootdir: %s\n", G_Options.rootDir);
        printf("currdir: %s\n", G_Options.currDir);
        printf("use three_button: %i\n", G_Options.threeButtons);
        printf("use vsync: %i\n", G_Options.useVsync);
        printf("auto adapt: %i\n", G_Options.autoAdapt);
        printf("novsync_delay: %i\n", G_Options.noVsyncDelay);
        printf("cycles_per_frame: %i\n", G_Options.cyclesPerFrame);
        printf("display_scale: %i\n", G_Options.displayScale);
        printf("debug: %i\n", G_Options.debug);
        printf("trace: %i\n", G_Options.trace);
    }
    return GETOPT_STATUS_OK_CONTINUE;
}

void
Runtime_Freeopts()
{
    if (G_Options.currDir) free(G_Options.currDir);
    if (G_Options.imgName) free(G_Options.imgName);
    if (G_Options.imgArgv) free(G_Options.imgArgv);
    if (G_Options.rootDir) free(G_Options.rootDir);
}

