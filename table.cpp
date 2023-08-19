#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <set>
#include <memory>
// #include <ultility>
#include <sys/stat.h>
#include "table.h"
#include "common.h"
using namespace std;

static void flushTable(const unique_ptr<Table>& t);

static Type strType(const string& line) {
    char* p;
    strtol(line.c_str(), &p, 10);
    return *p == 0 ? INT : CHAR;
}

static void setTbName(Conditions* cons, const string& tbName){
    if(cons->left){
        setTbName(cons->left, tbName);
    }
    if(cons->right){
        setTbName(cons->right, tbName);
    }
    if(cons->left == nullptr && cons->right == nullptr){
        cons->table = tbName;
    }
}

static string tablePath(const string& tableName){
    return string(workPath) + "/" + curdb + "/" + tableName + ".db";
}

Table* tableRead(const string& tableName){
    if(dbMeta.tableMap.count(tableName) == 0){
        return nullptr;
    }
    Table* table = new Table;
    table->tbName = tableName;
    table->feildMap = dbMeta.tableMap[tableName];
    ifstream ifs(tablePath(tableName));
    string line;
    int cnt = 0;
    while(getline(ifs, line)){
        if(line.size() == 0){
            continue;
        }
        // puts(line.c_str());
        istringstream iss(line);
        vector<string> entry;
        string field;
        while(iss >> field){
            entry.push_back(field);
        }
        table->items.emplace_back(entry, cnt);
        cnt++;
    }
    return table;
}

Table* tableProduct(const unique_ptr<Table>& t1, const unique_ptr<Table>& t2){
    if(t1->tbName == t2->tbName){
        return nullptr;
    }
    Table* table = new Table;
    table->tbName = t1->tbName + "_" + t2->tbName;
    for(auto it : t2->feildMap){
        table->feildMap[it.first] = it.second;
        
    }
    for(auto it : t1->feildMap){
        table->feildMap[it.first] = it.second;
        table->feildMap[it.first].index += t2->feildMap.size();
    }
    int cnt = 0;
    for(auto it1 : t1->items){
        for(auto it2 : t2->items){
            vector<string> entry;
            for(auto fld : it2.entry){
                entry.push_back(fld);
            }
            for(auto fld : it1.entry){
                entry.push_back(fld);
            }
            
            table->items.emplace_back(entry, cnt);
            cnt++;
        }
    }
    return table;
}

bool tableDrop(const string& tableName){
    if(dbMeta.tableMap.count(tableName) == 0){
        return false;
    }
    dbMeta.tableMap.erase(tableName);
    flushMetaFile();
    remove(tablePath(tableName).c_str());
    return true;
}

Table* tableIntersection(const unique_ptr<Table>& t1, const unique_ptr<Table>& t2){
    if(t1->tbName != t2->tbName){
        return nullptr;
    }
    Table* table = new Table;
    table->tbName = t1->tbName;
    table->feildMap = t1->feildMap;
    for(auto it1 : t1->items){
        for(auto it2 : t2->items){
            if(it1.index == it2.index){
                table->items.push_back(it1);
            }
        }
    }
    return table;
}

Table* tableUnion(const unique_ptr<Table>& t1, const unique_ptr<Table>& t2){
    if(t1->tbName != t2->tbName){
        return nullptr;
    }
    Table* table = new Table;
    table->tbName = t1->tbName;
    table->feildMap = t1->feildMap;
    set<Entry> st;
    for(auto it1 : t1->items){
        st.insert(it1);
    }
    for(auto it2 : t2->items){
        st.insert(it2);
    }
    for(auto it : st){
        table->items.push_back(it);
    }
    return table;
}

