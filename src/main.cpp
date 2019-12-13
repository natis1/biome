/*
 * Biome, A simple ncurses productivity program, made while procrastinating.
 * Copyright (C) 2019  Eli Stone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "display.h"
#include <zconf.h>
#include <signal.h>

#if !defined(__CYGWIN__)
#include <execinfo.h>
#endif

#if !defined(__CYGWIN__)

const size_t HANDLER_TRACEBACK_SIZE = 20;

void handler(int sig) {
    void *array[HANDLER_TRACEBACK_SIZE];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, HANDLER_TRACEBACK_SIZE);

    // print out all the frames to stderr
    std::cerr << "Segfault. Size is: " << size << std::endl;
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif

int main() {
#if !defined(__CYGWIN__)
    signal(SIGSEGV, handler);
#endif
    new display();
    return 0;
}
