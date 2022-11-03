#!/bin/bash

get_filename () {
    file=$1
    mode=$2
    args=$3
    filename=$(basename -- "$1")
    if [ $args = "," ]; then    
        echo "examples/${filename%.*}-$2-default.png"
    else
        echo "examples/${filename%.*}-$2-$(echo $3| tr , _).png"
    fi
}

gen_mode_args () {
    mode=$1
    arguments=$2
    for file in img/*; do
        for args in $arguments; do
            echo $file $mode $args
            output=$(get_filename $file $mode $args)
            ./Voronizer $file -m $mode -a $args -f $output
        done
    done
}


cd $(dirname $0)
make

gen_mode_args sobel ", ,10 ,100 0,30,0"
gen_mode_args kmeans-circles ", ,30,5 1,,3,14,2 ,,20,30,2"
gen_mode_args kmeans-lines ", ,,1  ,,5,10 ,,5,100"
gen_mode_args sift-circles ", 0,,5,0.6 3,,,0.5 ,,1,2"
gen_mode_args sift-lines ", 0 3,20, 1,100"

for smooth in 0 3 10 20; do
    echo smooth $smooth
    ./Voronizer img/lena.jpg -s $smooth -f examples/smooth_$smooth.png
done

for cmap in twilight rainbow bone cividis turbo; do
    echo cmap $cmap
    ./Voronizer img/lena.jpg -c $cmap    -f examples/cmap_$cmap.png
done

for cmap in twilight bone cividis turbo; do
    echo cmap $cmap random
    ./Voronizer img/lena.jpg -c $cmap -r -f examples/cmap_"$cmap"_random.png
done