bool tableCreate(const CreateStruct* sql){
    if(dbMeta.tableMap.count(sql->table) > 0){
        return false;
    }
    dbMeta.tableMap[sql->table] = FeildMap();

    // int cnt = 0;
    ofstream ofs(tablePath(sql->table));
    ofs << endl;
    ofs.close();
    for(auto cField : sql->fields){
        dbMeta.tableMap[sql->table][make_pair(sql->table, cField.field)] = FeildMeta(sql->table, cField.type, cField.field, 0);
        // cnt++;
    }
    int cnt = 0;
    for(auto& it : dbMeta.tableMap[sql->table]){
        it.second.index = cnt;
        cnt++;
    }
    flushMetaFile();
    return true;
}
//TODO: check whether the cons is legal.
Table* tableSelect(SelectStruct* sql){
    // puts("selecting");
    vector<unique_ptr<Table>> tables;
    auto sTables = sql->st;
    if(sTables.size() == 1){
        if(sql->cons){
            setTbName(sql->cons, sql->st[0].table);
        }
        for(int i = 0; i < sql->sf.size(); i++){
            sql->sf[i].table = sql->st[0].table;
        }   
    }
    for(auto tbName : sTables){
        // printf("tblen: %ld\n", dbMeta.tableMap.count(tbName.table));
        if(dbMeta.tableMap.count(tbName.table) == 0){
            return nullptr;
        }
    }
    for(auto tbName : sTables){
        tables.emplace_back(tableRead(tbName.table));
    }
    bool flag = false;
    vector<SelectedFields> sfTmp;
    if(sql->sf.empty()){
        // puts("selecting star");
        flag = true;
        for(auto& tb : tables){
            for(auto it : tb->feildMap){
                // printf("%s.%s\n", it.first.first.c_str(), it.first.second.c_str());
                sfTmp.emplace_back(it.first.first, it.first.second);
            }
        }
    }
    // concat tables
    while(tables.size() > 1){
        unique_ptr<Table> a(std::move(tables[tables.size() - 1]));
        tables.pop_back();
        unique_ptr<Table> b(std::move(tables[tables.size() - 1]));
        tables.pop_back();
        tables.emplace_back(tableProduct(a, b));
        a.reset();
        b.reset();
    }
    unique_ptr<Table> t(std::move(tables[0]));
    // printTable(t);
    Table* filterRes = tableFilter(t, sql->cons);
    Table* res = new Table;
    res->tbName = filterRes->tbName + "_selected_res";
    auto sFields = sql->sf;
    if(flag){
        sFields = sfTmp;
    }
    // get selected fields meta
    for(auto fields : sFields){
        res->feildMap[make_pair(fields.table, fields.field)] = filterRes->feildMap[make_pair(fields.table, fields.field)];
    }
    // get selected fields content
    for(auto ito : filterRes->items){
        vector<string> entry;
        for(auto iti : res->feildMap){
            // printf("%d ", iti.second.index);
            entry.push_back(ito.entry[iti.second.index]);
        }
        // puts("");
        res->items.emplace_back(entry, ito.index);
    }
    delete filterRes;
    return res;

}


bool tableInsert(const InsertStruct* sql){
    if(dbMeta.tableMap.count(sql->table) == 0){
        return false;
    }
    if(sql->ins.size() != sql->fields.size())
        return false;
    unique_ptr<Table> table(tableRead(sql->table));
    for(auto fds: sql->fields){
        if((table->feildMap).count(make_pair(sql->table, fds)) == 0){
            return false;
        }
    }
    for(int i = 0; i < sql->ins.size(); i++){
        if(strType(sql->ins[i]) != table->feildMap[make_pair(sql->table, sql->fields[i])].type){
            return false;
        }
    }
    vector<string> entry;
    for(int i = 0; i < table->feildMap.size(); i++){
        entry.push_back("null");
    }
    for(int i = 0; i < sql->ins.size(); i++){
        entry[table->feildMap[make_pair(sql->table, sql->fields[i])].index] = sql->ins[i];
    }
    table->items.emplace_back(entry, table->items.size());
    flushTable(table);
    return true;
}

bool tableUpdate(const UpdateStruct* sql){
    if(dbMeta.tableMap.count(sql->table) == 0){
        return false;
    }
    if(sql->cons){
        setTbName(sql->cons, sql->table);
    }
    unique_ptr<Table> table(tableRead(sql->table));
    for(auto csets : sql->sets){
        if(table->feildMap.count(make_pair(sql->table, csets->left->value)) == 0)
            return false;
        if(csets->right->type == CORT_INT && table->feildMap[make_pair(sql->table, csets->left->value)].type == CHAR || csets->right->type == CORT_STR && table->feildMap[make_pair(sql->table, csets->left->value)].type == INT) {
            return false;
        }
    }
    unique_ptr<Table> filterRes(tableFilter(table, sql->cons));
    for(auto it : filterRes->items){
        for(auto csets : sql->sets){
            table->items[it.index].entry[table->feildMap[make_pair(sql->table, csets->left->value)].index] = csets->right->value;
        }
    }
    flushTable(table);
    return true;
}


