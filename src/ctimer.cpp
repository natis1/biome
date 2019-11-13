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

#include "ctimer.h"
#include "display.h"
#include <assert.h>
#include <curses.h>
#include <cmath>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <sstream>

ctimer::ctimer(int color, double timeTotal)
{
    this->colorMode = color;
    initConfigData();
    initColors();
    this->timeTotal = timeTotal;
}



void ctimer::initConfigData()
{
    std::string saveFolder = std::getenv("HOME");
    saveFolder += display_consts::SAVE_LOCATION;
    std::vector<std::string> saveFile;
    if (display::getDir(true, saveFolder, saveFile) != 0) {
        mkdir(saveFolder.c_str(), 0755);
    }
    //std::cout << "Save file size is " << saveFile.size() << std::endl << "And the timer length is " << timeTotal;

    bool hasFile = (saveFile.size() >= 1);
    if (hasFile) {
        hasFile = false;
        for (unsigned int i = 0; i < saveFile.size(); i++) {
            std::cerr << saveFile.at(i) << std::endl;
            if (saveFile.at(i) == "timer.conf") {
                hasFile = true;
                break;
            }
        }

    }
    if (!hasFile) {
        config = {1000, 1000, 0, 0, 0, 0, 200, 800, 200, 400, 0, 0, 1000, 1000, 1000, 0, 0, 0};
        pseudojson::Value configFile = toJson(config);
        pseudojson::writeToFile(configFile, saveFolder + "timer.conf");
    }

    config = fromJson<ctimer::configData>(pseudojson::fileToPseudoJson(saveFolder + "timer.conf"));

    colors[0] = config.text_foreground_r;
    colors[1] = config.text_foreground_g;
    colors[2] = config.text_foreground_b;
    colors[3] = config.text_background_r;
    colors[4] = config.text_background_g;
    colors[5] = config.text_background_b;
    colors[6] = config.bar_foreground_r;
    colors[7] = config.bar_foreground_g;
    colors[8] = config.bar_foreground_b;
    colors[9] = config.bar_background_r;
    colors[10] = config.bar_background_g;
    colors[11] = config.bar_background_b;
    colors[12] = config.done_foreground_r;
    colors[13] = config.done_foreground_g;
    colors[14] = config.done_foreground_b;
    colors[15] = config.done_background_r;
    colors[16] = config.done_background_g;
    colors[17] = config.done_background_b;

}


void ctimer::initColors()
{
    if (colorMode == 24) {
        // The colors of the text
        init_color(100, colors[0], colors[1], colors[2]);
        init_color(101, colors[3], colors[4], colors[5]);

        // Colors of the partially filled bar
        init_color(102, colors[6], colors[7], colors[8]);
        init_color(103, colors[9], colors[10], colors[11]);

        // Colors of the full bar
        init_color(104, colors[12], colors[13], colors[14]);
        init_color(105, colors[15], colors[16], colors[17]);

        init_pair(40, 100, 101);
        init_pair(41, 102, 103);

        // Color pair 4 and 5 is inverted because ncurses chars lack the full bar character. so instead we use spaces but anti-colored.
        init_pair(42, 104, 105);
        init_pair(43, 103, 102);
    } else {
        // No custom colors for you sorry.
        display::loadEightBitColor(40, forest::YELLOW, forest::BLACK, colorMode);
        display::loadEightBitColor(41, forest::GREEN, forest::RED, colorMode);
        display::loadEightBitColor(42, forest::WHITE, forest::BLACK, colorMode);
        display::loadEightBitColor(43, forest::RED, forest::GREEN, colorMode);
    }
}

bool ctimer::startDisplay(const std::string timerName)
{
    this->timerName = timerName;
    curs_set(0);
    return timerLoop();
}



