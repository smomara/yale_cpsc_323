# Yale CPSC 323 Introduction to Systems Programming & Computer Organization (Fall 2020)

This repository is designed to be a valuable resource for individuals interested in engaging with the course materials for [CPSC 323: Introduction to Systems Programming & Computer Organization](https://zoo.cs.yale.edu/classes/cs323/), as offered in Fall 2020 through the Yale Zoo website. It provides a comprehensive overview of the course structure, including a detialed schedule that outlines readings and homework assignments in the sequence they were originally presented. Additionally, users will have acces to an array of class materials, including readings and handouts, facilitating a structured and immersive learning experience.

## Overview

### Topics Covered/Emphasized
- **Systems Programming in a High Level Language**
    - User-level interfaces to a typical operating system (LINUX)
    - Writing programs (e.g. a shell) that interact with the operating system
- **Elementary Machine Architecture / Computer Organization**
    - Computer arithmetic and general structure/organization of machines
    - Approaches to parallelism (vector, SIMD, MIMD, networks)
    - Instruction set architectures (ISAs) and pipelining instruction execution
- **Operating Systems**
    - Implications of concurrency and implementation of semaphores
    - Implementation and ramifications of virtual memory and caches
- **Other**
    - Data compression; error detection and correction; computer networks

## Primary Texts

There are two primary texts used throughout the course:
- [HP6](https://github.com/smomara/yale_cpsc_323/blob/main/resources/HP6.pdf): John L. Hennesy and David A. Patterson, "Computer Architecture: A Quantitative Approach", 6th edition, Morgan Kaufman, 2017
- [MS](https://github.com/smomara/yale_cpsc_323/blob/main/resources/MS.pdf): Neil Matthew and Richard Stones, "Beginning Linux Programming", 4th edition, Wrox, 2007

## Schedule

The following table is an aggregation of the [reading assignments](https://zoo.cs.yale.edu/classes/cs323/current/reading) and the homework assignments from the course's web page on the Yale Zoo website. Homework assignments appear on the date they are due in bold.

Bear in mind these dates are from the Fall 2020 offering of the course. If you are following along, just make sure you do it in order.

| Date  | Reading                                               | Topic                                                    |
|-------|-------------------------------------------------------|----------------------------------------------------------|
| 08/31 | HP6 Appendix J, pp. 2-3                               | Ripple-carry adders                                      |
|       | HP6 Appendix J, pp. 7-8                               | Signed numbers                                           |
|       | HP6 Appendix J, pp. 37-41                             | Carry-lookahead adders                                   |
|       | MS, pp. 377-392                                       | make: [Chapter 9, Problems of Multiple Source Files], [Chapter 9, The make Command and Makefiles] |
| 09/02 | MS, pp. 94-96                                         | Linux file structure: [Chapter 3, Linux File Structure]  |
|       | MS, pp. 106-108                                       | stat() & lstat(): [Chapter 3, Low-Level File Access: Other System Calls...] |
|       | MS, pp. 109-120                                       | Standard I/O Library: [Chapter 3, The Standard I/O Library], [Chapter 3, Formatted Input and Output] |
|       | MS, pp. 120-122                                       | File & directory maintenance: [Chapter 3, File and Directory Maintenance] |
|       | MS, pp. 122-126                                       | Scanning directories: [Chapter 3, Scanning Directories]  |
|       | MS, pp. 127-128                                       | Errors: [Chapter 3, Errors]                              |
|       | MS, pp. 392-406 [optional]                            | Revision control systems: [Chapter 9, Source Code Control] |
|       | MS, pp. 430-445 [optional]                            | Debugging and gdb: [Chapter 10, General Debugging Techniques], [Chapter 10, Debugging with gdb] |
| 09/07 | HP6 Appendix J, pp. 13-16                             | Floats                                                   |
|       | HP6 Appendix J, pp. 62-65                             | Pitfalls & history                                       |
| 09/16 | [Welch's paper](http://www.cs.duke.edu/courses/spring03/cps296.5/papers/welch_1984_technique_for.pdf)            | A Technique for High-Performance Data Compression |
| 09/18 | **Homework #1**                                       | Directory "Tree" Search                                  |
| 09/21 | [LZW handout](https://zoo.cs.yale.edu/classes/cs323/current/lzw.txt)                                             | The Lempel-Ziv-Welch Algorithm |
| 09/30 | [Decimal Version of Faux CRC](http://zoo.cs.yale.edu/classes/cs323/doc/decimal-crc.txt)                          | Decimal CRC |
|       | HP6 Chapter 1, pp. 1-10                               | Overview                                                 |
|       | HP6 Appendix B, pp. 1-15                              | Memory hierarchy                                         |
|       | HP6 Chapter 2, pp. 77-84 [optional]                   | Memory hierarchy                                         |
|       | HP6 Appendix B, pp. 22-23                             | Compulsory/capacity/conflict misses                      |
| 10/02 | **Homework #2**                                       | Parsing (Some) Bash Commands                             |
| 10/05 | HP6 Appendix D: pp. 1-5 [optional]                    | Introduction to disks                                    |
|       | HP6 Appendix D: pp. 6-10                              | RAID |
|       | HP6 Chapter 2: pp. 92-94                              | Flash memory |
|       | HP6 Chapter 1: pp. 18-23 [optional]                   | Trends in technology |
|       | HP6 Chapter 1: pp. 29-36 [optional]                   | Trends in cost |
|       | HP6 Chapter 2: pp. 84-89 [optional]                   | SRAM & DRAM technology |
| 10/07 | HP6 Appendix C: pp. 2-3 | Pipelining |
|       | HP6 Chapter 1: pp. 10-11 | Parallel architectures |
|       | HP6 Chapter 1: pp. 49-51 | Amdahl's Law |
|       | HP6 Chapter 4: pp. 283-293 | Vector machines |
|       | HP6 Chapter 4: pp. 293-304 [optional] | Vector machines |
|       | HP6 Chapter 4: pp. 304-307 | SIMD |
| 10/14 | HP6 Chapter 4: pp. 310-313 | GPUs |
| | HP6 Chapter 5: pp. 367-377 | MIMD |
| | HP6 Appendix F: pp. 29-44 [optional] | Networks |
| 10/21 | HP6 Appendix B: pp. 40-49 | Virtual memory |
| 10/23 | **Homework #3** | The Shell Game: The Macroprocesor |
| 10/28 | HP6 Chapter 5: pp. 412-417 | Hardware instructions for synchronization |
| 11/02 | MS, pp. 109-120 | Standard I/O Library [Chapter 3, The Standard I/O Library], [Chapter 3, Formatted Input and Output] |
| | MS, pp. 96-108 | Systems-level I/O [Chapter 3, System Calls and Device Drivers], [Chapter 3, Library Functions], [Chapter 3, Low-Level File Access] |
| 11/04 | MS, pp. 461-464 | Processes [Chapter 11, What Is a Process], [Chapter 11, Process Structure: Introduction], [Chapter 11, Process Structure: The Process Table], [Chapter 11, Process Structure: Viewing Processes] |
| | MS, pp. 468-480 | exec() and fork(), [Chapter 11, Starting New Processes] |
| | MS, pp. 481-484 | signals [Chapter 11, Signals: Introduction] |
| | MS, pp. 531-540 | pipes and dup() [Chapter 13, The Pipe Call], [Chapter 13, Parent and Child Processes] |
| | [Example of pipes](https://zoo.cs.yale.edu/classes/cs323/current/pipe.c) | pipes |
| 11/09 | MS, pp. 23-30  | bash (variables and quoting) Chapter 2, [The Shell as a Programming Language], [Chapter 2, Shell Syntax: Variables] |
| | MS, pp. 43-45 | bash (&& and \|\|) [Chapter 2, Shell Syntax: The AND List], [Chapter 2, Shell Syntax: The OR List] | 
| | MS, pp. 73- 74 | bash (here documents) [Chapter 2, Shell Syntax: Here Documents] |
| | MS, pp. 144-148 | Environment variables in C [Chapter 4, Environment Variables] |
| | MS, pp. 156-157 | Temporary files [Chapter 4, Temporary Files] |
| 11/11 | HP6 Chapter 1: pp. 11-17 | Overview of ISA |
| | HP6 Appendix A: pp. 1-20 | Instruction Set Architecture |
| | HP6 Appendix C: pp. 2-30 | Pipelining instructions & hazards |
| 11/13 | **Homework #4** | An Ounce of Compression |
| 11/16 | HP6 Chapter 3: pp. 168-176 | Pipelining instructions & hazards |
| | HP6 Chapter 3: pp. 182-188 | Branch prediction |
| 12/11 | **Homework #5** | The Shell Game: Sister Sue Saw B-Shells ... |