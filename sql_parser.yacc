%{
#define YYINITDEPTH 2000
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include "database.h"
#include "table.h"
#include "sql.h"
// #define YYSTYPE int
using namespace std;
// int yylex(void);
void yyerror(char *);

// int cnt = 0;
#include "sql_lexer.h"
%}

%union{
    string* str;
    pair<string, string>* sps;
    vector<string>* names;
    SelectedFields* sf_var;
    CreateFieldsDef* cfd_var;
    vector<SelectedFields>* sf_list;
    vector<CreateFieldsDef>* cfd_list;
    CreateStruct* cs_var;
    Conditions* cons_var;
    vector<Conditions*>* setting_list;
    vector<SelectedTables>* st_list;
    SelectStruct* ss_var;
    InsertStruct* is_var;
    DeleteStruct* ds_var;
    UpdateStruct* us_var;
    // Type type_var;
}

%token <str> NUM STRING ID
%token SELECT FROM WHERE INSERT INTO DELETE UPDATE SET TYPEINT TYPECHAR LBR RBR SEM COMMA USE DROP SHOW TABLES DATABASES TABLE DATABASE NEWLINE CREATE STAR DOT VALUES EXIT

%left OR
%left AND
%left LT LE GT GE EQ NEQ ASSIGN

%type <sf_var> SELECT_FIELD
%type <cfd_var> CREATE_FIELD
%type <sf_list> SELECT_FIELDS
%type <cfd_list> CREATE_FIELDS
%type <names> INSERT_FIELDS INSERT_VALUES
%type <cs_var> SQL_CREATE
%type <cons_var> CONS SETTING
%type <setting_list> SETTINGS
%type <st_list> SELECTED_TABLES
%type <ss_var> SQL_SELECT
%type <is_var> SQL_INSERT
%type <ds_var> SQL_DELETE
%type <us_var> SQL_UPDATE
%type <sps> TABLE_FIELD
/* %type <type_var> DTYPE */
%type SQL_CONTENT STMT DB_CREATE DB_USE DB_DROP TABLE_DROP DB_SHOW TABLE_SHOW

%%

SQL_CONTENT:
    STMT SEM NEWLINE SQL_CONTENT
    | STMT SEM NEWLINE
    /* | error SEM NEWLINE SQL_CONTENT */
    | NEWLINE
    ;

STMT:
    SQL_CREATE {TIMERBEGIN processBool(tableCreate($1));delete $1; TIMEREND tips(); }
    | SQL_SELECT {TIMERBEGIN Table* table = tableSelect($1);delete $1;  printTable(table); TIMEREND tips(); }
    | SQL_INSERT {TIMERBEGIN processBool(tableInsert($1));delete $1;  TIMEREND tips();}
    | SQL_UPDATE {TIMERBEGIN processBool(tableUpdate($1));delete $1;  TIMEREND tips();}
    | SQL_DELETE {TIMERBEGIN processBool(tableDelete($1));delete $1;  TIMEREND tips();}
    | DB_CREATE
    | DB_USE
    | DB_DROP
    | TABLE_DROP
    | DB_SHOW
    | TABLE_SHOW
    | EXIT {puts("Bye!"); exit(0);}
    ;

SQL_CREATE:
    CREATE TABLE ID LBR CREATE_FIELDS RBR {$$ = new CreateStruct(*$3, *$5);delete $3; delete $5; }
    ;

CREATE_FIELDS:
    CREATE_FIELDS COMMA CREATE_FIELD {$$ = $1; $$->push_back(*$3);delete $3; }
    | CREATE_FIELD {$$ = new vector<CreateFieldsDef>(); $$->push_back(*$1);delete $1; }
    ;

CREATE_FIELD:
    ID TYPECHAR LBR NUM RBR {$$ = new CreateFieldsDef(*$1, CHAR, atoi($4->c_str()));delete $1; delete $4; }
    | ID TYPEINT{$$ = new CreateFieldsDef(*$1, INT, 100);delete $1;}
    ;

/* DTYPE:
    TYPEINT {$$ = INT;}
    | TYPECHAR {$$ = CHAR;}
    ; */

SQL_SELECT:
    SELECT SELECT_FIELDS FROM SELECTED_TABLES WHERE CONS {$$ = new SelectStruct(*$2, *$4, $6); delete $2; delete $4; }
    | SELECT STAR FROM SELECTED_TABLES WHERE CONS {$$ = new SelectStruct(vector<SelectedFields>(), *$4, $6);delete $4; }
    | SELECT SELECT_FIELDS FROM SELECTED_TABLES {$$ = new SelectStruct(*$2, *$4, nullptr);delete $2; delete $4; }
    | SELECT STAR FROM SELECTED_TABLES {$$ = new SelectStruct(vector<SelectedFields>(), *$4, nullptr);delete $4; }
    ;

SELECT_FIELDS:
    SELECT_FIELDS COMMA SELECT_FIELD {$$ = $1; $$->push_back(*$3);delete $3; }
    | SELECT_FIELD {$$ = new vector<SelectedFields>(); $$->push_back(*$1);delete $1; }
    ;

SELECT_FIELD:
    ID {$$ = new SelectedFields("", *$1);delete $1; }
    | ID DOT ID {$$ = new SelectedFields(*$1, *$3); delete $1; delete $3; }
    ;

SELECTED_TABLES:
    SELECTED_TABLES COMMA ID {$$ = $1; $$->emplace_back(*$3); delete $3; }
    | ID {$$ = new vector<SelectedTables>(); $$->emplace_back(*$1); delete $1; }
    ;



