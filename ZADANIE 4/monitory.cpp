#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include "monitor.h" 

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int const threadsCounts = 4;
int const bufferSize = 9;

class Buffer : public Monitor
{
private:
    Condition full;  // producent czeka jesli bufor jest pelny
    Condition condA; // konsument A czeka tutaj
    Condition condB; // konsument B czeka tutaj

    std::vector<int> values;

    void printBuffer()
    {
        std::cout << "   [BUFOR: ";
        for (auto v : values) std::cout << v << " ";
        std::cout << "] Size: " << values.size() << "\n";
    }

    void signalNext()
    {
        if (values.size() > 3)
        {
            int val = values.front();
            if (val % 2 == 0)
            {
                signal(condA);
            }
            else
            {
                signal(condB);
            }
        }
    }

public:
    Buffer() 
    {
    }

    void put(int value)
    {
        enter();

        while (values.size() >= bufferSize)
        {
            wait(full);
        }

        values.push_back(value);
        std::cout << "PRODUCENT: Dodano " << value << ".";
        printBuffer();

        signalNext();

        leave();
    }

    int getA()
    {
        enter();
        while (values.size() <= 3 || values.front() % 2 != 0)
        {
            wait(condA);
        }

        int v = values.front();
        values.erase(values.begin());
        std::cout << "KONSUMENT A: ZABRAL " << v << ".";
        printBuffer();

        signal(full);

        signalNext();
        leave();
        return v;
    }

    int getB()
    {
        enter();


        while (values.size() <= 3 || values.front() % 2 == 0)
        {
            wait(condB);
        }

        int v = values.front();
        values.erase(values.begin());
        std::cout << "KONSUMENT B: ZABRAL " << v << ".";
        printBuffer();

        // powiadom producentow o zwolnieniu miejsca
        signal(full);

        // sprawdz czy pasuje front
        signalNext();

        leave();
        return v;
    }
};

Buffer buffer;


void* threadProd(void* arg)
{
    while (true) {
#ifdef _WIN32
        Sleep(800);
#else
        usleep(800000); // 800ms
#endif
        buffer.put(rand() % 100 + 1);
    }
    return NULL;
}

void* threadConsA(void* arg)
{
    while (true)
    {
        buffer.getA();
#ifdef _WIN32
        Sleep(1500);
#else
        usleep(8000); // 8ms
#endif
    }
    return NULL;
}

void* threadConsB(void* arg)
{
    while (true)
    {
        buffer.getB();
#ifdef _WIN32
        Sleep(1500);
#else
        usleep(8000); // 8ms
#endif
    }
    return NULL;
}

int main()
{
    srand(time(NULL));

#ifdef _WIN32
    HANDLE tid[threadsCounts];
    DWORD id;

    tid[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProd, 0, 0, &id);
    tid[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProd, 0, 0, &id);
    tid[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadConsA, 0, 0, &id);
    tid[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadConsB, 0, 0, &id);

    for (int i = 0; i < threadsCounts; i++)
        WaitForSingleObject(tid[i], INFINITE);
#else
    pthread_t tid[threadsCounts];

    pthread_create(&(tid[0]), NULL, threadProd, NULL);
    pthread_create(&(tid[1]), NULL, threadProd, NULL);
    pthread_create(&(tid[2]), NULL, threadConsA, NULL);
    pthread_create(&(tid[3]), NULL, threadConsB, NULL);

    for (int i = 0; i < threadsCounts; i++)
        pthread_join(tid[i], (void**)NULL);
#endif
    return 0;
}