void ctimer::rectangle(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2-x1);
    mvhline(y2, x1, 0, x2-x1);
    mvvline(y1, x1, 0, y2-y1);
    mvvline(y1, x2, 0, y2-y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

std::string ctimer::formatTime(double time)
{
    std::string tOut;
    int days = (int) (time / (MINUTE_LENGTH * HOUR_LENGTH * DAY_LENGTH));
    time = std::fmod(time, (MINUTE_LENGTH * HOUR_LENGTH * DAY_LENGTH));
    int hours = (int) (time / (MINUTE_LENGTH * HOUR_LENGTH));
    time = std::fmod(time, MINUTE_LENGTH * HOUR_LENGTH);
    int minutes = (int) (time / MINUTE_LENGTH);
    time = std::fmod(time, MINUTE_LENGTH);
    if (days > 0) {
        tOut += std::to_string(days) + ":";
    }
    if (hours > 9) {
        tOut += std::to_string(hours) + ":";
    } else {
        tOut += "0" + std::to_string(hours) + ":";
    }
    if (minutes > 9) {
        tOut += std::to_string(minutes) + ":";
    } else {
        tOut += "0" + std::to_string(minutes) + ":";
    }
    if (time < 10) {
        tOut += "0";
    }

    // Create an output string stream
    std::ostringstream streamObj3;
    streamObj3 << std::fixed;
    streamObj3 << std::setprecision(2);
    streamObj3 << time;
    tOut += streamObj3.str();

    return tOut;
}


bool ctimer::timerLoop()
{
    nodelay(stdscr, TRUE);
    auto start = std::chrono::system_clock::now();
    std::string endTime = formatTime(timeTotal);
    while (true) {
        int c = getch();
        // escape key
        if (c == 27 || c == 'q' || c == 'Q') {
            nodelay(stdscr, FALSE);
            attron(COLOR_PAIR(8U));
            move(0, 0);
            printw(("Are you sure you want to exit now?\n" + std::to_string(display::getTimerImpact((long) timeTotal)) + " trees will die!").c_str());
            printw("\n[y/N]");
            int j = getch();
            nodelay(stdscr, TRUE);
            if (j == 'y' || j == 'Y') {
                return false;
            }
        }

        auto curTime = std::chrono::system_clock::now();
        std::chrono::duration<double> t = curTime - start;
        if (t.count() > timeTotal) {
            nodelay(stdscr, FALSE);
            endTimer();
            return true;
        } else {
            double percentageBar = t.count() / timeTotal;
            int barSize = (int) (((COLS - 2) * percentageBar) + 0.5);

            attron(COLOR_PAIR(40U));
            rectangle(0, 0, 4, COLS - 1);
            int leftMid = (COLS / 2) - (timerName.length() / 2);
            if (leftMid > 0) {
                move(1, leftMid);
                printw(timerName.c_str());
            }
            attron(COLOR_PAIR(43U));
            std::string bar (barSize, ' ');
            std::string barEnd (COLS - 2 - barSize, ' ');
            move (2, 1);
            printw(bar.c_str());
            attron(COLOR_PAIR(41U));
            printw(barEnd.c_str());
            move (3, 1);
            attron(COLOR_PAIR(40U));
            printw(formatTime(t.count()).c_str());

            if (COLS - 1 - endTime.length() > 0) {
                move (3, COLS - 1 - endTime.length());
                printw(endTime.c_str());
            }

            refresh();
            // meme

        }
        usleep(100000);
        move(0, 0);
        clrtobot();
    }
    return false;
}

void ctimer::endTimer()
{
    std::string endTime = formatTime(timeTotal);
    while (true) {

        int barSize = (COLS - 2);

        attron(COLOR_PAIR(40U));
        rectangle(0, 0, 4, COLS - 1);
        int leftMid = (COLS / 2) - (timerName.length() / 2);
        if (leftMid > 0) {
            move(1, leftMid);
            printw(timerName.c_str());
        }
        attron(COLOR_PAIR(43U));
        std::string bar (barSize, ' ');
        move (2, 1);
        printw(bar.c_str());
        move (3, 1);
        attron(COLOR_PAIR(40U));
        printw(endTime.c_str());

        if (COLS - 1 - endTime.length() > 0) {
            move (3, COLS - 1 - endTime.length());
            printw(endTime.c_str());
        }

        getch();

        return;
    }
}


