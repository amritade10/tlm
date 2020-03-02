Part 1

Runing the simulation and see how the simulation time is continually incremented
The initiator is useing a quantum keeper to implement temporal decoupling
such that the initiator thread runs ahead for a quantum of 1 US before yielding control
back to the SystemC scheduler

Part 2

Trying to measure the simulation speed increase due to temporal decoupling by
comparing the speed of new version with the original. In order to notice the speed
increase

1. Increased the value of RUN_LENGTH (e.g. add 4 zeros)

Observe the effect of making the quantum larger and smaller and
ploting of CPU time against quantum for the values 250 NS, 1 US, 4US and 16 US


