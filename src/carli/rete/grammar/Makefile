all: rules.tab.cpp lex.rete.cpp

rules.tab.cpp: rules.tab.out
	cat preamble.txt rules.tab.out postamble.txt > $@

rules.tab.out: Makefile rules.yyy
	bison -o rules.tab.cpp -d rules.yyy
	mv rules.tab.cpp rules.tab.out

lex.rete.cpp: lex.rete.out
	cat preamble.txt lex.rete.out postamble.txt > $@

lex.rete.out: Makefile rules.lll
	flex -o lex.rete.cpp rules.lll
	mv lex.rete.cpp lex.rete.out

clean:
	rm lex.rete.cpp lex.rete.hh lex.rete.out rules.tab.cpp rules.tab.hh rules.tab.out
