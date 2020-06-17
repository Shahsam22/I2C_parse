#include<fstream>
#include<iostream>
#include<string>
#include<stdlib.h>
using namespace std;
string outputFileName = "";
string fileName;
void Parse_SCL_SDA(int* itemlen,string item,char* scl, char* sda)
{
	*itemlen = item.length();
	if (*itemlen == 9)
	{
		*scl = item[4];
		*sda = item[8];
	}
	if (*itemlen == 10)
	{
		*scl = item[5];
		*sda = item[9];
	}
	if (*itemlen == 11)
	{
		*scl = item[6];
		*sda = item[10];
	}
}

string GetHexFromBin(string sBinary)
{
	string rest("0x"), tmp, chr = "0000";
	int len = sBinary.length() % 2;
	chr = chr.substr(0, len);
	sBinary = chr + sBinary;
	for (int i = 0; i<sBinary.length(); i += 4)
	{
		
		tmp = sBinary.substr(i, 4);
		if (!tmp.compare("0000"))
		{
			rest = rest + "0";
		}
		else if (!tmp.compare("0001"))
		{
			rest = rest + "1";
		}
		else if (!tmp.compare("0010"))
		{
			rest = rest + "2";
		}
		else if (!tmp.compare("0011"))
		{
			rest = rest + "3";
		}
		else if (!tmp.compare("0100"))
		{
			rest = rest + "4";
		}
		else if (!tmp.compare("0101"))
		{
			rest = rest + "5";
		}
		else if (!tmp.compare("0110"))
		{
			rest = rest + "6";
		}
		else if (!tmp.compare("0111"))
		{
			rest = rest + "7";
		}
		else if (!tmp.compare("1000"))
		{
			rest = rest + "8";
		}
		else if (!tmp.compare("1001"))
		{
			rest = rest + "9";
		}
		else if (!tmp.compare("1010"))
		{
			rest = rest + "A";
		}
		else if (!tmp.compare("1011"))
		{
			rest = rest + "B";
		}
		else if (!tmp.compare("1100"))
		{
			rest = rest + "C";
		}
		else if (!tmp.compare("1101"))
		{
			rest = rest + "D";
		}
		else if (!tmp.compare("1110"))
		{
			rest = rest + "E";
		}
		else if (!tmp.compare("1111"))
		{
			rest = rest + "F";
		}
		else
		{
			continue;
		}
	}
	return rest;
}

