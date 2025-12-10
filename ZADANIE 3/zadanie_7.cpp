#include <iostream>
#include <vector>
#include <string>
#include "semaphore.h"

// 2 consumers, 2 producers. fifo buffer 9 elements. value can be read only if count>3. consument a reads even, consument b reads odd.



int const threadsCounts = 4;  // number of threads

int const bufferSize = 9;


class Buffer
{
private:
	Semaphore mutex;
	Semaphore empty;
	Semaphore semA;
	Semaphore semB;

	std::vector<int> values;

	void print(std::string name)
	{
		std::cout << "\n### " << name << " " << values.size() << "[";
		for (auto v : values)
			std::cout << v << ",";
		std::cout << "]###" << std::endl;
	}

public:
	Buffer() : mutex(1), empty(bufferSize), semA(0), semB(0)
	{
	}

	void put(int value)
	{
		empty.p(); // wait for a free slot
		mutex.p();
		// insert element to buffer
		values.push_back(value);
		print("P");
		if(values.size() == 4){
			signalNextConsumer();
		}
		mutex.v();
	
	}
    void signalNextConsumer()
    {
        if (values.size() > 3)
        {
            int val = values.front();
            if (val % 2 == 0)
            {
                std::cout << "Open semaphore A.\n";
                semA.v();
            }
            else
            {
                std::cout << "Open semaphore B.\n";
                semB.v();
            }
        }
    }
	int getA()
	{
   
		semA.p(); // wait for parity slot
		mutex.p();
		// read element from buffer (without removing)
		int v = values.front();
		print("A read");
        // remove element from buffer
        values.erase(values.begin());
        print("A remove");
        empty.v(); // signal, that there is a free slot
        signalNextConsumer(); // pass the turn
		mutex.v(); 
		return v;
	}

	int getB()
	{
		semB.p();
		mutex.p();
		// read element from buffer (without removing)
		int v = values.front();
		print("B read");
        // remove element from buffer
        values.erase(values.begin());
        print("B remove");
        empty.v(); // signal, that there is a free slot
        signalNextConsumer(); // pass the turn
		mutex.v();
		return v;
	}
};

Buffer buffer;

void* threadProd(void* arg)
{
//     int values[] = {0,1,2,3,4,5,6,7,8,9};
//     while(true){
// 	for (int i = 0; i < 10; ++i)
// 	{
// 		buffer.put(values[i]);
//         #ifdef _WIN32
//         Sleep(500);
//         #else
//         usleep(60000);
//         #endif
// 	}
// }
    while(true){
        usleep(60000);
        buffer.put(random()%100 + 1);

    }
	return NULL;
}

void* threadConsA(void* arg)
{
	
     while(true)
    {
        auto value = buffer.getA();
        #ifdef _WIN32
        Sleep(1000);
        #else
        usleep(60000);
        #endif
    }
	return NULL;
}

void* threadConsB(void* arg)
{
	while(true)
    {
        auto value = buffer.getB();
        #ifdef _WIN32
        Sleep(1000);
        #else
        usleep(60000);
        #endif
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

	// wait for threads to finish
	for (int i = 0; i < threadsCounts; i++)
		pthread_join(tid[i], (void**)NULL);
#endif
	return 0;
}
