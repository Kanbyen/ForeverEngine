#!/bin/sh
DIR=$PWD
cd ..
mkdir foreverwar_build
cd foreverwar_build
cmake -G "Eclipse CDT4 - Unix Makefiles" -DECLIPSE_CDT4_GENERATE_SOURCE_PROJECT=TRUE $DIR
