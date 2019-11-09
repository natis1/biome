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
#include <assert.h>


namespace forest
{
    static const int DATA_VERSION = 5;
    static const int OPTIONS_VERSION = 1;

    enum colors {
        RED,
        GREEN,
        BLUE,
        YELLOW,
        CYAN,
        LIGHT_CYAN,
        ORANGE,
        WHITE,
        BLACK,
        DARK_GREEN,
        DARK_GREENISH,
        LIGHT_GREEN,
        VERY_GREEN,
        PURE_GREY,
        DARK_GREY,
        MUDDY_BLUE,
        BROWN,
        LIGHT_BROWN,
        DARK_BROWN,
        VERY_BROWN,
        DARK_BLUE,
        VERY_DARK_GREY,
        VERY_DARK_BLUE,
        EIGHT_COLOR_WHITE
    };

    struct biome {
        std::string name;
        std::string plantName;
        std::string insectNameSingular;
        std::string insectName;
        double insectQuantityModifier;
        short uninhabitedColorPair[6];
        short inhabitedColorPair[6];
        std::vector<enum colors> colorDescriptions;
        std::vector<int> uninhabitedSymbols;
        std::vector<int> inhabitedSymbols;
    };

    const biome biomes[] {
        // Uninhabited- Water which is dark grey on black. Inhabited- Moss which is very green on dark blue.
        // Uninhabited is water. ~ . - . Inhabited is moss: m, M
        {"Swamp", "Moss", "Woodlouse", "Woodlice", 2.0 , {250, 250, 250, 0, 0, 0}, {200, 850, 75, 0, 0, 400}, {DARK_GREY, BLACK, VERY_GREEN, DARK_BLUE}, {'~', '.', '-'}, {'m', 'M'}},
        // Uninhabited is gray rocks on dark brown background. Inhabited is deep green on dark very dark grey.
        // Uninhabited is mostly empty with occasional rocks. Inhabited is '^', 't', 'Y'
        {"Mountain", "Cycads", "Wasp", "Wasps", 1.0 , {400, 400, 400, 350, 350, 0}, {100, 800, 100, 100, 100, 100}, {PURE_GREY, DARK_BROWN, DARK_GREEN, VERY_DARK_GREY}, {' ', ' ', ' ', ' ', ' ', 'o'}, {'*', 't', 'Y'}},
        // Uninhabited is yellow sand: '.', '`', ',', '*', ''' on grey background (mostly to not hurt people's eyes).
        // Inhabited is light green cacti: 'Y', 'I', 'V', '|' on darker grey background.
        {"Desert", "Cacti", "Beetle", "Beetles", 1.5 , {850, 850, 0, 500, 500, 500}, {500, 1000, 500, 300, 300, 300}, {YELLOW, PURE_GREY, LIGHT_GREEN, DARK_GREY}, {'.', '`', ',', '*', '\''}, {'Y', 'I', 'V', '|'}},
        // Uninhabited is grey sand: '.', '`', ',', '*', ''' on dark grey background (mostly to not hurt people's eyes).
        // Inhabited is big very green trees: 'T', '|', 'I' on dark green background.
        {"Chaparral", "Sequoia", "Ant", "Ants", 8.0 , {650, 650, 650, 400, 400, 400}, {100, 1000, 0, 0, 300, 0}, {PURE_GREY, DARK_GREY, VERY_GREEN, DARK_GREENISH}, {'.', '`', ',', '*', '\''}, {'T', '|', 'I'}},
        // Uninhabited is very brown trees: 't', 'I', '|', '^', 'Y' on dark greenish background
        // Inhabited is orange orchids: 'S', 'p', 'R', 'Q', 'g', 'J', 'U', 'b' on dark green background
        {"Jungle", "Orchids", "Bee", "Bees", 2.0 , {400, 400, 0, 0, 100, 0}, {1000, 500, 0, 0, 400, 0}, {VERY_DARK_GREY, DARK_GREENISH, ORANGE, DARK_GREENISH}, {'t', 'I', '|', 'V', 'Y'}, {'S', 'p', 'R', 'Q', 'g', 'J', 'U', 'b'}},
        // Uninhabited is light blue/cyan snow: '.', '`', ',', '*', ''' on dark blue background (mostly to not hurt people's eyes).
        // Inhabited is vivid green trees: 'T', 't', 'I', 'Y', '^' on dark greenish background
        {"Taiga", "Conifers", "Moth", "Moths", 1.0 , {600, 850, 850, 100, 100, 200}, {100, 900, 0, 0, 100, 0}, {LIGHT_CYAN, VERY_DARK_BLUE, VERY_GREEN, DARK_GREENISH}, {'.', '`', ',', '*', '\''}, {'T', 't', 'I', 'Y', 'J'}}
    };


