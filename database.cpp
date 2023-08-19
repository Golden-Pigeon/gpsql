#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include "database.h"
#include "common.h"
using namespace std;
string curdb = "";
DBMeta dbMeta = DBMeta();
static string dbMetaName(){
    return string(workPath) + "/meta.dat";
}

static int recursive_delete(const char *dir)
{
    int ret = 0;
    FTS *ftsp = NULL;
    FTSENT *curr;

    // Cast needed (in C) because fts_open() takes a "char * const *", instead
    // of a "const char * const *", which is only allowed in C++. fts_open()
    // does not modify the argument.
    char *files[] = { (char *) dir, NULL };

    // FTS_NOCHDIR  - Avoid changing cwd, which could cause unexpected behavior
    //                in multithreaded programs
    // FTS_PHYSICAL - Don't follow symlinks. Prevents deletion of files outside
    //                of the specified directory
    // FTS_XDEV     - Don't cross filesystem boundaries
    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!ftsp) {
        fprintf(stderr, "%s: fts_open failed: %s\n", dir, strerror(errno));
        ret = -1;
        goto finish;
    }

    while ((curr = fts_read(ftsp))) {
        switch (curr->fts_info) {
        case FTS_NS:
        case FTS_DNR:
        case FTS_ERR:
            fprintf(stderr, "%s: fts_read error: %s\n",
                    curr->fts_accpath, strerror(curr->fts_errno));
            break;

        case FTS_DC:
        case FTS_DOT:
        case FTS_NSOK:
            // Not reached unless FTS_LOGICAL, FTS_SEEDOT, or FTS_NOSTAT were
            // passed to fts_open()
            break;

        case FTS_D:
            // Do nothing. Need depth-first search, so directories are deleted
            // in FTS_DP
            break;

        case FTS_DP:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_DEFAULT:
            if (remove(curr->fts_accpath) < 0) {
                fprintf(stderr, "%s: Failed to remove: %s\n",
                        curr->fts_path, strerror(curr->fts_errno));
                ret = -1;
            }
            break;
        }
    }

finish:
    if (ftsp) {
        fts_close(ftsp);
    }

    return ret;
}


string dbDir(const string& dbname){
    return string(workPath) + "/" + dbname;
}

void init(){
    mkdir(workPath, 0777);
    curdb = "";
    dbMeta = DBMeta();
    ofstream ofs(dbMetaName(), ios::app);
    ofs << endl;
    ofs.close();
}

void flushMetaFile(){
    ofstream ofs(dbDir(curdb) + "/meta.dat");
    for(auto it : dbMeta.tableMap){
        for(auto fdm : it.second){
            ofs << it.first << " ";
            ofs << fdm.first.second << " ";
            ofs << (fdm.second.type == INT ? "INT" : "CHAR") << " ";
            ofs << 100 << endl;
        }
    }
    ofs.close();
}

bool create_database(const string& dbname){
    ifstream ifs(dbMetaName());
    string s;
    while(getline(ifs, s)){
        if(s.size() == 0)
            continue;
        if(s == dbname){
            return false;
        }
    }
    
    ifs.close();
    ofstream ofs(dbMetaName(), ios::app);
    ofs << dbname << endl;
    ofs.close();
    mkdir(dbDir(dbname).c_str(), 0777);
    ofstream tb(dbDir(dbname) + "/meta.dat");
    tb << endl;
    tb.close();
    return true;
}

bool drop_database(const string& dbname){
    vector<string> dbs;
    string s;
    bool hasdb = false;
    ifstream ifs(dbMetaName());
    while(getline(ifs, s)){
        if(s.size() == 0)
            continue;
        if(s == dbname)
            hasdb = true;
        else
            dbs.push_back(s);
    }
    ifs.close();
    if(!hasdb){
        return false;
    }
    if(curdb == dbname){
        curdb = "";
        dbMeta.tableMap.clear();
    }
    if(recursive_delete(dbDir(dbname).c_str()) != 0)
        puts("remove failed.");
    ofstream ofs(dbMetaName());
    for(auto db : dbs){
        ofs << db << endl;
    }
    return true;
}

bool use_database(const string& dbname){
    if(curdb == dbname){
        return true;
    }
    bool found = false;
    ifstream imeta(dbMetaName());
    string s;
    while(getline(imeta, s)){
        if(s.size() == 0)
            continue;
        if(s == dbname){
            found = true;
        }
    }
    imeta.close();
    if(!found){
        return false;
    }
    curdb = dbname;
    dbMeta.tableMap.clear();
    ifstream ifs(dbDir(dbname) + "/meta.dat");
    map<string, int> idMap;
    string fieldDis;
    while(getline(ifs, fieldDis)){
        if(fieldDis.size() == 0)
            continue;
        printf("fielDis: %s\n", fieldDis.c_str());
        istringstream iss(fieldDis);
        string tbName, fieldName, type, length;
        iss >> tbName >> fieldName >> type >> length;
        if(idMap.count(tbName) == 0){
            idMap[tbName] = 0;
            dbMeta.tableMap[tbName] = FeildMap();
        } else {
            idMap[tbName]++;
        }
        dbMeta.tableMap[tbName][make_pair(tbName, fieldName)] = FeildMeta(tbName, type == "INT" ? INT : CHAR, fieldName, idMap[tbName]);
    }
    ifs.close();
    return true;
}

vector<string> showDatabases(){
    ifstream ifs(dbMetaName());
    vector<string> dbs;
    string db;
    while(ifs >> db){
        dbs.push_back(db);
    }
    ifs.close();
    return dbs;
}

vector<string> showTables(){
    if(curdb.empty())
        return vector<string>();
    vector<string> tbs;
    for(auto it : dbMeta.tableMap){
        tbs.push_back(it.first);
    }
    return tbs;
}