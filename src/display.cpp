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
#include <string.h>
#include <assert.h>
#include <math.h>
#include <algorithm>
#include <unistd.h>
#include <thread>

using namespace display_consts;

// yuck
static volatile sig_atomic_t last_signal = 0;
static volatile bool handing_signals = false;

display::display()
{
    std::string biomefolder = std::getenv("HOME");
    biomefolder += SAVE_LOCATION;
    std::cout << "Saving data to " << biomefolder << std::endl;
    mkdir(biomefolder.c_str(), 0755);
    savePath = biomefolder;
    initscr();
    noecho();
    start_color();
    std::cerr << "has colors? " << has_colors() << " can change colors? " << can_change_color() << " num colors? " << COLORS << std::endl;
    if (!has_colors() || !can_change_color() || COLORS < 256) {
        move(0, 0);
        printw("Your terminal seems to lack true color support.\nThis might be because it's old, or because the TERM environment variable isn't properly set to:\nTERM=xterm-256color\nYou can still try to play the game, but the experience might be worse.\nIt's recommended you use a terminal with full color compatibility like xfce4-terminal or even xterm.\nPress any key to continue or ctrl+C to quit");
        getch();
        move(0, 0);
        clrtobot();
    }
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_color(66, 750, 750, 750); // Light grey
    init_pair(4, 66, COLOR_BLACK);
    init_color(67, 1000, 1000, 400); // Bright yellow
    init_pair(5, 67, COLOR_BLACK);
    init_color(68, 1000, 600, 300); // Orange
    init_pair(6, 68, COLOR_BLACK);
    init_pair(7, COLOR_GREEN, COLOR_BLACK);
    init_color(69, 200, 1000, 1000); // Cyan
    init_pair(8, 69, COLOR_BLACK);
    init_pair(9, COLOR_BLACK, COLOR_RED);

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = &sigabrtHandler;
    sa.sa_flags = 0;
    sigemptyset(&(sa.sa_mask));
    sigaddset(&(sa.sa_mask), SIGINT);
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTSTP, &sa, NULL) == -1 || sigaction(SIGABRT, &sa, NULL)) {
        perror("sigaction error");
        return;
    }
    std::thread t1(display::signalReset);

    // True means the game was loaded and you are free to go on
    bool mm = mainMenu();
    while (!mm) {
        mm = mainMenu();
    }
    drawForest();
    endwin();
/*
    if (std::ifstream(biomefolder + "forest")) {
        sfile = fromJson<forest::saveFile>(pseudojson::fileToPseudoJson(biomefolder + "forest"));
        std::cout << "loaded existing file" << std::endl;
    } else {

        getBiomeType();
        getForestName();
        forest::newForest(&sfile, 3);
        pseudojson::writeToFile(toJson(sfile), biomefolder + "forest");
        std::cout << "created new file" << std::endl;
    }

    nodelay(stdscr, true); */
}

long display::getZoomLevel()
{
    // Apparently log of 0 is undefined.
    // Someone should ask the president of math to change that.
    if (sfile.trees <= 0) {
        return 0;
    }
    double logTrees = std::log10((double) sfile.trees);
    if (logTrees < 3.0) {
        return 0;
    }
    // Only start zooming in once the number of trees is over 1000. 1000 trees will fill about half the environment.
    return ( (long) logTrees - 2);
}

long display::getNumInhabitedTiles()
{
    long zl = getZoomLevel();
    long divisor = std::pow(10, zl);
    return (sfile.trees / divisor);
}


