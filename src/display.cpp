/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2019  <copyright holder> <email>
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

display::display()
{
    std::string biomefolder = std::getenv("HOME");
    biomefolder += "/.config/biome/";
    std::cout << "Saving data to " << biomefolder << std::endl;
    mkdir(biomefolder.c_str(), 0755);
    this->savePath = biomefolder;

    if (std::ifstream(biomefolder + "forest")) {
        sfile = fromJson<forest::saveFile>(pseudojson::fileToPseudoJson(biomefolder + "forest"));
        std::cout << "loaded existing file" << std::endl;
    } else {
        forest::newForest(&sfile, 3);
        pseudojson::writeToFile(toJson(sfile), biomefolder + "forest");
        std::cout << "created new file" << std::endl;
    }

    std::cout << sfile.biomeType << " is biome type " << sfile.name << " is name" << std::endl;
}
