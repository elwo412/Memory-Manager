unset -v file arg1 arg2

run=false
all=false
excluded=""
files=""

alloc_policy=1
num_frames=16

while getopts raf:e: flag
do
    case "${flag}" in
        r) run=true;; #Run flag
        a) all=true;; #All flag
        f) files=${OPTARG};; #Specific file flag
		e) excluded=${OPTARG};; #Exclude file flag
    esac
done

shift $(( OPTIND - 1 ))

excluded=($excluded)

if [[ ! $1 == '' ]]
then
	alloc_policy=$1
fi

if $run
then
	if $all
	then
		#RUN ALL SCRIPTS BUT EXCLUDED
		make clean; make; sleep 2; clear;
		for i in {1..12}
		do
			if ! printf '%s\0' "${excluded[@]}" | grep -F -x -z -q -- $i
			then
				echo "Starting Input $i"
				echo
				sleep 0.5;
				./proj3 $alloc_policy $num_frames sample_input/input_$i
				echo "----"
			fi
		done
	else
		#RUN SPECIFIED SCRIPTS
		make clean; make; sleep 2; clear;
		for i in $files
		do
			if ! printf '%s\0' "${excluded[@]}" | grep -F -x -z -q -- $i
			then
				echo "Starting Input $i"
				echo
				sleep 0.5;
				./proj3 $alloc_policy $num_frames sample_input/input_$i
				echo "----"
			fi
		done
	fi
else
	#VERIFY CASES
	if $all
	then
		#VERIFY ALL SCRIPTS BUT EXCLUDED
		for i in {1..12}
		do
			if ! printf '%s\0' "${excluded[@]}" | grep -F -x -z -q -- $i
			then
				DIFF=$(diff output/result-$alloc_policy-$num_frames-input_$i sample_output/result-$alloc_policy-$num_frames-input_$i)
				if [ "$DIFF" == "" ] 
				then
				    echo "TEST $i PASSED"
				else
					echo "|> TEST $i FAILED"
				fi
			fi
		done
	else
		#VERIFY SPECIFIED SCRIPTS
		for i in $files
		do
			DIFF=$(diff output/result-$alloc_policy-$num_frames-input_$i sample_output/result-$alloc_policy-$num_frames-input_$i)
			if [ "$DIFF" == "" ] 
			then
			    echo "TEST $i PASSED"
			else
				echo "|> TEST $i FAILED"
			fi
		done
	fi
fi