//TODO: check whether the cons is legal.
bool tableDelete(const DeleteStruct* sql){
    if(dbMeta.tableMap.count(sql->table) == 0){
        return false;
    }
    if(sql->cons){
        setTbName(sql->cons, sql->table);
    }
    unique_ptr<Table> table(tableRead(sql->table));
    unique_ptr<Table> filterRes(tableFilter(table, sql->cons));
    unique_ptr<Table> res(new Table);
    res->feildMap = table->feildMap;
    res->tbName = table->tbName;
    for(auto it1 : table->items){
        bool flag = true;
        for(auto it2 : filterRes->items){
            if(it1.index == it2.index){
                flag = false;
            } 
        }
        if(flag){
            res->items.emplace_back(it1.entry, it1.index);
        }
    }
    flushTable(res);
    return true;
}

static bool checkInequality(const string& v1, const string& v2, CmpOpType cot, CmpOprType cort){
    if(v1 == "null" && v2 == "null" && cot == COT_EQ){
        return true;
    }
    if(v1 == "null" || v2 == "null"){
        return false;
    }
    if(cort == CORT_INT && (strType(v1) != INT || strType(v2) != INT))
        return false;
    int a, b;
    if(cort == CORT_INT){
        a = atoi(v1.c_str());
        b = atoi(v2.c_str());
        switch (cot)
        {
        case COT_LT:
            return a < b;
            break;
        case COT_GT:
            return a > b;
            break;
            
        case COT_EQ:
            return a == b;
            break;

        case COT_NEQ:
            return a != b;
            break;

        case COT_LE:
            return a <= b;
            break;

        case COT_GE:
            return a >= b;
            break;
        default:
            return false;
            break;
        }
    } else {
        switch (cot)
        {
        case COT_LT:
            return v1 < v2;
            break;
        case COT_GT:
            return v1 > v2;
            break;
            
        case COT_EQ:
            return v1 == v2;
            break;

        case COT_NEQ:
            return v1 != v2;
            break;

        case COT_LE:
            return v1 <= v2;
            break;

        case COT_GE:
            return v1 >= v2;
            break;
        default:
            return false;
            break;
        }
    }
    

}

Table* tableFilter(const unique_ptr<Table>& t, const Conditions* con){
    if(con == nullptr){
        Table* res = new Table;
        res->tbName = t->tbName;
        res->feildMap = t->feildMap;
        res->items = t->items;
        return res;
    }
    if(con->left->left == nullptr){
        Table* res = new Table;
        res->tbName = t->tbName;
        res->feildMap = t->feildMap;
        int index = t->feildMap[make_pair(con->left->table, con->left->value)].index;
        // puts("filter field");
        // printf("%s %s\n", con->table.c_str(), con->left->value.c_str());
        // puts("beginning filter");
        for(auto it : t->items){
            // printf("%s %s\n", it.entry[index].c_str(), con->right->value.c_str());
            if(checkInequality(it.entry[index], con->right->value, con->comp_op, con->right->type)){
                res->items.push_back(it);
            }
        }
        // puts("filter res");
        // printTable(res);
        return res;
    }
    unique_ptr<Table> L(tableFilter(t, con->left));
    unique_ptr<Table> R(tableFilter(t, con->right));
    Table* res;
    if(con->comp_op == COT_AND){
        res = tableIntersection(L, R);
    } else {
        res = tableUnion(L, R);
    }
    return res;
}

static void flushTable(const unique_ptr<Table>& t){
    ofstream ofs(tablePath(t->tbName));
    for(auto it : t->items){
        for(auto field : it.entry){
            ofs << field << " ";
        }
        ofs << endl;
    }
    ofs.flush();
    ofs.close();
}