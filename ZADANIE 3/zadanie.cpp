#include <iostream>
#include <string>
#include <ctime>
#include "monitor.h"

#define BUFFER_SIZE 9
#define MIN_TO_CONSUME 3 

class Buffer
{
private:
    Semaphore mutex;       
    Semaphore semProducer; 
    Semaphore semConsumerA;
    Semaphore semConsumerB;

    int A[BUFFER_SIZE];
    int head;  
    int tail;  
    int count; 

    int waitingA; 
    int waitingB; 

    void printBuffer(std::string action, int val)
    {
        std::cout << "\n--------------------------------------------------\n";
        std::cout << action << ": " << val << "\n";
        std::cout << "Bufor [" << count << " elem]: { ";
        
        int tempHead = head;
        for(int i = 0; i < count; i++)
        {
            std::cout << A[tempHead];
            if (i < count - 1) std::cout << ", ";
            
            tempHead = (tempHead + 1) % BUFFER_SIZE;
        }
        std::cout << " }\n";
        std::cout << "--------------------------------------------------\n";
    }

    void signalNext()
    {
        if (count > MIN_TO_CONSUME) 
        {
            int val = A[head]; 

            if (val % 2 == 0 && waitingA > 0) {
                semConsumerA.v(); 
            }
            else if (val % 2 != 0 && waitingB > 0) {
                semConsumerB.v(); 
            }
            else {
                mutex.v();
            }
        }
        else 
        {
            mutex.v();
        }
    }

public:
    Buffer() 
        : mutex(1),           
          semProducer(BUFFER_SIZE), 
          semConsumerA(0),    
          semConsumerB(0)     
    {
        head = 0;
        tail = 0;
        count = 0;
        waitingA = 0;
        waitingB = 0;
    }

    void put(int item)
    {
        semProducer.p(); // czekaj na miejsce
        mutex.p();       //s sekcja krytyczna

        // wstawianie
        A[tail] = item;
        tail = (tail + 1) % BUFFER_SIZE;
        count++;

        printBuffer("PRODUCENT dodal", item);

        signalNext(); 
    }

    int get(bool wantEven)
    {
        mutex.p();


        if (count <= MIN_TO_CONSUME || (A[head] % 2 == 0) != wantEven)
        {
            if (wantEven) waitingA++; else waitingB++; // konsument czeka na zmianes
            
            signalNext();
            
            if (wantEven) semConsumerA.p(); else semConsumerB.p();
            
            if (wantEven) waitingA--; else waitingB--;
        }

        // pobieranie
        int item = A[head];
        head = (head + 1) % BUFFER_SIZE;
        count--;

        std::string name = wantEven ? "KONSUMENT A (parzyste) pobral" : "KONSUMENT B (nieparzyste) pobral";
        printBuffer(name, item);

        semProducer.v(); // zwalnianie miejsca w buforze dla producenta
        signalNext();

        return item;
    }
};

Buffer buffer;


void * producent(void * arg)
{
    for(int i = 0; i < 20; ++i)
    {
        int r = rand() % 100 + 1; 
        buffer.put(r);
        
        #ifdef _WIN32
        Sleep(200);
        #else
        usleep(200000);
        #endif
    }
    return 0;
}

void * konsumentA(void * arg)
{
    for(int i = 0; i < 10; ++i)
    {
        buffer.get(true); // true = chce parzyste
    }
    return 0;
}

void * konsumentB(void * arg)
{
    for(int i = 0; i < 10; ++i)
    {
        buffer.get(false); // false = chce nieparzyste
    }
    return 0;
}

int main()
{
    srand(time(NULL));

#ifdef _WIN32
    HANDLE tid[4];
    DWORD id;
    tid[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producent, 0, 0, &id);
    tid[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producent, 0, 0, &id);
    tid[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)konsumentA, 0, 0, &id);
    tid[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)konsumentB, 0, 0, &id);

    for (int i = 0; i < 4; i++) WaitForSingleObject(tid[i], INFINITE);
#else
    pthread_t tid[4];
    pthread_create(&(tid[0]), NULL, producent, NULL);
    pthread_create(&(tid[1]), NULL, producent, NULL);
    pthread_create(&(tid[2]), NULL, konsumentA, NULL);
    pthread_create(&(tid[3]), NULL, konsumentB, NULL);

    for (int i = 0; i < 4; i++) pthread_join(tid[i], (void**)NULL);
#endif
    return 0;
}