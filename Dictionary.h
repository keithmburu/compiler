//
// Created by Keith_Mburu on 3/15/2021.
//

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <hc_list.h>
#include <hc_list_helpers.h>
#include <utility>

using namespace HaverfordCS;
using namespace std;

/**
 * class that instantiates dictionary with string : integer entries
 *
 * @author Keith Mburu
 * @version 3/24/2021
 */
class Dictionary {

public:

    /**
     * constructor method for empty dictionary
     */
    Dictionary() {
        dictList = list<std::pair<string,int>>();
    }

    /**
     * constructor method for new dictionary with old dictionary plus new entry
     *
     * @param oldDict old dictionary
     * @param entry string to be added to old dictionary
     * @param integer integer to be assigned to new entry
     */
    Dictionary(const Dictionary &oldDict, const string &entry, const int &integer) {
        dictList = list<pair<string,int>>(pair<string,int>(entry, integer), oldDict.dictList);
    }

    /**
     * getter method for list that stores dictionary data
     *
     * @return list that stores dictionary data
     */
    list<pair<string,int>> getList() {
        return dictList;
    }

    /**
     * setter method for list that stores dictionary data
     *
     * @param newList list to replace dictionary list
     */
    void setList(list<pair<string, int>> newList) {
        dictList = newList;
    }

    // learnt std::pair get<i>() method from https://en.cppreference.com/w/cpp/utility/pair
    // learnt to_string for int to string from http://www.cplusplus.com/reference/string/to_string/
    /**
     * method that accumulates dictionary entries in string
     *
     * @return string containing dictionary entries
     */
    string toStringHelper(const list<pair<string,int>> &dictList, string dictString) {
        if (empty(rest(dictList))) {
            return dictString += get<0>(head(dictList)) + " : " + to_string(get<1>(head(dictList)));
        }
        dictString += get<0>(head(dictList)) + " : " + to_string(get<1>(head(dictList))) + "\n";
        return toStringHelper(rest(dictList), dictString);
    }

    /**
     * method that represents dictionary as a string
     *
     * @return string containing dictionary entries
     */
    string toString() {
        return toStringHelper(getList(), "");
    }

    // other methods defined in Dictionary.cpp
    int lookup (const string &entry);
    int lookupHelper (const list<pair<string,int>> &dList, const string &entry);
    bool contains (const string &entry);
    bool containsHelper (const list<pair<string,int>> &dList, const string &entry);
    void add (string entry, int integer);
    bool replace (const string &entry, int integer);
    bool replaceHelper (list<pair<string, int>> newList, list<pair<string,int>> dList, const string &entry, int integer);
    string toCode ();

private:

    list<pair<string,int>> dictList; // list that stores dictionary data

};

#endif