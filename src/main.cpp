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

bool osIsSmart()
{
    #ifdef _WIN32
    return false;
    #elif _WIN64
    return false;
    #else
    return true;
    #endif
}

int main(int argc, char **argv) {
    if (!osIsSmart()) {
        std::cout << "Unfortunately, your OS is not supported at this time" << std::endl;
        return 1;
    }

    display();
    return 0;
}
