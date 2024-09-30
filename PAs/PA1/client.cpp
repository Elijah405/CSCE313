/*
	Author of the starter code
	Yifan Ren
	Department of Computer Science & Engineering
	Texas A&M University
	Date: 9/15/2024

	Please include your Name, UIN, and the date below
	Name:
	UIN:
	Date:
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <sys/wait.h>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	int m2 = MAX_MESSAGE;
	string filename = "";
	bool new_chan = false;
	vector<FIFORequestChannel *> channels;

	// Add other arguments here
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1)
	{
		switch (opt)
		{
		case 'p':
			p = atoi(optarg);
			break;
		case 't':
			t = atof(optarg);
			break;
		case 'e':
			e = atoi(optarg);
			break;
		case 'f':
			filename = optarg;
			break;
		case 'm':
			m2 = atoi(optarg);
			break;
		case 'c':
			new_chan = true;
			break;
		}
	}

	// Task 1:
	// Run the server process as a child of the client process

	char mStr[12];
	sprintf(mStr, "%d", m2);
	char *cmd1[] = {(char *)"./server", (char *)"-m", mStr, nullptr};

	pid_t pid1 = fork();

	if (pid1 == 0)
	{

		execvp(cmd1[0], cmd1);
		perror("exec failed");
		exit(1);
	}

	FIFORequestChannel chans("control", FIFORequestChannel::CLIENT_SIDE);
	channels.push_back(&chans);

	// Task 4:
	// Request a new channel
	if (new_chan)
	{
		MESSAGE_TYPE newChanel = NEWCHANNEL_MSG;
		chans.cwrite(&newChanel, sizeof(MESSAGE_TYPE));
		string nameOfNew;
		chans.cread(&nameOfNew, sizeof(string));
		FIFORequestChannel *sisterChannel = new FIFORequestChannel(nameOfNew, FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(sisterChannel);
	}

	FIFORequestChannel chan = *(channels.back());

	// Task 2:
	// Request data points
	if (p != -1 && t != -1 && e != -1)
	{
		char buf[MAX_MESSAGE];
		datamsg x(p, t, e);

		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg));
		double reply;
		chan.cread(&reply, sizeof(double));
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	else if (p != -1)
	{
		ofstream outfile("/home/elijahorozco405/CSCE313/PAs/PA1/received/x1.csv");
		if (!outfile.is_open())
		{
			std::cerr << "Error opening file!" << std::endl;
			return 1;
		}

		for (int i = 0; i < 1000; i++)
		{

			char buf[MAX_MESSAGE];
			datamsg x(p, (i * 0.004), 1);
			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg));
			double reply;
			chan.cread(&reply, sizeof(double));

			outfile << i * 0.004 << "," << reply << ",";

			char buf3[MAX_MESSAGE];
			datamsg x3(p, (i * 0.004), 2);
			memcpy(buf3, &x3, sizeof(datamsg));
			chan.cwrite(buf3, sizeof(datamsg));
			double reply3;
			chan.cread(&reply3, sizeof(double));

			outfile << reply3 << endl;
			// cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		}
		outfile.close();
	}

	// Task 3:
	// Request files
	if (!(filename == ""))
	{
		string path = "/home/elijahorozco405/CSCE313/PAs/PA1/received/";
		ofstream outfile(path + filename);
		if (!outfile.is_open())
		{
			std::cerr << "Error opening file!" << std::endl;
			return 1;
		}

		filemsg fm(0, 0);
		string fname = filename;

		int len = sizeof(filemsg) + (fname.size() + 1);
		char *buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		chan.cwrite(buf2, len);

		__int64_t file_length;
		chan.cread(&file_length, sizeof(__int64_t));

		char *bufResponse = new char[m2];

		// loop file size/buffer capacity
		int loopSize = 0;
		if (file_length % m2 != 0)
		{
			loopSize = file_length / m2 + 1;
		}
		else
		{
			loopSize = file_length / m2;
		}

		// loop to get data
		for (int index = 0; index < loopSize; index++)
		{

			int off = m2 * index;
			int remainder = file_length - m2 * index;

			filemsg *file_request = (filemsg *)buf2;
			file_request->offset = off;

			file_request->length = (m2 < remainder) ? m2 : remainder;

			chan.cwrite(buf2, len);
			int buffSize = (m2 < remainder) ? m2 : remainder;

			chan.cread(bufResponse, buffSize);
			outfile.write(bufResponse, (m2 < remainder) ? m2 : remainder);
		}
		outfile.close();

		delete[] buf2;
		delete[] bufResponse;

		// cout << "The length of " << fname << " is " << file_length << endl;
	}

	if (new_chan)
	{
		for (auto chans : channels)
		{
			delete chans;
		}
		MESSAGE_TYPE m = QUIT_MSG;
		chan.cwrite(&m, sizeof(MESSAGE_TYPE));
	}

	// Task 5:
	//  Closing all the channels
	MESSAGE_TYPE m = QUIT_MSG;
	chan.cwrite(&m, sizeof(MESSAGE_TYPE));
}