std::vector<bool> display::getPloppedTiles(long numPlopped, long int seed, int x, int y, double stdevPerX, double stdevPerY)
{
    std::vector<std::pair<double, int>> array;
    array.reserve(x * y);
    double midX = x / 2.0 - 0.5;
    double midY = y / 2.0 - 0.5;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> rn(0.0, 65536.0);

    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            double standardX = (i - midX) * stdevPerX;
            double standardY = (j - midY) * stdevPerY;
            if (standardX < 0 )
                standardX = - standardX;
            if (standardY < 0)
                standardY = - standardY;
            array.push_back(std::make_pair(((double) std::pow(M_E, (- (std::pow(standardX, 2.0) + std::pow(standardY, 2.0)))) * rn(rng)), (i * y + j)));
        }
    }
    std::sort(array.rbegin(), array.rend());

    std::vector<int> tileOrder;
    std::vector<bool> areTilesPlopped;
    tileOrder.reserve(x * y);
    areTilesPlopped.reserve(x * y);
    for (int i = 0; i < (x * y); i++) {
        tileOrder.push_back(array[i].second);
        areTilesPlopped.push_back(false);
    }

    // Invert vector code.
    for (int i = 0; i < numPlopped; i++) {
        areTilesPlopped.at(tileOrder.at(i)) = true;
    }
    return areTilesPlopped;
}

std::vector<int> display::forestOpacity()
{
    std::vector<int> newColors;

    for (int i = 0; i < 6; i++) {
        int c1 = forest::biomes[sfile.biomeType].inhabitedColorPair[i];
        int c2 = forest::biomes[sfile.biomeType].uninhabitedColorPair[i];
        // Why? Because how our eyes see color. With 1.0 it transitions too slowly at low healths.
        double fh = std::pow(forestHealth / (100.0), 0.8);

        int weightedAvg = (c1 * fh ) + (c2 * (1 - fh));
        newColors.push_back(weightedAvg);
    }

    return newColors;
}


std::vector<int> display::adjacentTileOpacity()
{
    std::vector<int> newColors;

    for (int i = 0; i < 6; i++) {
        int c1 = forest::biomes[sfile.biomeType].inhabitedColorPair[i];
        int c2 = forest::biomes[sfile.biomeType].uninhabitedColorPair[i];
        // Why? Because how our eyes see color. With 1.0 it transitions too slowly at low healths.
        double fh = std::pow(forestHealth / (100.0), 0.8);

        std::vector<int> a;
        a.push_back((c1 * (fh - 0.1 ) ) + (c2 * (1.1 - fh)));
        a.push_back((c1 * (fh - 0.2 ) ) + (c2 * (1.2 - fh)));
        a.push_back((c1 * (fh - 0.3 ) ) + (c2 * (1.3 - fh)));
        a.push_back((c1 * (fh - 0.4 ) ) + (c2 * (1.4 - fh)));
        a.push_back((c1 * (fh - 0.5 ) ) + (c2 * (1.5 - fh)));
        a.push_back((c1 * (fh - 0.6 ) ) + (c2 * (1.6 - fh)));
        a.push_back((c1 * (fh - 0.7 ) ) + (c2 * (1.7 - fh)));
        a.push_back((c1 * (fh - 0.8 ) ) + (c2 * (1.8 - fh)));
        a.push_back((c1 * (fh - 0.9 ) ) + (c2 * (1.9 - fh)));

        for (int j = 0; j < a.size(); j++) {
            if (0.1 * (j + 1) > fh) {
                a[j] = c2;
            }
            newColors.push_back(a[j]);
        }
    }

    return newColors;

}

std::string display::getScaleString(long scale)
{
    if (scale < 2) {
        return (scale == 0) ?  "1" : "2.5";
    } else {
        std::string sca;
        if (scale % 2 == 1) {
            sca = "25";
            for (int i = 3; i < scale; i = i + 2) {
                sca += "0";
            }
        } else {
            sca = "10";
            for (int i = 3; i < scale; i = i + 2) {
                sca += "0";
            }
        }
        if (sca.size() >= 8) {
            sca = sca.substr(0, sca.size() - 6);
            sca += "M"; // wow
        } else if (sca.size() >= 5) {
            sca = sca.substr(0, sca.size() - 3);
            sca += "k";
        }
        return sca;
    }
}



void display::drawStatsScreen()
{
    // I dunno the actual one. This is my best layman's guess.
    const long healthyBugTreeRatio = 5000;
    long actualInsectCount = (long) (healthyBugTreeRatio * (forestHealth / 100.0) * forest::biomes[sfile.biomeType].insectQuantityModifier);
    long uniqueInsectSpecies = std::pow(sfile.trees, 0.02);
    move(1, 0);
    printw("Biome type: \n");
    printw("Unique insect species ");

}



