.PHONY: clean test cleandb test1
.DEFAULT_GOAL = gpsql
sql_lexer.cpp sql_lexer.h: sql_lexer.lex sql_parser.h sql.h
	lex --outfile=sql_lexer.cpp --header-file=sql_lexer.h sql_lexer.lex

sql_parser.cpp sql_parser.h: sql_parser.yacc database.h table.h sql.h
	yacc sql_parser.yacc --defines=sql_parser.h -o sql_parser.cpp -t

sql_lexer.o: sql_lexer.cpp sql_lexer.h sql_parser.h sql.h
	g++ -c sql_lexer.cpp

sql_parser.o: sql_parser.cpp sql_parser.h sql_lexer.h database.h table.h sql.h
	g++ -c sql_parser.cpp

database.o: database.cpp database.h common.h sql.h
	g++ -c database.cpp

table.o: table.cpp table.h common.h sql.h database.h
	g++ -c table.cpp

gpsql: sql_lexer.o sql_parser.o database.o table.o
	g++ sql_lexer.o sql_parser.o database.o table.o -o gpsql -ly

clean:
	-rm -rf sql_lexer.o sql_parser.o database.o table.o sql_lexer.cpp sql_lexer.h sql_parser.cpp sql_parser.h gpsql
 
test:
	cat test.txt | ./gpsql

test1:
	cat test1.txt | ./gpsql

cleandb:
	-rm -rf /tmp/gpsql