    struct saveFile {
        long dataVersion;
        long biomeType;
        long trees;
        long productiveSeconds;
        long biomeSeed;
        std::string name;


        long lastRunTime;
        long dailyStreak;
        std::vector<double> weeklyRuntimes;
        std::vector<double> weeklyRundifficulties;

        bool operator==(const saveFile& rhs) const {
            assert (weeklyRundifficulties.size() == 7 && weeklyRuntimes.size() == 7);

            return
            std::tie(dataVersion, biomeType, trees, productiveSeconds, biomeSeed, name, lastRunTime, dailyStreak, weeklyRuntimes[0], weeklyRuntimes[1], weeklyRuntimes[2], weeklyRuntimes[3], weeklyRuntimes[4], weeklyRuntimes[5], weeklyRuntimes[6], weeklyRundifficulties[0], weeklyRundifficulties[1], weeklyRundifficulties[2], weeklyRundifficulties[3], weeklyRundifficulties[4], weeklyRundifficulties[5], weeklyRundifficulties[6]) ==
            std::tie(rhs.dataVersion, rhs.biomeType, rhs.trees, rhs.productiveSeconds, rhs.biomeSeed, rhs.name, rhs.lastRunTime, rhs.dailyStreak, rhs.weeklyRuntimes[0], rhs.weeklyRuntimes[1], rhs.weeklyRuntimes[2], rhs.weeklyRuntimes[3], rhs.weeklyRuntimes[4], rhs.weeklyRuntimes[5], rhs.weeklyRuntimes[6], rhs.weeklyRundifficulties[0], rhs.weeklyRundifficulties[1], rhs.weeklyRundifficulties[2], rhs.weeklyRundifficulties[3], rhs.weeklyRundifficulties[4], rhs.weeklyRundifficulties[5], rhs.weeklyRundifficulties[6]);
        }

        constexpr static auto properties = std::make_tuple
           (
            property(&saveFile::dataVersion, "dataVersion"),
            property(&saveFile::biomeType, "biomeType"),
            property(&saveFile::trees, "trees"),
            property(&saveFile::productiveSeconds, "productiveSeconds"),
            property(&saveFile::biomeSeed, "biomeSeed"),
            property(&saveFile::name, "name"),
            property(&saveFile::lastRunTime, "lastRuntime"),
            property(&saveFile::dailyStreak, "dailyStreak"),
            property(&saveFile::weeklyRuntimes, "weeklyRuntimes"),
            property(&saveFile::weeklyRundifficulties, "weeklyRunDifficulties")
           );

        constexpr static auto properties_v3 = std::make_tuple
           (
            property(&saveFile::dataVersion, "dataVersion"),
            property(&saveFile::biomeType, "biomeType"),
            property(&saveFile::trees, "trees"),
            property(&saveFile::productiveSeconds, "productiveSeconds"),
            property(&saveFile::name, "name")
           );
        constexpr static auto properties_v1 = std::make_tuple
           (
            property(&saveFile::dataVersion, "dataVersion"),
            property(&saveFile::name, "name")
           );
    };

    struct optionsFile {
        long optionsVersion;
        bool blockWebsites;
        std::vector<std::string> sitesBlocked;
        bool blockInternet;
        double idealHoursPerWeek;

        constexpr static auto properties = std::make_tuple
        (
        property(&optionsFile::optionsVersion, "Version Number"),
         property(&optionsFile::blockWebsites, "Block Websites?"),
         property(&optionsFile::sitesBlocked, "Sites Blocked"),
         property(&optionsFile::blockInternet, "Block Internet?"),
         property(&optionsFile::idealHoursPerWeek, "Goal Hours Per Week")
        );


    };

    struct optionsFile* newOptions(optionsFile *o);
    struct saveFile* newForest(saveFile *s, long biome, std::string name);
    struct saveFile* updateForest(saveFile *s);
}

#endif // FOREST_H
