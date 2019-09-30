### Multi-Paxos implementation in C++
Co-author: Fanzhong Kong

#### Abstract
Multi-paxos in pure c++ is very simple.

#### Usage
Use the makefile with `Server` and `Client` directory to compile. It will auto generate the executable.

1. Replica
Main file is `ServiceMain.cc`.
```
./server <Sid> <Skipped_Slot> 
```

Other paremeters are edited in the file `service.config`. **MAKE SURE ALL THREE SERVICE.CONFIG ARE SAME**

2. Client

```
./server <Cid> <seq> <port>
```
Other paremeters are edited in the file `service.config`. **MAKE SURE ALL THREE SERVICE.CONFIG ARE SAME**

3. Pressure Test
In the main directory.

```
python3 Service.py
```
Other paremeters are edited in the file `service.config` and `replica.config`. **MAKE SURE ALL THREE SERVICE.CONFIG ARE SAME**

The `Service.py` supports many operations. Refer to it code for more details.

NOTE:
Using this code for any course purpose will violate the Honor Code in University of Michigan.