void display::drawForest()
{
    forestHealth = 0;
    nodelay(stdscr, FALSE);
    //std::cerr << "Forest seed is " << sfile.biomeSeed << std::endl;
    //getch();
    while (true) {
        std::mt19937 rng(sfile.biomeSeed);
        std::uniform_int_distribution<int> colorRn(0, 5);
        std::uniform_int_distribution<int> inhabitedRn(0, forest::biomes[sfile.biomeType].inhabitedSymbols.size() - 1);
        std::uniform_int_distribution<int> uninhabitedRn(0, forest::biomes[sfile.biomeType].inhabitedSymbols.size() - 1);
        long tiles = getNumInhabitedTiles();
        long zoomLevel = getZoomLevel();
        move(0, 0);
        clrtobot();
        move(0, 0);
        const int x = 22;
        const int y = 80;
        char forestGrid[x][y];

        std::vector<int> biomeColors = forestOpacity();

        init_color(80, biomeColors[0], biomeColors[1], biomeColors[2]);
        init_color(81, biomeColors[3], biomeColors[4], biomeColors[5]);
        init_pair(14, 80, 81);
        init_color(82, forest::biomes[sfile.biomeType].uninhabitedColorPair[0], forest::biomes[sfile.biomeType].uninhabitedColorPair[1], forest::biomes[sfile.biomeType].uninhabitedColorPair[2]);
        init_color(83, forest::biomes[sfile.biomeType].uninhabitedColorPair[3], forest::biomes[sfile.biomeType].uninhabitedColorPair[4], forest::biomes[sfile.biomeType].uninhabitedColorPair[5]);
        init_pair(15, 82, 83);

        std::vector<int> adjColors = adjacentTileOpacity();
        for (int i = 0; i < 9; i++) {
            init_color(90 + 2 * i, adjColors[0 + i], adjColors[9 + i], adjColors[18 + i]);
            init_color(91 + 2 * i, adjColors[27 + i], adjColors[36 + i], adjColors[45 + i]);
            init_pair(16 + i, 90 + 2 * i, 91 + 2 * i);
        }

        std::vector<bool> plTiles = getPloppedTiles(tiles, sfile.biomeSeed + zoomLevel, x, y, 4.0 / x, 4.0 / y);
        for (int i = 0; i < x; i++) {
            for (int j = 0; j < y; j++) {
                if (plTiles.at(i * y + j)) {
                    forestGrid[i][j] = forest::biomes[sfile.biomeType].inhabitedSymbols[inhabitedRn(rng)];
                } else {
                    forestGrid[i][j] = forest::biomes[sfile.biomeType].uninhabitedSymbols[uninhabitedRn(rng)];
                }
            }
        }

        for (int i = 0; (i < x) && (i < LINES - 2); i++) {
            for (int j = 0; (j < y) && (j < COLS); j++) {
                move(i, j);
                if (plTiles.at(i * y + j))
                    attron(COLOR_PAIR(14U));
                else {
                    const int boundarySize = 8;
                    int leftExtents = (j < boundarySize) ? j : boundarySize;
                    int rightExtents = (j > (y - (1 + boundarySize))) ? (y - j) : (1 + boundarySize);
                    int topExtents = (i < boundarySize) ? i : boundarySize;
                    int botExtents = (i > (x - (1 + boundarySize))) ? (x - i) : (1 + boundarySize);
                    int totalTiles = 0;
                    int treeTiles = 0;

                    for (int k = -topExtents; k < botExtents; k++) {
                        for (int l = - leftExtents; l < rightExtents; l++) {
                            totalTiles++;
                            if (plTiles.at( (i + k) * y + (j + l))) {
                                treeTiles++;
                            }
                        }
                    }
                    double forestCover = ((double) treeTiles) / totalTiles;
                    forestCover = std::pow(forestCover, 0.3);
                    forestCover = forestCover * 8.999;
                    attron(COLOR_PAIR(24U - (std::floor(forestCover))));
                }
                addch(forestGrid[i][j]);
            }
        }
        int xpos, ypos;
        getyx(stdscr, ypos, xpos);
        if (xpos != 0)
            addch('\n');

        if (forestHealth < 100) {
            attron(COLOR_PAIR(1U));
        } else {
            attron(COLOR_PAIR(7U));
        }
        printw("Health [");
        int j = forestHealth;
        for (int i = 0; i < 50; i++) {
            if (j > 0) {
                addch('#');
                j = j - 2;
            } else {
                addch(' ');
            }
        }
        addch(']');
        attron(COLOR_PAIR(1U));
        printw( (" 1 tile = " + getScaleString(zoomLevel) + "m").c_str());

        addch('\n');
        attron(COLOR_PAIR(1U));
        printw("s - stats  |  g - grow trees  |  q - quit");

        char c = getch();
        if (c == 's' || c == 'S') {
            drawStatsScreen();
        } else if (c == 'g' || c == 'G') {

        } else if (c == 'q' || c == 'Q') {
            return;
        }
    }

}




