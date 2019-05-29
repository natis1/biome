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
        double growthModifier;
        int defaultColor;
        std::string name;
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
