pushd marioai/classes
gdb --args /usr/bin/java -Djava.library.path=../.. -ea ch.idsia.scenarios.Play ch.idsia.ai.agents.ai.JNIAgent -lt 0 -ld 3 -ls 42 -vis on -maxFPS on -not 20 -ewf on -i on -pw on
mv *.dot ../../
popd
