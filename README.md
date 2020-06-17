# I2C_parse

Given a I^2C trace files, Write down various Functions to determine Transactions
Read the text files which are given in the file eg:T0.txt,T1.txt..... and write functions for the following 
Use Void i2c_parse(ifstream infile, ofstream outfile)

Outfile File Contents
1> Total # of Transactions
2> Total # of Master Write
3> Total # of Master Reads
4> Total # of Ack Transactions
5> Total # of Nack Transactions
6> List of Read/Write Transactions i.e
ex:  Type    Address     Data
      R       0x40       0xAB     ----> Hex Format
