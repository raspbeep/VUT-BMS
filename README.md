## RDS Encoder and Decoder
This project implements an RDS (Radio Data System) encoder and decoder for groups 0A and 2A.
### Features
Encodes RDS data for groups 0A and 2A

Decodes RDS data from binary strings
Supports various RDS fields including PI, PTY, TP, MS, TA, AF, PS, and RT
### Usage
#### Encoder
``` sh
./rds_encoder -g [GROUP] [FLAGS...]
```
Example:
``` sh
./rds_encoder -g 0A -pi 12345 -pty 4 -tp 1 -ms 1 -ta 0 -af 104.5,98.0 -ps "RadioXYZ"
```
#### Decoder
``` sh
./rds_decoder -b BINARY_STRING
```
### Building
Compile the project using a C++ compiler that supports C++14 or later.
### Author
Pavel Kratochvil,
Faculty of Information Technology,
Brno University of Technology
