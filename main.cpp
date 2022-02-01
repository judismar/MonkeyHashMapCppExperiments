#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <chrono>
#include <unistd.h>
#include <time.h>

#include "HashMap.hpp"
#include "WFEHashMap.hpp"
#include "LBEHashMap.hpp"
#include "MonkeyHashMap.hpp"

using namespace std;
using namespace chrono;

#define KEY_POOL_SIZE 5
#define KEY_POOL_MIN_LENGTH 8
#define KEY_SIZE 50

#define VALUE_MIN 0
#define VALUE_MAX 100

int NUM_PUTTERS,
	NUM_GETTERS,
	NUM_EXP,
	ITER,
	MU;

const string OUTPUT_FOLDER = "output/";
const string OUTPUT_FILE_EXTENSION = ".out";

int hashMapType = 1;

HashMap* hm;
MonkeyHashMap<string,int> *mhm;

default_random_engine generator(chrono::system_clock::now().time_since_epoch().count());
exponential_distribution<double> *exp_distribution;
vector<string> keyPool;

string getOutputFilename(int hmType, int opType, int tid) {
	return OUTPUT_FOLDER
		+ string(hmType==0?"wait_free_":"lock_based_")
		+ string(opType==0?"put_":(opType==1?"get_":"remove_"))
		+ to_string(tid)
		+ OUTPUT_FILE_EXTENSION;
}

long long getSysTime() {
	return chrono::system_clock::now().time_since_epoch().count();
}

string generateRandomString(int length) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	//static uniform_int_distribution<int> distribution(0, sizeof(alphanum) - 1);

	string s = "";

	for (int i = 0; i < length; i++)
		//s += alphanum[distribution(generator)];
		s += alphanum[rand() % 62];
	return s;
}

void generateKeyPool(int minLength, int maxLength, int size) {
	uniform_int_distribution<int> distribution(minLength, maxLength);
	for (int i = 0; i < size; i++) {
		keyPool.push_back(generateRandomString(distribution(generator)));
	}
}

string getRandomKey() {
	static uniform_int_distribution<int> distribution(0, KEY_POOL_SIZE - 1);
	return keyPool[distribution(generator)];
}

void putter(HashMap *hm, int me) {
	//static uniform_int_distribution<int> distribution(VALUE_MIN, VALUE_MAX);

	for (int i = 0; i < ITER; i++) {
		string key = generateRandomString(KEY_SIZE);
		hm->put(key, i);
	}
}

void getter(HashMap *hm, int me) {

	for (int i = 0; i < ITER; i++) {
		string key = generateRandomString(KEY_SIZE);
		int* r;
		r = hm->get(key);
	}
}

void remover(HashMap *hm, int me) {
	string filename = getOutputFilename(hashMapType, 2, me);
	ofstream fout(filename);

	long long before, after;
	for (int i = 0; i < ITER; i++) {
		string key = getRandomKey();
		before = getSysTime();
		hm->remove(key);
		after = getSysTime();
		fout<<(after-before)<<endl;

		usleep((*exp_distribution)(generator));
	}
	fout.close();
}

void putterMonkey(MonkeyHashMap<string,int> *hm, int me) {

	for (int i = 0; i < ITER; i++) {
		string key = generateRandomString(KEY_SIZE);
		hm->put(key, i);
	}
}

void getterMonkey(MonkeyHashMap<string,int> *hm, int me) {

	for (int i = 0; i < ITER; i++) {
		string key = generateRandomString(KEY_SIZE);
		int r;
		r = hm->get(key);
	}
}

void runTests() {
	cout << "Wait-free Hash Map:\n";
	milliseconds totalTime;
	string output = "x = 2:16\ny = [";
	for(int numThreads = 1; numThreads <= 15; numThreads++)
	{
		totalTime = milliseconds(0);
		thread* putterThread;
		thread* getters[numThreads];
		for(int experiment = 0; experiment < NUM_EXP; experiment++)
		{
			hm = new WFEHashMap(KEY_SIZE);
			milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			putterThread = new thread(putter, hm, 0);
			for(int i = 0; i < numThreads; i++) getters[i] = new thread(getter, hm, i+1);
			putterThread->join();
			for(int i = 0; i < numThreads; i++) getters[i]->join();
			milliseconds elapsed = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - start;
			totalTime += elapsed;
			delete putterThread;
			for(int i = 0; i < numThreads; i++) delete getters[i];
		}
		cout << "\nAverage time: " << totalTime.count()/(float)(NUM_EXP) << "\n";
		output += to_string(totalTime.count()/(float)(NUM_EXP)) + " ";
	}
	output += "]";
	ofstream fout("output_WFHM.txt");
	fout << output << endl;
	fout.close();
}

void runTestsMonkey() {
	cout << "Monkey Hash Map:\n";
	milliseconds totalTime;
	string output = "x = 2:16\ny = [";
	for(int numThreads = 1; numThreads <= 15; numThreads++)
	{
		totalTime = milliseconds(0);
		thread* putterThread;
		thread* getters[numThreads];
		for(int experiment = 0; experiment < NUM_EXP; experiment++)
		{
			mhm = new MonkeyHashMap<string,int>(ITER, 0.5);
			milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			putterThread = new thread(putterMonkey, mhm, 0);
			for(int i = 0; i < numThreads; i++) getters[i] = new thread(getterMonkey, mhm, i+1);
			putterThread->join();
			for(int i = 0; i < numThreads; i++) getters[i]->join();
			milliseconds elapsed = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - start;
			totalTime += elapsed;
			delete mhm;
			delete putterThread;
			for(int i = 0; i < numThreads; i++) delete getters[i];
		}
		cout << "\nAverage time: " << totalTime.count()/(float)(NUM_EXP) << "\n";
		output += to_string(totalTime.count()/(float)(NUM_EXP)) + " ";
	}
	output += "]";
	ofstream fout("output_monkey.txt");
	fout << output << endl;
	fout.close();
}

int main(int argc, char** argv) {
	string in_file;

	if(argc>=2)
		in_file = argv[1];
	else {
		cout<<"Enter the input file name: ";
		cin>>in_file;
	}

	ifstream fin(in_file);
	fin>>NUM_PUTTERS>>NUM_GETTERS>>NUM_EXP>>ITER>>MU;
	fin.close();

	exp_distribution = new exponential_distribution<double>(1/((double)MU));

	generateKeyPool(KEY_POOL_MIN_LENGTH, KEY_SIZE, KEY_POOL_SIZE);

	runTests();
	runTestsMonkey();

	return 0;
}