void i2c_parse(ifstream &infile,ofstream &outfile)
{
	infile.open(fileName);
	if (infile.fail())
	{
		cout << "Error opening file...press any key to continue..." << endl;
		cin.get();
		exit(1);
	}
	string item;
	bool inputTableHeaderFlag = true;
	int transactionCount = 0;
	int masterRead = 0, masterWrite = 0;
	int ackCount = 0, nackCount = 0;
	char currentOperation;
	int itemlen;

	char prev_scl = '1', prev_sda = '1', scl = '1', sda = '1';//prev_scl and prev_sda variables to store scl and sda values before they change, useful for look ahead
	bool StartStopFlag = false;
	bool ReadWriteFlag = false;
	bool addressReadFlag = false;

	bool dataReadFlag = false;
	bool AckFlag = false;
	bool NackFlag = false;
	string address, data1, data2;
	bool data1Received = false;
	bool data2Received = false;
	int count, addressLength=7, dataLength=8;
	
	count = 1;
	address.resize(addressLength);


	data1.resize(dataLength);
	data2.resize(dataLength);
	data2 = "00000000";

	

	string outputFileContent = "";
	string transactions = "\n6) List of Read/Write Transactions\n";
	transactions += "TYPE\t\tAddress\t\tData1\t\tData2\n";
	while (!infile.eof())
	{
		prev_scl = scl;
		prev_sda = sda;
		getline(infile, item);
		if (inputTableHeaderFlag == false)
		{
			Parse_SCL_SDA(&itemlen, item, &scl, &sda);
			if (prev_scl == '1' && prev_sda == '1' && scl == '1' && sda == '0')
			{
				transactionCount++;
				StartStopFlag = true;
				addressReadFlag = true;
				AckFlag = false;
				dataReadFlag = false;
				data1Received = false;
				data2Received = false;
				address = "0000000";
				data1 = "00000000";
				data2 = "00000000";
				continue;
			}


			if (StartStopFlag == true)
			{
				if (scl == '1')
				{
					if (addressReadFlag == true)
					{
						address[addressLength - count] = sda;
						count++;
						if (count == address.size() + 1)
						{
							count = 1;
							addressReadFlag = false;
							ReadWriteFlag = true;
							continue;
						}
					}
					if (ReadWriteFlag == true)
					{
						if (sda == '1') {
							masterRead++;
							currentOperation = 'R';
						}
						else {
							masterWrite++;
							currentOperation = 'W';
						}

						ReadWriteFlag = false;
						prev_scl = scl;
						prev_sda = sda;
						getline(infile, item);
						Parse_SCL_SDA(&itemlen, item, &scl, &sda);
						AckFlag = true;
						NackFlag = false;
						continue;
					}
					if (AckFlag == true)
					{
						AckFlag = false;
						if (scl == '1' && sda == '1')
						{
							nackCount++;
							NackFlag = true;
							data1Received = false;
							data1 = "00000000";
							data2 = "00000000";
							continue;
						}

						ackCount++;
						dataReadFlag = true;
						if (NackFlag) {
							NackFlag = false;
							data1Received = false;
							dataReadFlag = false;
							prev_scl = scl;
							prev_sda = sda;
							getline(infile, item);
							Parse_SCL_SDA(&itemlen, item, &scl, &sda);
						}
						continue;
					}
					if (dataReadFlag == true)
					{
						if (scl == '1' && sda == '0')
						{
							prev_scl = scl;
							prev_sda = sda;
							getline(infile, item);
							Parse_SCL_SDA(&itemlen, item, &scl, &sda);
							if (prev_scl == '1' && prev_sda == '0' && scl == '1' && sda == '1')
							{
								StartStopFlag = false;

								reverse(address.begin(), address.end());
								reverse(data1.begin(), data1.end());
								reverse(data2.begin(), data2.end());


								if (data2Received == true)
								{
									transactions += string(1,currentOperation) + "\t\t" + GetHexFromBin(address) + "\t\t" + GetHexFromBin(data1) + "\t\t" + GetHexFromBin(data2) + "\n";
									
								}
								else
								{
									transactions += string(1,currentOperation) + "\t\t" + GetHexFromBin(address) + "\t\t" + GetHexFromBin(data1) + "\n";
								

								}
								data1 = "00000000";
								data2 = "00000000";
								data1Received = false;
							
								continue;
							}
							else
							{
								if (data1Received == false)
									data1[dataLength - count] = prev_sda;
								else{
									data2[dataLength - count] = prev_sda;
									data2Received = true;
								}
								count++;
								if (count == 9)
								{
									count = 1;

									AckFlag = true;
									dataReadFlag = false;
									data1Received = true;
									continue;
								}
								continue;
							}
						}
						if (data1Received == false)
							data1[dataLength - count] = sda;
						else {
							data2[dataLength - count] = sda;
							data2Received = true;
						}
						count++;
						if (count == 9)
						{
							count = 1;
							AckFlag = true;
							dataReadFlag = false;
							data1Received = true;
							continue;
						}
					}

				}
			}
		}
		else {
			inputTableHeaderFlag = false;
		}
	}
	outputFileContent += "1) Total number of transactions : " +to_string(transactionCount);
	outputFileContent += "\n2) Total number of MasterWrites : " + to_string(masterWrite);
	outputFileContent += "\n3) Total number of MasterReads  : " + to_string(masterRead);
	outputFileContent += "\n4) Total number of Acknowledged Transactions     : " + to_string(ackCount);
	outputFileContent += "\n5) Total number of Not Acknowledged Transactions : " + to_string(nackCount);
	outputFileContent += transactions;
	infile.close();
	cout << "Press any key to generate output file" << endl;
	std::cin.get();
	
	outfile << outputFileContent;
	outfile.close();
	cout << "File : " << outputFileName << " is generated...press any key to exit..." << endl;
	std::cin.get();
}

int main(int argc, char **argv)
{
	ifstream infile;
	
	if (argc > 1)
	{
		fileName = argv[1];
		outputFileName = "Output_" + fileName;
		ofstream outfile(outputFileName);
		i2c_parse(infile,outfile);
	}
	else {
		cout << "No command line arguments found" << endl << endl;
		cout << "Enter File Name: ";
		getline(cin, fileName, '\n');
		outputFileName = "Output_" + fileName;
		ofstream outfile(outputFileName);
		i2c_parse(infile, outfile);
	}
}
