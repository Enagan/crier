#!/bin/sh

#echo $PWD

inputFileDir="."
inputFile="CrierTest.proto"
outputDir="../src/protogen"

echo "Updating Protocol"

protoc -I=$inputFileDir --cpp_out=$outputDir $inputFileDir/$inputFile
