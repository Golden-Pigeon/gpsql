#ifndef SQL_H
#define SQL_H

#include <string>
#include <vector>
#include <ctime>
#include <cstdio>

#define TIMERBEGIN do{ clock_t c_start = clock();
#define TIMEREND clock_t c_end = clock(); printf("executing time: %ld ms\n", c_end - c_start); } while(0);

using namespace std;

typedef enum{ INT, CHAR } Type;

typedef enum{ COT_AND, COT_OR, COT_LT, COT_GT, COT_EQ, COT_NEQ, COT_LE, COT_GE, COT_ASSIGN, COT_NIL } CmpOpType;

typedef enum{ CORT_ID, CORT_STR, CORT_INT, CORT_NIL } CmpOprType;

struct CreateFieldsDef {
    string field;
    Type type;
    int length;
    CreateFieldsDef(string field, Type type, int length) : field(field), type(type), length(length) {}
    CreateFieldsDef() {}
    
};

struct CreateStruct {
    string table;
    vector<CreateFieldsDef> fields;
    CreateStruct(string table, vector<CreateFieldsDef> fields) : table(table), fields(fields) {}
    CreateStruct() {}
};

struct Conditions {
    Conditions *left = nullptr;
    Conditions *right = nullptr;
    CmpOpType comp_op;
    CmpOprType type;
    string value;
    string table;
    Conditions() {}
    Conditions(Conditions *left, Conditions *right, CmpOpType comp_op, CmpOprType type, string value, string table) : left(left), right(right), comp_op(comp_op), type(type), value(value), table(table) {}
    
    ~Conditions(){
        if(left){
            delete left;
            left = nullptr;
        }
        if(right){
            delete right;
            right = nullptr;
        }
    }
};

struct SelectedFields {
    string table;
    string field;
    SelectedFields(string table, string field) : table(table), field(field) {}
    SelectedFields() {}
};

struct SelectedTables {
    string table;
    SelectedTables(string table) : table(table) {}
    SelectedTables() {}
};

struct SelectStruct {
    vector<SelectedFields> sf;
    vector<SelectedTables> st;
    Conditions *cons = nullptr;
    SelectStruct(vector<SelectedFields> sf, vector<SelectedTables> st, Conditions *cons) : sf(sf), st(st), cons(cons) {}
    SelectStruct() {}
    ~SelectStruct() {
        if(cons)
            delete cons;
    }
};

struct InsertStruct {
    vector<string> fields;
    string table;
    vector<string> ins;
    InsertStruct(vector<string> fields, string table, vector<string> ins) : fields(fields), table(table), ins(ins) {}
    InsertStruct() {}
};

struct DeleteStruct {
    string table;
    Conditions* cons = nullptr;
    DeleteStruct(string table, Conditions* cons) : table(table), cons(cons) {}
    DeleteStruct() {}
    ~DeleteStruct() {
        if(cons)
            delete cons;
    }
};

struct UpdateStruct {
    string table;
    Conditions* cons = nullptr;
    vector<Conditions*> sets;
    UpdateStruct(string table, Conditions* cons, vector<Conditions*> sets) : table(table), cons(cons), sets(sets) {}
    UpdateStruct() {}
    ~UpdateStruct() {
        if(cons)
            delete cons;
        for(auto csets : sets){
            delete csets;
        }
    }
};

inline void processBool(bool res){
    if(res){
        puts("Execution succeed.");
    } else {
        puts("Execution failed.");
    }
}

inline void tips(){
    printf("gpsql>");
}
#endif