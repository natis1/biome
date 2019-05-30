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

#include <curses.h>
#include "display.h"
#include <signal.h>
#include <random>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>


display::display()
{
    std::string biomefolder = std::getenv("HOME");
    biomefolder += "/.config/biome/";
    std::cout << "Saving data to " << biomefolder << std::endl;
    mkdir(biomefolder.c_str(), 0755);
    this->savePath = biomefolder;
    initscr();
    start_color();
    std::cerr << "has colors? " << has_colors() << " can change colors?" << can_change_color() << " num colors? " << COLORS << std::endl;
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_color(65, 750, 750, 750); // Light grey
    init_pair(4, 65, COLOR_BLACK);
    init_color(66, 1000, 1000, 400); // Bright yellow
    init_pair(5, 66, COLOR_BLACK);
    init_color(67, 1000, 600, 300); // Orange
    init_pair(6, 67, COLOR_BLACK);
    init_pair(7, COLOR_GREEN, COLOR_BLACK);
    init_color(68, 200, 1000, 1000); // Cyan
    init_pair(8, 68, COLOR_BLACK);



    getBiomeType();
    getForestName();


    if (std::ifstream(biomefolder + "forest")) {
        sfile = fromJson<forest::saveFile>(pseudojson::fileToPseudoJson(biomefolder + "forest"));
        std::cout << "loaded existing file" << std::endl;
    } else {


        forest::newForest(&sfile, 3);
        pseudojson::writeToFile(toJson(sfile), biomefolder + "forest");
        std::cout << "created new file" << std::endl;
    }

    std::cout << sfile.biomeType << " is biome type " << sfile.name << " is name" << std::endl;


    signal(SIGABRT, display::sigabrtHandler);
    signal(SIGKILL, display::sigabrtHandler);
    signal(SIGSEGV, display::sigabrtHandler);
    signal(SIGTSTP, display::sigabrtHandler);


    // 1 Normal
    // 2 Good (Based on protium crystal color. See Half-Rose's drawings)
    // 3 Alright/neutral
    // 4 Warning
    // 5 Danger
    // 6 DANGER!


    // 7 Radiant
    // 8 User Input
    init_pair(7, COLOR_BLACK, COLOR_YELLOW);
    init_pair(8, COLOR_GREEN, COLOR_BLACK);

    move(0, 0);

    nodelay(stdscr, true);
}

