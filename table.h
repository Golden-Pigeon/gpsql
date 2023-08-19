#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdio>
#include "sql.h"
#include "database.h"
using namespace std;

// typedef vector<string> Entry;

//for select, Entry.index is useless. for other, it is important;
struct Entry {
    vector<string> entry;
    int index;
    bool operator<(const Entry& another) const {
        return index < another.index;
    }
    bool operator==(const Entry& another) const {
        return index == another.index;
    }
    Entry(vector<string> entry, int index) : entry(entry), index(index) {}
};


struct Table{
    string tbName;
    vector<Entry> items;
    FeildMap feildMap;
};

extern Table* tableProduct(const unique_ptr<Table>& t1, const unique_ptr<Table>& t2);

extern Table* tableFilter(const unique_ptr<Table>& t, const Conditions* con);

extern Table* tableRead(const string& tableName);

extern bool tableCreate(const CreateStruct* sql);

extern bool tableDrop(const string& tableName);

extern Table* tableSelect(SelectStruct* sql);

extern bool tableInsert(const InsertStruct* sql);

extern bool tableUpdate(const UpdateStruct* sql);

extern bool tableDelete(const DeleteStruct* sql);

extern Table* tableUnion(const unique_ptr<Table>& t1, const unique_ptr<Table>& t2);

extern Table* tableIntersection(const unique_ptr<Table>& t1, const unique_ptr<Table>& t2);

inline void printTable(const Table* table){
    for(auto it : table->feildMap){
        printf("%s.%s ", it.first.first.c_str(), it.first.second.c_str());
    }
    printf("\n");
    for(auto it: table->items){
        for(auto str : it.entry){
            printf("%s ", str.c_str());
        }
        printf("\n");
    }
}
inline void printTable(const unique_ptr<Table>& table){
    // for(auto it : table->feildMap){
    //     printf("%s.%s %d ", it.first.first.c_str(), it.first.second.c_str(), it.second.index);
    // }
    // printf("\n");
    vector<vector<string>> printing_list;
    for(auto it: table->items){
        bool flag = true;
        for(auto printing : printing_list){
            if(it.entry == printing){
                flag = false;
            }
            if(flag){
                printing_list.push_back(it.entry);
            }
        }
    }
    for(auto it: printing_list){
        for(auto str : it){
            printf("%s ", str.c_str());
        }
        printf("\n");
    }
}
#endif