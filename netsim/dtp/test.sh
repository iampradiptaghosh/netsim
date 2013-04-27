#!/bin/bash

tests=("examples/chain" "examples/dumb_bell_1" "examples/dumb_bell_2" "examples/dumb_bell_3" "examples/test_recommended_1" "examples/test_recommended_2" "examples/test_recommended_3" "examples/test_recommended_4" "config/case1" "config/case2" "config/case3" "config/case4")
outputs=("results/res_examples_chain" "results/res_examples_dumb_bell_1" "results/res_examples_dumb_bell_2" "results/res_examples_dumb_bell_3" "results/res_examples_test_recommended_1" "results/res_examples_test_recommended_2" "results/res_examples_test_recommended_3" "results/res_examples_test_recommended_4" "results/res_config_case1" "results/res_config_case2" "results/res_config_case3" "results/res_config_case4")

rm -rf output*
rm -rf *.rcv
for i in 0 1 2 3 4 5 6 7 8 9 10 11; do
	echo "Testing ${tests[$i]}"
	./netsim_app -f ${tests[$i]} -t 4 &> output
	cat output | grep -a "Tore down FDTP flow" > output_time
	cat output_time | cut -d "(" -f2 | cut -d ")" -f1 > output_sum
	cat output_sum | gawk 'BEGIN { sum=0; n=0; }{ sum+=$1; n+=1; }END { printf("%f\n%f", sum, sum/n);}' >> output_sum
	#cat output_sum | awk '{s+=$1 n+=1} END {print s}' >> output_sum
	cat output_sum > ${outputs[$i]}
	diff test1.txt dest_test1.txt
        diff test2.txt dest_test2.txt
        diff test3.txt dest_test3.txt
        diff test4.txt dest_test4.txt
        diff test5.txt dest_test5.txt
        diff test6.txt dest_test6.txt
        diff test7.txt dest_test7.txt
        diff test8.txt dest_test8.txt
        diff test1.pdf dest_test1.pdf
        diff test2.pdf dest_test2.pdf
        diff test3.pdf dest_test3.pdf
        diff test4.pdf dest_test4.pdf
	rm -rf output*
	rm -rf *.rcv
done