int getDir (std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

void display::mainMenu()
{
    std::vector<std::string> files = std::vector<std::string>();
    getDir(this->savePath, files);


}


void display::biomeTypeHelper(int biome)
{
    // Biome starts at 0 and goes up from there
    int x;
    std::string line;
    attron(COLOR_PAIR(1U));
    for (int i = 0; i < NUM_BIOMES; i++) {
        std::istringstream t(BIOME_DESCRIPTIONS[i]);
        (i % 2 == 1) ? x = 20 : x = 0;
        int j = 0;
        while(std::getline(t, line)) {
            move(6* (i / 2) + j, x);
            j++;
            printw("%s", line.c_str());
        }
    }


    (biome % 2 == 1) ? x = 20 : x = 0;

    attron(COLOR_PAIR(static_cast<unsigned int>(biome) + 3U));
    int i = 0;
    std::istringstream t(BIOME_DESCRIPTIONS[biome]);
    while(std::getline(t, line)) {
        move(6* (biome / 2) + i, x);
        i++;
        printw("%s", line.c_str());
    }
}

int display::getBiomeType()
{
    int biome = 0;
    biomeTypeHelper(biome);

    while (true) {
        int c = getch();

        if (c == ERR) {
            break;
        } if (c == KEY_RESIZE) {
            clear();
            biomeTypeHelper(biome);
        } else if ( (c == KEY_ENTER || c == 10)) {
            return biome;
        } else if (c == KEY_STAB || c == 9) {
            biome++;
            if (biome >= NUM_BIOMES) {
                biome = 0;
            }
            biomeTypeHelper(biome);
        }
    }
}



void display::forestNameHelper(bool state)
{
    attron(COLOR_PAIR(1U));
    move(0, 0);
    printw("Please name your biome");
    move(1, 0);
    clrtoeol();
    move(1, 0);
    if (state) {
        attron(COLOR_PAIR(1U));
        printw("Randomize");
        move(1, 20);
        printw("|");
        attron(COLOR_PAIR(2U));
        printw("                    ");
        move(1, 21);
    } else {
        attron(COLOR_PAIR(2U));
        printw("Randomize           ");
        attron(COLOR_PAIR(1U));
        printw("|");
    }
}


std::string display::getForestName()
{
    std::mt19937 rng(time(0));
    std::uniform_int_distribution<int> numWordsRand(1, 100);
    std::uniform_int_distribution<int> firstWordRand(0, STARTING_RANDOM_NAMES_SIZE - 1);
    std::uniform_int_distribution<int> suffixWordRand(0, RANDOM_NAMES_SIZE - 1);
    //std::cerr << "Starting random size is " << STARTING_RANDOM_NAMES_SIZE << " and random size is " << RANDOM_NAMES_SIZE << std::endl;

    bool currentState = true;
    std::string currentName = "";
    forestNameHelper(currentState);
    while (true) {
        int c = getch();

        if (currentState) {
            if (c == ERR) {
                break;
            } if (c == KEY_RESIZE) {
                clear();
                forestNameHelper(currentState);
            } else if ( (c == KEY_ENTER || c == 10) && currentName.size() > 0) {
                return currentName;
            } else if (c == 8 || c == KEY_BACKSPACE) {
                if (!currentName.empty()) {
                    currentName = currentName.substr(0, currentName.size() - 1);
                }
            } else if (c == KEY_STAB || c == 9) {
                currentState = !currentState;
            } else {
                currentName = currentName + static_cast<char>(c);
            }

            forestNameHelper(currentState);
            move(1, 21);
            printw("%s", currentName.c_str());
        } else {
            if (c == KEY_ENTER || c == 10 || c == 'n') {
                int wordNum = numWordsRand(rng);
                if (wordNum <= 10) {
                    currentName = RANDOM_NAMES[suffixWordRand(rng)];
                } else if (wordNum <= 20) {
                    currentName = STARTING_RANDOM_NAMES[firstWordRand(rng)];
                    std::string tstr = RANDOM_NAMES[suffixWordRand(rng)];
                    tstr[0] = tolower(tstr[0]);
                    currentName += tstr;
                } else if (wordNum <= 30) {
                    currentName = STARTING_RANDOM_NAMES[firstWordRand(rng)];
                    currentName += " ";
                    currentName += RANDOM_NAMES[suffixWordRand(rng)];
                } else if (wordNum <= 40) {
                    currentName = RANDOM_NAMES[suffixWordRand(rng)];
                    std::string tstr = RANDOM_NAMES[suffixWordRand(rng)];
                    tstr[0] = tolower(tstr[0]);
                    currentName += tstr;
                } else if (wordNum <= 60) {
                    currentName = RANDOM_NAMES[suffixWordRand(rng)];
                    currentName += " ";
                    currentName += RANDOM_NAMES[suffixWordRand(rng)];
                } else if (wordNum <= 70) {
                    currentName = STARTING_RANDOM_NAMES[firstWordRand(rng)];
                    currentName += RANDOM_NAMES[suffixWordRand(rng)];
                    currentName += " ";
                    currentName += RANDOM_NAMES[suffixWordRand(rng)];
                } else if (wordNum <= 80) {
                    currentName = STARTING_RANDOM_NAMES[firstWordRand(rng)];
                    currentName += " ";
                    currentName += RANDOM_NAMES[suffixWordRand(rng)];
                    std::string tstr = RANDOM_NAMES[suffixWordRand(rng)];
                    tstr[0] = tolower(tstr[0]);
                    currentName += tstr;
                } else if (wordNum <= 90) {
                    currentName = RANDOM_NAMES[suffixWordRand(rng)];
                    currentName += " ";
                    currentName += RANDOM_NAMES[suffixWordRand(rng)];
                    std::string tstr = RANDOM_NAMES[suffixWordRand(rng)];
                    tstr[0] = tolower(tstr[0]);
                    currentName += tstr;
                } else {
                    currentName = RANDOM_NAMES[suffixWordRand(rng)];
                    std::string tstr = RANDOM_NAMES[suffixWordRand(rng)];
                    tstr[0] = tolower(tstr[0]);
                    currentName += tstr;
                    currentName += " ";
                    currentName += RANDOM_NAMES[suffixWordRand(rng)];
                }
            } else if (c == KEY_STAB || c == 9) {
                currentState = !currentState;
                forestNameHelper(currentState);
            }
            forestNameHelper(currentState);
            move(1, 21);
            printw("%s", currentName.c_str());
        }
    }

    return currentName;
}



void display::sigabrtHandler(int sig)
{
    if (sig == SIGKILL || sig == SIGSEGV || sig == SIGABRT) {
        endwin();
        return;
    } else if (sig == SIGTSTP) {
        std::cout << "Biome paused. Resume with the \"fg\" command." << std::endl;
        endwin();
        return;
    }

}


