#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import math
import code
import shutil
from subprocess import call

# Adapted from https://hg.python.org/cpython/file/3.4/Lib/statistics.py
def mean(data):
    """Return the sample arithmetic mean of data."""
    n = len(data)
    if n < 1:
        raise ValueError('mean requires at least one data point')
    return sum(data)/n # in Python 2 use sum(data)/float(n)

def _ss(data):
    """Return sum of square deviations of sequence data."""
    c = mean(data)
    ss = sum((x-c)**2 for x in data)
    return ss

def variance(data):
    """Return the sample variance of data. """
    if iter(data) is data:
        data = list(data)
    n = len(data)
    if n < 2:
        raise StatisticsError('variance requires at least two data points')
    ss = _ss(data)
    return ss/(n-1)

def stdev(data):
    """Return the square root of the sample variance. """
    var = variance(data)
    try:
        return var.sqrt()
    except AttributeError:
        return math.sqrt(var)

def median(data):
    """Return the median (middle value) of numeric data."""
    data = sorted(data)
    n = len(data)
    if n == 0:
        raise StatisticsError("no median for empty data")
    if n%2 == 1:
        return data[n//2]
    else:
        i = n//2
        return (data[i - 1] + data[i])/2



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-n', type=int, nargs='?',
        default=20, help='The number of times to run the simulation')

    parser.add_argument('-s', type=str, required=True, nargs='?', help='The first spheres file')
    parser.add_argument('-o', type=str, required=True, nargs='?', help='The second spheres file')
    args = parser.parse_args()
    iterations = args.n

    devnull = open(os.devnull, 'w')
    currentDir = os.getcwd()
    resFilePath = currentDir + "/results.json"
    gameDir = "ZRHS2015"

    shutil.copy(args.s, 'sphere1/zr.cpp')
    shutil.copy(args.o, 'sphere2/zr.cpp')

    os.chdir("..")

    startingTime = time.time()

    results = []
    results.append([])
    results.append([])
    print "Running the simulation "+str(iterations)+" times"

    for i in range(iterations):
        currentTime = time.time()
        expectedTime = "?" if i == 0 else str((currentTime - startingTime) / i * (iterations - i))
        print "Running iteration "+str(i)+". Expected to finish in "+expectedTime+" s."

        call(["./RunTest.sh", gameDir], stdout = devnull)


        with open(resFilePath) as results_file:
            data = json.load(results_file)

            score0 = data["satData"][0]["dF"][0][-1]
            if score0 == None: score0 = data["satData"][0]["dF"][0][-2]

            score1 = data["satData"][1]["dF"][0][-1]
            if score1 == None: score1 = data["satData"][1]["dF"][0][-2]


            results[0].append(score0)
            results[1].append(score1)

    # process results

    print ""
    print "Simulation finished."
    print "Result arrays for spheres 1 and 2:"
    print results[0]
    print results[1]
    print ""
    sums = [sum(results[0]), sum(results[1])]
    print "Total scores: "+str(sums)
    print ""

    numWinsOne = 0
    numWinsTwo = 0
    numWinsDraw = 0
    for i in range(iterations):
        if results[0][i] > results[1][i]: numWinsOne += 1
        elif results[1][i] > results[0][i]: numWinsTwo += 1
        else: numWinsDraw += 1
    print "Sphere 1 won <"+str(numWinsOne)+"> times. Sphere 2 won <"+str(numWinsTwo)+"> times. Scores were equal <"+str(numWinsDraw)+"> times."
    print ""

    diffs = []
    for i in range(iterations):
        diffs.append(results[0][i] - results[1][i])

    print "Score difference array: "+str(diffs)
    print ""
    print "Score difference statistics:"
    print "Mean "+str(mean(diffs))
    print "Median "+str(median(diffs))
    print "Standard deviation "+str(stdev(diffs))
