/*
Jordan Dehmel, 2023
jdehmel@outlook.com

Handles enumerations in Oak Enumerations
(unfortunately) must have templates. There must
be only one enumeration per name, like structs.
The insertion of enums will not be handled by
this file. Enums in Oak are very similar to
structs- The only difference in the signatures
is whether it says 'struct' or 'enum'.
*/

#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <map>
#include <string>
#include <vector>

#include "type_builder.hpp"

using namespace std;

// Used in type sizing later
const static char enumTypeStr[] = "i32";

struct __enumLookupData
{
    map<string, Type> options;
    vector<string> order;

    bool erased = false;
};

extern map<string, __enumLookupData> enumData;

#endif
