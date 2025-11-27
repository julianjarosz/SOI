#include <iostream>
#include <vector>
#include <string>
#include "semaphore.h"

int const threadsCounts = 5;  // number of threads

int const bufferSize = 9;

class Buffer
{
private:
	std::vector<int> values;

	Semaphore mutex;
	Semaphore full;
	Semaphore semA;
	Semaphore semB;
	Semaphore semC;
	bool readA;
	bool readB;
	bool waitA;
	bool waitB;
	bool waitC;

	void print(std::string name)
	{
		std::cout << "\n### " << name << " " << values.size() << "[";
		for (auto v : values)
			std::cout << v << ",";
		std::cout << "]###\n";
	}

public:

	Buffer() : mutex(1), readA(false), readB(false), full(bufferSize), semA(0), semB(0), semC(0), waitA(false), waitB(false), waitC(false)
	{
	}


	bool canA()
	{
		return values.size() > 3 && ! readA;
	}
	
	bool canB()
	{
		return values.size() > 3 && ! readB;
	}
	
	bool canC()
	{
		return values.size() > 3 && ! readA && ! readB;
	}

	void put(int value)
	{
		full.p();
		mutex.p();
		// insert element to buffer
		values.push_back(value);
		print("P");
		if( values.size() == 4)
		{
			if( waitA && canA() )
				semA.v();
			else
				if( waitB && canB() )
					semB.v();
				else
					if( waitC && canC() )
						semC.v();
					else
						mutex.v();
		}
		else
			mutex.v();
	}

	int getA()
	{
		mutex.p();
		if( ! canA() )
		{
			waitA = true;
			print("A wait");
			mutex.v();
			semA.p();
			waitA = false;
		}
		// read element from buffer (without removing)
		int v = values.front();
		readA = true;
		print("A read");
		if( readB )
		{
			// remove element from buffer
			values.erase(values.begin());
			print("A remove");
			readA = readB = false;
			full.v();
			if( waitB && canB() )
				semB.v();
			else
				if( waitC && canC() )
					semC.v();
				else
					mutex.v();
		}
		else
		{
			if( waitB && canB() )
				semB.v();
			else
				mutex.v();
		}
		return v;
	}
	int getB()
	{
		mutex.p();
		if( ! canB() )
		{
			waitB = true;
			print("B wait");
			mutex.v();
			semB.p();
			waitB = false;
		}
		// read element from buffer (without removing)
		int v = values.front();
		readB = true;
		print("B read");
		if( readA )
		{
			// remove element from buffer
			values.erase(values.begin());
			print("B remove");
			readA = readB = false;
			full.v();
			if( waitA && canA() )
				semA.v();
			else
				if( waitC && canC() )
					semC.v();
				else
					mutex.v();
		}
		else
		{
			if( waitA && canA() )
				semA.v();
			else
				mutex.v();
		}
		return v;
	}
	
	int getC()
	{
		mutex.p();
		if( ! canC() )
		{
			print("C wait");
			waitC = true;
			mutex.v();
			semC.p();
			waitC = false;
		}
		// read and remove element from buffer
		
		int v = values.front();
		print("C read");
		values.erase(values.begin());
		print("C remove");
		full.v();
		mutex.v();
		
		return v;
	}
};

Buffer buffer;

void* threadProd(void* arg)
{
	for (int i = 0; i < 10; ++i)
	{
		buffer.put(i);
	}
	return NULL;
}

void* threadConsA(void* arg)
{
	for (int i = 0; i < 17; ++i)
	{
		auto value = buffer.getA();
	}
	return NULL;
}

void* threadConsB(void* arg)
{
	for (int i = 0; i < 17; ++i)
	{
		auto value = buffer.getB();
	}
	return NULL;
}

void* threadConsC(void* arg)
{
	for (int i = 0; i < 17; ++i)
	{
		auto value = buffer.getC();
	}
	return NULL;
}

int main()
{
#ifdef _WIN32
	HANDLE tid[threadsCounts];
	DWORD id;

	// create threads
	tid[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProd, 0, 0, &id);
	tid[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProd, 0, 0, &id);
	tid[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadConsA, 0, 0, &id);
	tid[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadConsB, 0, 0, &id);
	tid[4] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadConsC, 0, 0, &id);

	// wait for threads to finish
	for (int i = 0; i <= threadsCounts; i++)
		WaitForSingleObject(tid[i], INFINITE);
#else
	pthread_t tid[threadsCounts];

	// create threads
	pthread_create(&(tid[0]), NULL, threadProd, NULL);
	pthread_create(&(tid[1]), NULL, threadProd, NULL);
	pthread_create(&(tid[2]), NULL, threadConsA, NULL);
	pthread_create(&(tid[3]), NULL, threadConsB, NULL);
	pthread_create(&(tid[4]), NULL, threadConsC, NULL);

	// wait for threads to finish
	for (int i = 0; i < threadsCounts; i++)
		pthread_join(tid[i], (void**)NULL);
#endif
	return 0;
}
