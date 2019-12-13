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

#include "tools.h"
#include <sstream>

using namespace pseudojson;

std::string pseudojson::getValue(const pseudojson::ValueData* v)
{
    std::string prefix;
    std::string affix;
    std::string val;
    if (v->lon != UNSET_INTVAL) {
        prefix = "l";
        val = std::to_string(v->lon);
    } else if (v->d != UNSET_DOUBLEVAL) {
        prefix = "d";
        val = std::to_string(v->d);
    } else if (v->string.length() > 0) {
        prefix = "\"";
        affix = "\"";
        val = v->string;
    } else if (!v->dptr.empty()) {
        prefix = "D";
        for (unsigned int i = 0; i < v->dptr.size(); i++) {
            if (i != 0) {
                val += ",";
            }
            std::ostringstream strs;
            strs << v->dptr.at(i);
            val += strs.str();
        }
    } else if (v->boolSetState != UNSET_INTVAL) {
        prefix = "B_";
        if (v->boolSetState == 1) {
            val = "TRUE";
        } else {
            val = "FALSE";
        }
    } else if (v->dataType == DATATYPE_SPTR) {
        prefix = "S_ARRAY ";
        for (unsigned int i = 0; i < v->sptr.size(); i++) {
            if (i != 0) {
                val += ",";
            }
            val += v->sptr.at(i);
        }
    }

    return prefix + val + affix;
}

std::vector<std::string> pseudojson::split(const std::string &text, char sep) {
    std::vector<std::string> tokens;
    std::size_t start = 0;
    std::size_t end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
    return tokens;
}

std::pair<std::string, pseudojson::Value> pseudojson::stringToValue(const std::string &valueLine)
{
    std::vector<std::string> x = pseudojson::split(valueLine, ':');
    x.at(0) = x.at(0).substr(0, x.at(0).size()-1);
    x.at(1).erase(0, 1);

    pseudojson::Value val;

    switch (x.at(1).at(0)) {
        case '"':
            x.at(1).erase(0, 1);
            x.at(1) = x.at(1).substr(0, x.at(1).size()-1);
            val.data.string = x.at(1);
            break;
        case 'l':
            x.at(1).erase(0, 1);
            val.data.lon = std::stol(x.at(1));
            break;
        case 'd':
            x.at(1).erase(0, 1);
            val.data.d = std::stod(x.at(1));
            break;
        case 'B':
            if (x.at(1).at(2) == 'T' || x.at(1).at(2) == 't') {
                val.data.boolSetState = 1;
            } else {
                val.data.boolSetState = 0;
            }
            break;
        case 'S': {
            val.data.dataType = DATATYPE_SPTR;
            if (x.at(1).size() <= 8) {
                break;
            }
            x.at(1).erase(0, 8);
            std::vector<std::string> a = split(x.at(1), ',');
            for (unsigned int i = 0; i < a.size(); i++)
                val.data.sptr.push_back(a.at(i));
            break;
        }
        case 'D': {
            x.at(1).erase(0, 1);
            std::vector<std::string> a = split(x.at(1), ',');
            for (unsigned int i = 0; i < a.size(); i++) {
                val.data.dptr.push_back(std::stod(a.at(i)));
            }
        }
    }
    if (x.at(1).at(0) == '"') {
        x.at(1).erase(0, 1);
        x.at(1) = x.at(1).substr(1, x.at(1).size()-1);
        val.data.string = x.at(1);
    }
    return std::make_pair(x.at(0), val);
}

pseudojson::Value pseudojson::fileToPseudoJson(std::string filename)
{
    std::ifstream in(filename);
    std::string str;

    pseudojson::Value dat;

    while (std::getline(in, str)) {
        if (str.size() > 3) {
            auto pair = stringToValue(str);
            dat.data.subObject.insert(pair);
        }
    }
    return dat;
}

bool pseudojson::writeToFile(pseudojson::Value val, std::string filename)
{
    try {
        std::ofstream os(filename);
        for( auto const& [key, val] : val.data.subObject ) {
            os << key << " : " << pseudojson::getValue(&val.data) << std::endl;
        }
        os.close();
    } catch (std::exception e) {
        return 0;
    }
    return 1;
}





template<>
long& pseudojson::asAny<long>(Value& value) {
    return value.data.lon;
}

template<>
const long& pseudojson::asAny<long>(const Value& value) {
    return value.data.lon;
}

template<>
bool& pseudojson::asAny<bool>(Value& value) {
    return value.data.boolSetBool;
}

template<>
const bool& pseudojson::asAny<bool>(const Value& value) {
    return value.data.boolSetBool;
}

template<>
const std::string& pseudojson::asAny<std::string>(const Value& value) {
    return value.data.string;
}

template<>
std::string& pseudojson::asAny<std::string>(Value& value) {
    return value.data.string;
}

template<>
double& pseudojson::asAny<double>(Value& value) {
    return value.data.d;
}

template<>
const double& pseudojson::asAny<double>(const Value& value) {
    return value.data.d;
}

template<>
std::vector<double>& pseudojson::asAny<std::vector<double>>(Value& value) {
    return value.data.dptr;
}

template<>
const std::vector<double>& pseudojson::asAny<std::vector<double>>(const Value& value) {
    return value.data.dptr;
}


template<>
std::vector<std::string>& pseudojson::asAny<std::vector<std::string>>(Value& value) {
    return value.data.sptr;
}

template<>
const std::vector<std::string>& pseudojson::asAny<std::vector<std::string>>(const Value& value) {
    return value.data.sptr;
}

