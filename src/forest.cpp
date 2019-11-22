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

#include "forest.h"
#include <stdio.h>
#include <string.h>


using namespace forest;

struct optionsFile * forest::newOptions(optionsFile* o)
{
    o->optionsVersion = OPTIONS_VERSION;
    o->blockInternet = 0;
    o->blockWebsites = 0;
    o->idealHoursPerWeek = 7;
    return o;
}


struct saveFile* forest::newForest(saveFile *s, long biome = 0, std::string name = "invalid")
{
    s->biomeType = biome;
    s->dataVersion = DATA_VERSION;
    s->productiveSeconds = 0;
    s->trees = 0;
    s->name = name;
    s->biomeSeed = time(0);
    s->lastRunTime = 0;
    s->dailyStreak = 0;
    for (int i = 0; i < 7; i++) {
        s->weeklyRundifficulties.push_back(0.0);
        s->weeklyRuntimes.push_back(0.0);
    }

    pseudojson::Value jsonForest = toJson(*s);
    return s;
}

struct saveFile * forest::updateForest(forest::saveFile* s)
{
    // Pre 5
    if (s->dataVersion < 5) {
        s->biomeSeed = time(0);
        s->lastRunTime = 0;
        s->dailyStreak = 0;
        for (int i = 0; i < 7; i++) {
            s->weeklyRundifficulties.push_back(0.0);
            s->weeklyRuntimes.push_back(0.0);
        }
    }


    pseudojson::Value jsonForest = toJson(*s);

    for( auto const& [key, val] : jsonForest.data.subObject ) {
        std::cout << key << " : " << pseudojson::getValue(&val.data) << std::endl;
    }

    pseudojson::writeToFile(jsonForest, "test.txt");

    s->dataVersion = DATA_VERSION;
    return s;
}

