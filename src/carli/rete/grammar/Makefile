all: rules.tab.cpp rules.tab.hpp lex.rete_.cpp

rules.tab.cpp rules.tab.hpp: rules.y
	bison -o rules.tab.cpp -d rules.y

lex.rete_.cpp: rules.l rules.tab.hpp
	flex -o $@ --nounistd rules.l

clean:
	rm lex.rete_.cpp rules.tab.cpp rules.tab.hpp
