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

#ifndef FOREST_H
#define FOREST_H

#include "tools.h"


namespace forest
{
    const int DATA_VERSION = 3;

    struct biome {
        std::string name;
        std::string plantName;
        std::string insectName;
        short uninhabitedColorPair[6];
        short inhabitedColorPair[6];
        std::vector<int> uninhabitedSymbols;
        std::vector<int> inhabitedSymbols;
    };

    const biome biomes[] {
        // Uninhabited- Water which is muddy blue on brown. Inhabited- Moss which is very green on brown.
        // Uninhabited is water. ~ . - . Inhabited is moss: m, M
        {"Swamp", "Moss", "Woodlice", {300, 300, 500, 500, 300, 0}, {200, 850, 75, 400, 300, 0}, {'~', '.', '-'}, {'m', 'M'}},
        // Uninhabited is gray rocks on light brown background. Inhabited is deep green on dark brown.
        // Uninhabited is mostly empty with occasional rocks. Inhabited is '^', 't', 'Y'
        {"Mountain", "Cycads", "Wasps", {500, 500, 500, 850, 850, 500}, {100, 500, 100, 250, 250, 0}, {' ', ' ', ' ', ' ', ' ', 'o'}, {'^', 't', 'Y'}},
        // Uninhabited is yellow sand: '.', '`', ',', '*', ''' on grey background (mostly to not hurt people's eyes).
        // Inhabited is light green cacti: 'Y', 'I', 'V', '|' on darker grey background.
        {"Desert", "Cacti", "Beetles", {850, 850, 0, 500, 500, 500}, {500, 1000, 500, 300, 300, 300}, {'.', '`', ',', '*', '\''}, {'Y', 'I', 'V', '|'}},
        // Uninhabited is orange sand: '.', '`', ',', '*', ''' on grey background (mostly to not hurt people's eyes).
        // Inhabited is big dark green trees: 'T', '|', 'I' on dark greenish background.
        {"Chaparral", "Sequoia", "Ants", {900, 550, 0, 500, 500, 500}, {100, 500, 0, 0, 100, 0}, {'.', '`', ',', '*', '\''}, {'T', '|', 'I'}},
        // Uninhabited is very brown trees: 't', 'I', '|', '^', 'Y' on dark greenish background
        // Inhabited is orange orchids: 'S', 'p', 'R', 'Q', 'g', 'J', 'U', 'b' on dark greenish background
        {"Jungle", "Orchids", "Bees", {1, 2, 3, 0, 100, 0}, {1, 2, 3, 0, 100, 0}, {'t', 'I', '|', '^', 'Y'}, {'S', 'p', 'R', 'Q', 'g', 'J', 'U', 'b'}},
        // Uninhabited is light blue/cyan snow: '.', '`', ',', '*', ''' on dark blue background (mostly to not hurt people's eyes).
        // Inhabited is vivid green trees: 'T', 't', 'I', 'Y', '^' on dark greenish background
        {"Taiga", "Conifers", "Moths", {600, 850, 850, 100, 100, 200}, {100, 900, 0, 0, 100, 0}, {'.', '`', ',', '*', '\''}, {'T', 't', 'I', 'Y', '^'}}
    };


    struct saveFile {
        long dataVersion;
        long biomeType;
        long trees;
        long productiveSeconds;
        double supermeme;
        std::string name;

        bool operator==(const saveFile& rhs) const {
            return std::tie(dataVersion, biomeType, trees, productiveSeconds) == std::tie(rhs.dataVersion, rhs.biomeType, rhs.trees, rhs.productiveSeconds);
        }

        constexpr static auto properties = std::make_tuple(
            property(&saveFile::dataVersion, "dataVersion"),
            property(&saveFile::biomeType, "biomeType"),
            property(&saveFile::trees, "trees"),
            property(&saveFile::productiveSeconds, "productiveSeconds"),
            property(&saveFile::name, "name"),
            property(&saveFile::supermeme, "supermeme")
        );
    };

    struct optionsFile {
        long optionsVersion;
        long zoomLevel;
    };

    struct saveFile* newForest(saveFile *s, long biome);
}

#endif // FOREST_H
