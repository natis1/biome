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

#pragma once

#include "tools.h"


/**
 * @todo write docs
 */
class ctimer
{
public:
    struct configData {
        long text_foreground_r;
        long text_foreground_g;
        long text_foreground_b;
        long text_background_r;
        long text_background_g;
        long text_background_b;
        long bar_foreground_r;
        long bar_foreground_g;
        long bar_foreground_b;
        long bar_background_r;
        long bar_background_g;
        long bar_background_b;
        long done_foreground_r;
        long done_foreground_g;
        long done_foreground_b;
        long done_background_r;
        long done_background_g;
        long done_background_b;

        bool operator==(const configData& rhs) const {
            return std::tie(text_foreground_r, text_foreground_g, text_foreground_b, text_background_r, text_background_g, text_background_b, bar_foreground_r, bar_foreground_g, bar_foreground_b, bar_background_r, bar_background_g, bar_background_b, done_foreground_r, done_foreground_g, done_foreground_b, done_background_r, done_background_g, done_background_b) == std::tie(rhs.text_foreground_r, rhs.text_foreground_g, rhs.text_foreground_b, rhs.text_background_r, rhs.text_background_g, rhs.text_background_b, rhs.bar_foreground_r, rhs.bar_foreground_g, rhs.bar_foreground_b, rhs.bar_background_r, rhs.bar_background_g, rhs.bar_background_b, rhs.done_foreground_r, rhs.done_foreground_g, rhs.done_foreground_b, rhs.done_background_r, rhs.done_background_g, rhs.done_background_b);
        }

        constexpr static auto properties = std::make_tuple(
            property(&configData::text_foreground_r, "text_foreground_r"),
            property(&configData::text_foreground_g, "text_foreground_g"),
            property(&configData::text_foreground_b, "text_foreground_b"),
            property(&configData::text_background_r, "text_background_r"),
            property(&configData::text_background_g, "text_background_g"),
            property(&configData::text_background_b, "text_background_b"),
            property(&configData::bar_foreground_r, "bar_foreground_r"),
            property(&configData::bar_foreground_g, "bar_foreground_g"),
            property(&configData::bar_foreground_b, "bar_foreground_b"),
            property(&configData::bar_background_r, "bar_background_r"),
            property(&configData::bar_background_g, "bar_background_g"),
            property(&configData::bar_background_b, "bar_background_b"),
            property(&configData::done_foreground_r, "done_foreground_r"),
            property(&configData::done_foreground_g, "done_foreground_g"),
            property(&configData::done_foreground_b, "done_foreground_b"),
            property(&configData::done_background_r, "done_background_r"),
            property(&configData::done_background_g, "done_background_g"),
            property(&configData::done_background_b, "done_background_b"));
    } config;


    int colorMode;
    int colors[18];
    bool startDisplay(std::string timerName, double timeTotal);
    void initConfigData();
    void initColors();
    double getTimerPercentage();
private:
    double timeTotal;
    double timePassed;
    std::string timerName;
    bool timerLoop();
    void endTimer();
    void rectangle(int y1, int x1, int y2, int x2);
    std::string formatTime(double time);
};

