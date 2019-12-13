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
#define UNSET_INTVAL (-9969420)
#define UNSET_DOUBLEVAL (-9969.420)
#define DATATYPE_SPTR (6)
#define DATATYPE_FIXEDSIZE (-10)

#include <array>
#include <tuple>
#include <map>
#include <iostream>
#include <vector>
#include <fstream>

// sequence for
template <typename T, T... S, typename F>
constexpr void for_sequence(std::integer_sequence<T, S...> seq, F&& f) {
	using unpack_t = long[];
	(void)unpack_t{(static_cast<void>(f(std::integral_constant<T, S>{})), 0)..., 0};
}

// Sample implementation of a json-like data structure. It is only there for the example to compile and actually produce a testable output
namespace pseudojson {
    struct Value;

    struct ValueData {
        std::map<std::string, Value> subObject;
        std::string string;
        long lon = UNSET_INTVAL;
        double d = UNSET_DOUBLEVAL;
        std::vector<double> dptr;
        std::vector<std::string> sptr;
        long boolSetState = UNSET_INTVAL;
        bool boolSetBool;
        long dataType = DATATYPE_FIXEDSIZE;
    };

    struct Value {
        ValueData data;

        Value& operator[](std::string name) {
            return data.subObject[std::move(name)];
        }

        const Value& operator[](const std::string &name) const {
            auto it = data.subObject.find(std::move(name));

            if (it != data.subObject.end()) {
                return it->second;
            }

            throw;
        }

        Value& operator=(const std::string &value) {
            data.string = value;
            return *this;
        }

        Value& operator=(long value) {
            data.lon = value;
            data.boolSetBool = (value == 1);
            return *this;
        }

        Value& operator=(bool value) {
            data.boolSetState = (value) ? 1 : 0;
            data.boolSetBool = value;
            return *this;
        }

        Value& operator=(double value) {
            data.d = value;
            return *this;
        }

        Value& operator=(int value) {
            data.lon = value;
            return *this;
        }

        Value& operator=(const std::vector<double> &value) {
            data.dptr = value;
            return *this;
        }

        Value& operator=(const std::vector<std::string> &value) {
            data.sptr = value;
            data.dataType = DATATYPE_SPTR;
            return *this;
        }

    };

    std::string getValue(const ValueData *v);
    std::pair<std::string, Value> stringToValue(const std::string &valueLine);
    pseudojson::Value fileToPseudoJson(std::string filename);

    std::vector<std::string> split(const std::string &s, char delim);
    bool writeToFile(Value val, std::string filename);

    template<typename T> T& asAny(Value&);
    template<typename T> const T& asAny(const Value&);

    template<>
    long& asAny<long>(Value& value);

    template<>
    const long& asAny<long>(const Value& value);

    template<>
    bool& asAny<bool>(Value& value);

    template<>
    const bool& asAny<bool>(const Value& value);

    template<>
    const std::string& asAny<std::string>(const Value& value);

    template<>
    std::string& asAny<std::string>(Value& value);

    template<>
    double& asAny<double>(Value& value);

    template<>
    const double& asAny<double>(const Value& value);

    template<>
    std::vector<double>& asAny<std::vector<double>>(Value& value);

    template<>
    const std::vector<double>& asAny<std::vector<double>>(const Value& value);
}

template<typename Class, typename T>
struct PropertyImpl {
    constexpr PropertyImpl(T Class::*aMember, const char* aName) : member{aMember}, name{aName} {}

    using Type = T;

    T Class::*member;
    const char* name;
};

// One could overload this function to accept both a getter and a setter instead of a member.
template<typename Class, typename T>
constexpr auto property(T Class::*member, const char* name) {
    return PropertyImpl<Class, T>{member, name};
}


// unserialize function
template<typename T>
T fromJson(const pseudojson::Value& data) {
    T object;

    //constexpr auto v1Props = std::tuple_size<decltype(T::properties_v1)>::value;
    // We first get the number of properties
    constexpr auto nbProperties = std::tuple_size<decltype(T::properties)>::value;

    // We iterate on the index sequence of size `nbProperties`

    for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i){
        // get the property
        constexpr auto property = std::get<i>(T::properties);

        // get the type of the property
        using Type = typename decltype(property)::Type;

        // set the value to the member
        object.*(property.member) = pseudojson::asAny<Type>(data[property.name]);
    });


    return object;
}

// unserialize function
template<typename T>
T fromJsonForest(const pseudojson::Value& data) {
    T object;

    constexpr auto nbProperties_v1 = std::tuple_size<decltype(T::properties_v1)>::value;

    int dataVers = 0;

    for_sequence(std::make_index_sequence<nbProperties_v1>{}, [&](auto i){
        constexpr auto property = std::get<i>(T::properties_v1);
        using Type = typename decltype(property)::Type;
        object.*(property.member) = pseudojson::asAny<Type>(data[property.name]);
        if (property.name == "dataVersion") {
            dataVers = data[property.name].data.lon;
        }
    });

    if (dataVers == 3) {
        constexpr auto nbProperties = std::tuple_size<decltype(T::properties_v3)>::value;

        for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i){
            constexpr auto property = std::get<i>(T::properties_v3);
            using Type = typename decltype(property)::Type;
            object.*(property.member) = pseudojson::asAny<Type>(data[property.name]);
        });
    } else if (dataVers == 5) {
        constexpr auto nbProperties = std::tuple_size<decltype(T::properties)>::value;

        for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i){
            constexpr auto property = std::get<i>(T::properties);
            using Type = typename decltype(property)::Type;
            object.*(property.member) = pseudojson::asAny<Type>(data[property.name]);
        });
    }
    return object;
}

template<typename T>
pseudojson::Value toJson(const T& object) {
    pseudojson::Value data;

    // We first get the number of properties
    constexpr auto nbProperties = std::tuple_size<decltype(T::properties)>::value;

    // We iterate on the index sequence of size `nbProperties`
    for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i){
        // get the property
        constexpr auto property = std::get<i>(T::properties);

        // set the value to the member
        data[property.name] = object.*(property.member);
    });

    return data;
}
