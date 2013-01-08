./puddleworld-batch.py -r 10 -s 200000
./puddleworld.py experiment-pw/*_5_5_*/*.out
./puddleworld.py experiment-pw/*_7_7_*/*.out
./puddleworld.py experiment-pw/*_9_9_*/*.out
./puddleworld.py experiment-pw/*_11_11_*/*.out
./puddleworld.py experiment-pw/*_13_13_*/*.out
./puddleworld.py experiment-pw/*_3_13_*/*.out
./puddleworld.py \
	experiment-pw/specific_*_7_7_*/*.out \
	experiment-pw/inv-root-update-count_*_5_5_*/*.out \
	experiment-pw/inv-root-update-count_*_7_7_*/*.out \
	experiment-pw/inv-log-update-count_*_9_9_*/*.out \
	experiment-pw/inv-log-update-count_*_11_11_*/*.out \
	experiment-pw/even_*_13_13_*/*.out \
	experiment-pw/inv-root-update-count_*_3_13_*/*.out

./mountaincar-batch.py -r 10 -s 500000
./mountaincar.py experiment-mc/*_5_5_*/*.out
./mountaincar.py experiment-mc/*_7_7_*/*.out
./mountaincar.py experiment-mc/*_9_9_*/*.out
./mountaincar.py experiment-mc/*_11_11_*/*.out
./mountaincar.py experiment-mc/*_13_13_*/*.out
./mountaincar.py experiment-mc/*_3_13_*/*.out
./mountaincar.py \
	experiment-mc/specific_*_7_7_*/*.out \
	experiment-mc/inv-root-update-count_*_5_5_*/*.out \
	experiment-mc/inv-root-update-count_*_7_7_*/*.out \
	experiment-mc/inv-log-update-count_*_9_9_*/*.out \
	experiment-mc/inv-log-update-count_*_11_11_*/*.out \
	experiment-mc/even_*_13_13_*/*.out \
	experiment-mc/inv-root-update-count_*_3_13_*/*.out
