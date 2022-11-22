# Butilca Rares

Task:

    This is an example of an implementation for a Map-Reduce program that analyzes a set of files
    and counts the powers for a set of exponents.

    The set of exponents is given by the number of reducers, each reducer checking the powers with a
    given exponent. The exponents start from 2 and end at number of reducers + 1. Thus, if a reducer
    has the exponent 3, then it will count the unique apparitions of numbers that have the
    format x^3, with x a natural number. Note that only numbers greater than 0 are accepted.

    The mappers will return the lists of numbers they find mapped by exponent, so that the reducers
    will be able to group the numbers from a certain exponent into sets for an easy count of
    unique apparitions.

Generic functions:

    In map-reducer.h you will find a generic implementation for the mapper and reducer functions.
    The mapper maps into lists of values that the reducers will process. Note that the mapper
    function always returns lists, thus if the implementation needs the function for a file to
    return a single value, that value will be represented as a list of size 1. Furthermore,
    the reducer function receives all lists for all keys, thus the implementer is given the choice
    whether their reducer will work on the whole map or only on one key.

Input:

    A file that contains the number of files to be processed on the first line and the names of
    those files on separate lines.

    Each file to be processed begins with the number of numbers that the file contains and those
    on separate lines.

    example:

    MainFile:
    4
    File1.txt
    File2.txt
    File3.txt
    File4.txt

    File1.txt:
    6
    243
    9
    27
    243
    81
    243

    File2.txt:
    6
    81
    9
    27
    243
    27
    27

    File3.txt:
    6
    9
    27
    9
    81
    9
    53

    File4.txt:
    4
    243
    243
    243
    1

Output:

    Output will be written in files with the name out<exponent>.txt.

    example:
    out2.txt:
    3

    out3.txt:
    2

    out4.txt:
    2

    out5.txt:
    2

Run:

    The program will run with the command ./powers <mapper count> <reducer count> <main file name>.

    example:
    ./powers 4 4 file.txt will use the files written in the main file file.txt and will use 4 mapper
    threads and 4 reducer threads.

References:

    Map-reduce: https://en.wikipedia.org/wiki/MapReduce
