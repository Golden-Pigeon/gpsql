#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include "sql.h"
using namespace std;

extern string curdb;

struct FeildMeta{
    string table;
    Type type;
    string field;
    int index;
    FeildMeta() {}
    FeildMeta(string table, Type type, string field, int index) : table(table), type(type), field(field), index(index) {}
};

typedef pair<string, string> tableField;

typedef map<tableField, FeildMeta> FeildMap;

// struct TableField {
//     string field;
//     Type type;
//     int length;
//     TableField(string field, Type type, int length) : field(field), type(type), length(length) {}
// };

struct DBMeta{
    map<string, FeildMap> tableMap;
};

extern DBMeta dbMeta;

extern void init();

extern string dbDir(const string& dbname);

extern void flushMetaFile();

extern bool create_database(const string& dbname);

extern bool drop_database(const string& dbname);

extern bool use_database(const string& dbname);

extern vector<string> showDatabases();

extern vector<string> showTables();

inline void printVector(const vector<string>& vec){
    for(auto str : vec){
        printf("%s\n", str.c_str());
    }
}

#endif