int display::getDir(std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << "Error(" << errno << ") opening " << dir << std::endl;
        return errno;
    }
    int err = 0;

    while ((dirp = readdir(dp)) != NULL) {
        std::string s = dirp->d_name;
        if ( strncmp(dirp->d_name, ".", 1) == 0 || s.find(".conf") != s.npos)
            continue;
        if (s.length() > 6 && (s.compare(s.size() - 6, 6, ".biome") == 0)) {
            files.push_back(std::string(dirp->d_name));
        } else {
            err = 420;
            printw(("Possible biome file \"" + s + "\" does not end in .biome. Please rename it if it's a valid file.\n").c_str());
        }
    }
    closedir(dp);
    return err;
}

// most recent file
std::string display::mostRecentFile(std::vector<std::string> *files)
{
    long bestTime = 0;
    std::string name;

    for (unsigned int i = 0; i < files->size(); i++) {
        std::string fPath = savePath + files->at(i);
        char *filePath = &fPath[0u];
        struct stat attrib;
        stat(filePath, &attrib);
        long time = static_cast<long int>((attrib.st_ctime));
        if (time > bestTime) {
            name = files->at(i);
            bestTime = time;
        }
    }
    return name;
}

bool display::mainMenu()
{
    std::vector<std::string> files = std::vector<std::string>();
    move(0, 0);
    if (getDir(this->savePath, files) != 0) {
        printw("Press any key to continue, or ^C to exit");
        std::cerr << "Press any key to continue, or ^C to exit" << std::endl;
        int i = 0;
        do {
            i = getch();
        } while (i == -1);
        //std::cerr << "Pressed a button " << i;
        move(0, 0);
        clrtobot();
    }
    bool newGame = (files.size() < 9);
    bool continueGame = (files.size() > 0);
    std::string recentFile;
    if (continueGame) {
        recentFile = mostRecentFile(&files);
        std::cerr << "most recent file is: " << recentFile << std::endl;
    }

    move(1, 0);
    if (newGame) {
        printw("(N)ew biome\n");
    }
    if (continueGame) {
        printw("(C)ontinue biome: ");
        printw(recentFile.substr(0, recentFile.length() - 6).c_str());
        printw("\n");
    }
    printw("Or load a biome below with a number key:\n\n");
    // Bleh, I have to do the awful 1 indexed for loop
    for (int i = 1; i <= files.size(); i++) {
        printw("(%d) %s\n", i, files.at( (i - 1) ).substr(0, files.at( (i - 1)).length() - 6).c_str());
    }

    init_color(65, 0, 0, 0);
    init_pair(65, 65, COLOR_BLACK);
    attron(COLOR_PAIR(65U));
    printw("\nCan you see this text?\nYour terminal emulator lacks custom colors, but is pretending to have them.\nThese are known to be broken on qterminal and konsole.\nFor other terminals, check your emulator settings.");
    while (true) {
        move(LINES - 1, COLS - 1);
        int c = getch();

        if (c == ERR) {
            // ignore, but redraw menu
            return false;
        } else if (newGame && (c == 'n' || c == 'N')) {
            // newgame...
            move(0, 0);
            clrtobot();
            int biomeType = getBiomeType();
            move(0, 0);
            clrtobot();
            std::string biomeName = getForestName();
            forest::newForest(&sfile, biomeType, biomeName);
            pseudojson::writeToFile(toJson(sfile), this->savePath + biomeName + ".biome");
            std::cout << "created new file" << std::endl;

            return true;
        } else if (continueGame && (c == 'c' || c == 'C')) {
            // Continue listed game
            sfile = fromJsonForest<forest::saveFile>(pseudojson::fileToPseudoJson(this->savePath + recentFile));
            if (sfile.dataVersion < forest::DATA_VERSION) {
                sfile = *forest::updateForest(&sfile);
                std::cout << "Updated forest to version: " << sfile.dataVersion << std::endl;
            }
            pseudojson::writeToFile(toJson(sfile), this->savePath + sfile.name + ".biome");

            return true;
        } else if (c >= '1' && c <= '9') {
            int realInt = c - '0';
            if (realInt <= files.size()) {
                realInt--;
                sfile = fromJsonForest<forest::saveFile>(pseudojson::fileToPseudoJson(this->savePath + files[realInt]));
                if (sfile.dataVersion < forest::DATA_VERSION) {
                    sfile = *forest::updateForest(&sfile);
                    std::cout << "Updated forest to version: " << sfile.dataVersion << std::endl;
                }
                pseudojson::writeToFile(toJson(sfile), this->savePath + sfile.name + ".biome");

                return true;
            }
        }
    }
    return false;
}


