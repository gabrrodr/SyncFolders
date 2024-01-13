# Synchronization Tool

This is a simple synchronization tool written in C (and Python) that synchronizes files and directories between a source and a replica.
The tool periodically checks for changes and updates the replica accordingly.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Testing](#testing)

## Introduction

This synchronization tool was developed in C as a learning project to explore file and directory manipulation in a Unix-like environment.
It utilizes standard C libraries to traverse directories, compare files, and perform synchronization.
I decided to also write it in Python to brush up on my skills and to challenge and prove to myself I could write it in both.
Turns out it wasn't a big challenge and I was underestimating my abilities.
It was a difficult project because I set a deadline of 3 days to complete it and I didn't have a lot of time.
I'm pretty happy with how it turned out and I'm sure I could make it better, which I wil, but for now I'll submit it.

When I started i wrote this as a guide with all the things i had to figure out and the things i already knew:

Parse arguments needed
Error message (if number of args wrong)
Need infinite loop because periodic synchronization (interval in seconds)
Const char because i dont want the functions to make alterations to these
open source directory and open LogFile
    in logFile i write alterations 
        file creation/copying/removal
Iterate through files in Source (howww?)
    -is it file or a directory?
    (get info and then see what it is)???
    directory?
        -recursive (subdirectories)
    file?
        -copy to replica (create folder if doesnt exist)
                check if the entry is in both but theyre different (byte by byte)
                    fopen and read both, if one is null continue
                    fgetc(src) and fget(path)
                    if EOF == -1? cool
                    remove the one in replica and copy the one from source
        -open source and replica file
        -read from source and write to replica
            -close source and replica file
    fopen (logFile) in append mode
close source directory and log file
Output must have the logFile operations
## Features

- Recursive synchronization of directories and subdirectories.
- Detection of changes in files (creation, deletion, modification).
- Logging of operations performed during synchronization.
- Periodic synchronization at a specified interval.

## Testing

Clone the repository and compile the synchronization tool:

bash
git clone git@github.com:gabrrodr/SyncFolders.git
cd C
-make
./syncLoop sourcePath replicaPath 30 LogFile
or
cd Python
-python programname sourcePath replicaPath 30 LogFile
