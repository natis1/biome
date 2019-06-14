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

#pragma once

/**
 * @todo write docs lol
 */

#include "forest.h"
#include <sys/stat.h>

const std::string STARTING_RANDOM_NAMES[] = { "West", "North", "East", "South", "Blue", "Green", "Rolling", "Red", "Fort", "Death", "First", "Starry", "Hidden"};
const std::string RANDOM_NAMES[] = { "Gale", "Gulch", "River", "Forest", "Lake", "Wood", "Air", "Hill", "Stream", "Night", "Flame", "Dune", "Ground", "Fall"};
const size_t STARTING_RANDOM_NAMES_SIZE = std::extent<decltype(STARTING_RANDOM_NAMES)>::value;
const size_t RANDOM_NAMES_SIZE = std::extent<decltype(RANDOM_NAMES)>::value;
const std::string BIOME_DESCRIPTIONS[] = {
    "Swamp\n + Nutrient rich\n + Moss\n + Woodlice", "Mountain\n + Easily defensible\n + Cycads\n + Wasps",
    "Desert\n + Hidden oasis\n + Cacti\n + Beetles", "Chaparral\n + Perfect weather\n + Sequoia\n + Ants",
    "Jungle\n + Full of life\n + Orchids\n + Bees", "Taiga\n + Winter wonderland\n + Conifers\n + Moths"
};

const size_t NUM_BIOMES = std::extent<decltype(BIOME_DESCRIPTIONS)>::value;


class display
{
public:
    display();
    static void sigabrtHandler(int sig);
private:
    std::string getForestName();
    void forestNameHelper(bool state);
    void biomeTypeHelper(int biome);
    int getBiomeType();
    int getDir(std::string dir, std::vector<std::string> &files);
    bool mainMenu();
    std::string mostRecentFile(std::vector<std::string> *files);

    std::string savePath;
    forest::saveFile sfile;
    forest::optionsFile ofile;

};