void display::biomeTypeHelper(int biome)
{
    // Biome starts at 0 and goes up from there
    int x;
    std::string line;
    attron(COLOR_PAIR(1U));
    for (unsigned int i = 0; i < NUM_BIOMES; i++) {
        std::istringstream t(BIOME_DESCRIPTIONS[i]);
        (i % 2 == 1) ? x = 20 : x = 0;
        int j = 0;
        while(std::getline(t, line)) {
            move(6* (i / 2) + j, x);
            j++;
            printw(line.c_str());
        }
    }


    (biome % 2 == 1) ? x = 20 : x = 0;

    attron(COLOR_PAIR(static_cast<unsigned int>(biome) + 3U));
    int i = 0;
    std::istringstream t(BIOME_DESCRIPTIONS[biome]);
    while(std::getline(t, line)) {
        move(6* (biome / 2) + i, x);
        i++;
        printw(line.c_str());
    }
}

int display::getBiomeType()
{
    unsigned int biome = 0;
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


    assert(false);
    return 0;
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
            } else if (c == 8 || c == KEY_BACKSPACE || c == 127) {
                if (!currentName.empty()) {
                    currentName = currentName.substr(0, currentName.size() - 1);
                }
            } else if (c == KEY_STAB || c == 9) {
                currentState = !currentState;
            } else if (c == '/' || c == '\\' || c == '.') {
                // Ignore input to sanitize for the system.
            } else {
                currentName = currentName + static_cast<char>(c);
            }

            forestNameHelper(currentState);
            move(1, 21);
            clrtoeol();
            move(1, 21);
            printw(currentName.c_str());
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
            printw(currentName.c_str());
        }
    }

    return currentName;
}

void display::signalReset()
{
    while (true) {
        std::cerr << "Resetting signal" << std::endl;
        last_signal = 0;
        sleep(2);
    }
}


void display::sigabrtHandler(int sig)
{
    if (sig == SIGINT && last_signal != SIGINT) {
        last_signal = sig;
        move (0, 0);
        clrtobot();
        attron(COLOR_PAIR(9U));
        printw("Are you sure you want to exit Biome?\nAll currently growing trees will be lost.\nPress ctrl+c twice rapidly to confirm.");
        getch();
        attron(COLOR_PAIR(1U));
    } else if (sig == SIGINT && last_signal == SIGINT) {
        std::cerr << "Exiting Biome because ctrl c pressed twice" << std::endl;
        endwin();
        exit(sig);
    }
    if (sig == SIGKILL || sig == SIGSEGV || sig == SIGABRT) {
        endwin();
        // Save data here.
        exit(sig);
    } else if (sig == SIGTSTP) {
        sigabrtHandler(SIGINT);
    }
}


