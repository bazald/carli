all: rules.tab.cpp rules.tab.hpp lex.rete.cpp

rules.tab.cpp rules.tab.hpp: rules.yyy
	bison -o rules.tab.cpp -d rules.yyy

lex.rete.cpp: rules.lll rules.tab.hpp
	flex -o $@ --nounistd rules.lll

clean:
	rm lex.rete.cpp rules.tab.cpp rules.tab.hpp
