# whichcpus

A simple program for Linux to show which CPU cores are actually
available.  It determines this empirically, by starting many
simultaneous threads and seeing which cores they run on.

## Usage:

To build:

    $ make

Then e.g.:

    $ ./whichcpus
    0,1,2,3
    $ taskset -c 1,3 ./whichcpus
    1,3
    $ taskset -c 1 sh -c 'while true ; do : ; done' &
    $ ./whichcpus
    0,2,3
