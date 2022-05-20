//
// Created by Keith_Mburu on 3/15/2021.
//

#include "Dictionary.h"

// learnt std::pair get<i>() method from https://en.cppreference.com/w/cpp/utility/pair
/**
 * method that recurses through dictionary checking for entry
 *
 * @return true or false
 */
bool Dictionary::containsHelper (const list<pair<string,int>> &dList, const string &entry) {
    if (empty(dList)) {
        return false;
    }
    else {
        if ((get<0>(head(dList)) == entry)) {
            return true;
        }
        return containsHelper(rest(dList), entry);
    }
}

/**
 * method that checks if entry is in dictionary
 *
 * @return true or false
 */
bool Dictionary::contains (const string &entry) {
    return containsHelper(getList(), entry);
}

// learnt std::pair get<i>() method from https://en.cppreference.com/w/cpp/utility/pair
/**
 * method that recurses through dictionary until it finds and returns integer assigned to entry
 *
 * @param entry string being looked up
 * @param dList dictionary data field
 * @return the integer
 */
int Dictionary::lookupHelper (const list<pair<string,int>> &dList, const string &entry) {
    if (empty(dList)) {
        return -1;
    }
    else if ((get<0>(head(dList)) == entry)) {
        return get<1>(head(dList));
    }
    return lookupHelper(rest(dList), entry);
}

/**
 * method that checks the integer assigned to entry of dictionary
 *
 * @param entry string being looked up
 * @return the integer
 */
int Dictionary::lookup (const string &entry) {
    return lookupHelper(getList(), entry);
}

/**
 * method that converts dictionary to the string that constructs it
 *
 * @return the string
 */
string Dictionary::toCode () {
    if (empty(dictList)) {
        return "Dictionary()\n";
    }
    list<pair<string, int>> temp = dictList;
    string dictCode = "Dictionary()";
    for (int i = 0; i < length(dictList); i++) {
        // learnt to_string for int to string from http://www.cplusplus.com/reference/string/to_string/
        dictCode = "Dictionary(" + dictCode + ", \"" + get<0>(first(temp)) + "\", " + to_string(get<1>(first(temp))) +
                ")";
        temp = rest(temp);
    }
    return dictCode;
}

/**
 * method that adds new entry to dictionary by constructing new data field
 *
 * @param entry string to be added
 * @param integer integer assigned to the string
 */
void Dictionary::add (string entry, int integer) {
    list<pair<string, int>> newList = list<pair<string, int>>(pair<string, int>(entry, integer), dictList);
    dictList = newList;
}

// learnt std::pair get<i>() method from https://en.cppreference.com/w/cpp/utility/pair
/**
 * method that builds new dictionary data field with redefined entry
 *
 * @return true if successful
 */
bool Dictionary::replaceHelper (list<pair<string, int>> newList, list<pair<string,int>> dList, const string &entry, int
integer) {
    if (empty(dList)) {
        // finished recursion, now replace dictionary data field
        setList(newList);
        return true;
    }
    if ((get<0>(head(dList)) == entry)) {
        newList = list<pair<string, int>>(pair<string, int>(get<0>(head(dList)), integer), newList);
        // continue recursing to build the rest of the dictionary
        return replaceHelper(newList, rest(dList), entry, integer);
    }
    newList = list<pair<string, int>>(head(dList), newList);
    return replaceHelper(newList, rest(dList), entry, integer);
}

/**
 * method that replaces the integer assigned to an entry of a dictionary
 *
 * @return true if the replacement was successful
 */
bool Dictionary::replace (const string &entry, int integer) {
    replaceHelper(list<pair<string, int>>(), getList(), entry, integer);
}


