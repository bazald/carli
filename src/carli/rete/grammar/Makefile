all: rules.tab.cpp lex.rete.cpp

rules.tab.cpp: rules.tab.out
	cat preamble.txt rules.tab.out postamble.txt > $@

rules.tab.out: rules.yyy
	bison -o rules.tab.cpp -d rules.yyy
	mv rules.tab.cpp rules.tab.out

lex.rete.cpp: lex.rete.out
	cat preamble.txt lex.rete.out postamble.txt > $@

lex.rete.out: rules.lll
	flex -o lex.rete.cpp --nounistd rules.lll
	mv lex.rete.cpp lex.rete.out

clean:
	rm lex.rete.cpp lex.rete.out rules.tab.cpp rules.tab.out rules.tab.hpp
