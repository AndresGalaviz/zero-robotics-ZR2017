#!/bin/bash
# Shell script to build the game, test it and open the results.
# Command line options:
#   -b: build the game. Optional.
#   -u: do not display game after building and executing it. Optional.
#   -c: use the old Flash-based visualizer. Optional.
#   -s <path>: path to the first sphere file. Optional. OVERWRITES EXISTING ZR.CPP.
#   -o <path>: path to the second sphere file. Optional. OVERWRITES EXISTING ZR.CPP.
#

OPTIND=1

game="ZRHS2017"
build=false
common_name="common" #name of the common folder

sphere_file_name="zr.cpp"
sphere_moved_file_name="zr_old.cpp"

new_vis_location="/home/zrgamedev/svn/ZeroRobotics/FrontEnd/visualization"

sphere_1_file_dir="sphere1"
sphere_2_file_dir="sphere2"

sphere_1_custom_file=false
sphere_2_custom_file=false

unshow_simulation=false

old_visualizer=false

function cleanup {
  cd $starting_dir
  if [ -n "$server_pid" ]; then
    kill -9 $server_pid
  fi

  if [ -n "$firefox_pid" ]; then
    kill $firefox_pid
  fi
}

trap cleanup EXIT
trap cleanup INT

# check if the sphere is in the right directory
if [ ! -d $common_name ]; then
  echo "Can't find folder '$common_name'. Check if you are in ZRHS2016"
  exit 0
fi

starting_dir=$(pwd)

while getopts "cbus:o:" opt; do
    case "$opt" in
    b)  build=true
        ;;
    s)  sphere_1_custom_file_path=$OPTARG
        sphere_1_custom_file=true
        ;;
    o)  sphere_2_custom_file_path=$OPTARG
        sphere_2_custom_file=true
        ;;
    u)  unshow_simulation=true
        ;;
    c)  old_visualizer=true
        ;;
    esac
done

# shift $((OPTIND-1))

# [ "$1" = "--" ] && shift

# echo "verbose=$verbose, output_file='$output_file', Leftovers: $@"

if $sphere_1_custom_file; then
  if [ ! -e $sphere_1_custom_file_path ]; then echo "Sphere 1 file not found. Exiting."; exit 0; fi
  cp $sphere_1_custom_file_path "$sphere_1_file_dir/$sphere_file_name"
fi

if $sphere_2_custom_file; then
  if [ ! -e $sphere_2_custom_file_path ]; then echo "Sphere 2 file not found. Exiting."; exit 0; fi
  cp $sphere_2_custom_file_path "$sphere_2_file_dir/$sphere_file_name"
fi

cd ..
if $build; then
  echo "Building the game\n"
  ./BuildGame.sh $game
else
  echo "Skipped building the game. If you want that, use ./BuildRunShow.sh -b"
fi

echo "\nRunning the simulation\n"
./RunTest.sh $game

if $unshow_simulation; then
    echo "\nSkipped displaying the simulation.\n"
else
  if $old_visualizer; then
    echo "Using the old visualizer."
    cp $game/results.json ../Visualization_JSON/bin/results.json
    DISPLAY=:0.0 firefox ~/svn/ZeroRobotics_VM/Visualization_JSON/bin/pv3dtest.html
  else
    cp $game/results.json $new_vis_location/json/duelResult.json
    cd $new_vis_location
    gulp watch
    wait

    cleanup
  fi
fi