CONS:
    CONS OR CONS {$$ = new Conditions($1, $3, COT_OR, CORT_NIL, "", "");}
    | CONS AND CONS {$$ = new Conditions($1, $3, COT_AND, CORT_NIL, "", "");}
    | TABLE_FIELD LT NUM {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_INT, *$3, ""), COT_LT, CORT_NIL, "", ""); delete $1; delete $3; }
    | TABLE_FIELD GT NUM {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_INT, *$3, ""), COT_GT, CORT_NIL, "", "");  delete $1; delete $3; }
    | TABLE_FIELD LE NUM {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_INT, *$3, ""), COT_LE, CORT_NIL, "", ""); delete $1; delete $3;  }
    | TABLE_FIELD GE NUM {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_INT, *$3, ""), COT_GE, CORT_NIL, "", "");  delete $1; delete $3; }
    | TABLE_FIELD ASSIGN NUM {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_INT, *$3, ""), COT_EQ, CORT_NIL, "", ""); delete $1; delete $3;  }
    | TABLE_FIELD NEQ NUM {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_INT, *$3, ""), COT_NEQ, CORT_NIL, "", ""); delete $1; delete $3;  }
    | TABLE_FIELD LT STRING {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_STR, *$3, ""), COT_LT, CORT_NIL, "", ""); delete $1; delete $3;  }
    | TABLE_FIELD GT STRING {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_STR, *$3, ""), COT_GT, CORT_NIL, "", "");  delete $1; delete $3; }
    | TABLE_FIELD LE STRING {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_STR, *$3, ""), COT_LE, CORT_NIL, "", "");  delete $1; delete $3; }
    | TABLE_FIELD GE STRING {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_STR, *$3, ""), COT_GE, CORT_NIL, "", ""); delete $1; delete $3;  }
    | TABLE_FIELD ASSIGN STRING {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_STR, *$3, ""), COT_EQ, CORT_NIL, "", ""); delete $1; delete $3;  }
    | TABLE_FIELD NEQ STRING {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, $1->second, $1->first), new Conditions(nullptr, nullptr, COT_NIL, CORT_STR, *$3, ""), COT_NEQ, CORT_NIL, "", ""); delete $1; delete $3;  }
    | LBR CONS RBR {$$ = $2;}
    ;

TABLE_FIELD:
    ID DOT ID {$$ = new pair<string, string>(*$1, *$3); delete $1; delete $3;   }
    | ID {$$ = new pair<string, string>("", *$1); delete $1; }
    ;

SQL_INSERT:
    INSERT INTO ID LBR INSERT_FIELDS RBR VALUES LBR INSERT_VALUES RBR {$$ = new InsertStruct(*$5, *$3, *$9); delete $3; delete $5; delete $9;   }
    ;

INSERT_FIELDS:
    INSERT_FIELDS COMMA ID {$$ = $1; $$->push_back(*$3); delete $3; }
    | ID {$$ = new vector<string>(); $$->push_back(*$1); delete $1; }
    ;

INSERT_VALUES:
    INSERT_VALUES COMMA NUM {$$ = $1; $$->push_back(*$3); delete $3; }
    | INSERT_VALUES COMMA STRING {$$ = $1; $$->push_back(*$3); delete $3; }
    | NUM {$$ = new vector<string>(); $$->push_back(*$1); delete $1; }
    | STRING {$$ = new vector<string>(); $$->push_back(*$1); delete $1; }
    ;


SQL_UPDATE:
    UPDATE ID SET SETTINGS WHERE CONS {$$ = new UpdateStruct(*$2, $6, *$4);  delete $2;  delete $4;}
    | UPDATE ID SET SETTINGS {$$ = new UpdateStruct(*$2, nullptr, *$4); delete $2;  delete $4; }
    ;

SETTINGS:
    SETTINGS COMMA SETTING {$$ = $1;  $$->push_back($3);}
    | SETTING {$$ = new vector<Conditions*>(); $$->push_back($1);}
    ;

SETTING:
    ID ASSIGN NUM {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, *$1, ""), new Conditions(nullptr, nullptr, COT_NIL, CORT_INT, *$3, ""), COT_ASSIGN, CORT_NIL, "", ""); delete $1; delete $3; }
    | ID ASSIGN STRING {$$ = new Conditions(new Conditions(nullptr, nullptr, COT_NIL, CORT_ID, *$1, ""), new Conditions(nullptr, nullptr, COT_NIL, CORT_STR, *$3, ""), COT_ASSIGN, CORT_NIL, "", ""); delete $1; delete $3; }
    ;

SQL_DELETE:
    DELETE FROM ID WHERE CONS {$$ = new DeleteStruct(*$3, $5);  delete $3;}
    ;

DB_CREATE:
    CREATE DATABASE ID {TIMERBEGIN processBool(create_database(*$3)); delete $3; TIMEREND tips(); }
    ;

DB_USE:
    USE ID {TIMERBEGIN processBool(use_database(*$2)); delete $2; TIMEREND tips(); }
    ;

DB_DROP:
    DROP DATABASE ID {TIMERBEGIN processBool(drop_database(*$3)); delete $3; TIMEREND tips(); }
    ;

TABLE_DROP:
    DROP TABLE ID {TIMERBEGIN processBool(tableDrop(*$3)); delete $3;  TIMEREND tips();}
    ;

DB_SHOW:
    SHOW DATABASES {TIMERBEGIN printVector(showDatabases());TIMEREND tips(); }
    ;

TABLE_SHOW:
    SHOW TABLES {TIMERBEGIN printVector(showTables());TIMEREND tips(); }
    ;


%%

/* #include "lex.yy.c" */

void yyerror(char *s){
    fprintf(stderr, "%s\n", s);
}

int main(void) {
    ifstream ifs("banner.txt");
    string line;
    while(getline(ifs, line)){
        puts(line.c_str());
    }
    ifs.close();
    init();
    tips();
    yyparse();
    return 0;
}