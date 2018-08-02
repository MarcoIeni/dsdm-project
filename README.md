# Digital Systems Design Methodologies Project

Project for Digital Systems Design Methodologies Project course of M.Sc. Computer Science and Engineering at Politecnico di Milano.
It consists in the implementation of the [Retiming](https://en.wikipedia.org/wiki/Retiming) Algorithm described in paper [Optimized generation of data-path from C codes for FPGAs](https://ieeexplore.ieee.org/document/1395540/) in order to get the scheduling of a C piece of code.

You can find the code documentation at [this](https://marcoieni.github.io/dsdm-project) link.

## Dependencies
* Boost Graph Library

## Input
### Graph input file
The graph input file describes the graph that describes control and data dependencies of the basic operations of the c piece of code in SSA form.

The first line of the input file consists in the delay units of the operations in order and separated by a space.
The first number is always 0 because it is the weight of the entry node.
In the next lines, write the vertices of each arc of the graph separated by a space.

### Command line args
Command line args of the program in order:

1. path of the graph input file;
2. control steps number (number of flip flops in front of the entry node that needs to be redistributed);
3. required clock cycles.

## Output
The output file generated describes a graph with a similar format of the graph input file, because the first line contains the delay units of the operations, but the next lines are characterized by a third element, which is the number of flip flops that has to be inserted in the arc between the vertices that correspond to the first and the second element of each line.

## Graphs with cycles
The input graph contains cycles without any flip flop. It means that there are some cyclic combinatorial parts in the circuit.
A possible solution is to introduce flip flops in the input graph.
In order to do so you can specify the number of flip flops between each arc in the graph input file in the same way as it is done in the output file, i.e. by specifying it in the third element of the lines that describes the arcs of the graph.
