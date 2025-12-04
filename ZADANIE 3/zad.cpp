#include <iostream>
#include <string>
#include <vector>
#include "monitor.h"

#define BUFFER_SIZE 9
#define MIN_TO_CONSUME 3 

class Buffer
{
private:
    Semaphore mutex;       // chroni dostęp do tablicy "masz dostep"
    Semaphore empty;    // liczy wolne miejsca dla producenta "jest wolne miejsce"
    Semaphore full;     // liczy ile jest elementow "cos jest w srodku"

    int A[BUFFER_SIZE];
    int head;  
    int tail;  
    int count; 

    void printState(std::string who, std::string msg)
    {
        std::cout << " [" << who << "] -> " << msg 
                  <<"{ ";

        if (count == 0)
        {
            std::cout << "pusto";
        }
        else
        {
            int tempHead = head; 

            for (int i = 0; i < count; ++i)
            {
                std::cout << A[tempHead];

                if (i < count - 1) 
                    std::cout << ", ";

                tempHead = (tempHead + 1) % BUFFER_SIZE;
            }
        }
        std::cout << " }) " << count << "\n\n";
    }
public:
    Buffer() 
        : mutex(1),           
          empty(9), 
          full(0)
    {
        head = 0; tail = 0; 
        count = 0;
    }

    void put(int item, std::string prodName)
    {
        empty.p(); // czekaj na miejsce
        mutex.p();    // zablokuj bufor, producent z niego korzysta

        A[tail] = item;
        tail = (tail + 1) % BUFFER_SIZE;
        count++;

        printState(prodName, "WSTAWIL " + std::to_string(item));
        mutex.v();    // odblokuj bufor
        full.v();  // obudz konsumentow
    }


    bool tryGet(bool wantEven, std::string name)
    {
        // czekamy na element
        full.p(); 
        
        // sprawdzamy go
        mutex.p();

        printState(name, "PROBUJE CZYTAC...");

        // jezeli jest za malo elementow, to musimy zablokowac probe
        if (count <= MIN_TO_CONSUME)
        {
            printState(name, "BLOKADA: Za malo elementow!");
            mutex.v();   // wyjscie z bufora
            full.v(); // obudzenie innego konsumenta
            return false;
            
        }

        // pobieramy wartosc z poczatku
        int val = A[head];
        bool isEven = (val % 2 == 0);

        if (isEven != wantEven)
        {
            printState(name, "BLOKADA: Zla parzystosc!");
            mutex.v();
            full.v(); // obudzenie innego konsumenta, bo jednemu nie pasowala wartosc
            return false;
        }

        // zgadza sie parzystosc i wielkosc bufora, weic pobieramy element i przesuwamy kolejke
        int item = A[head];
        head = (head + 1) % BUFFER_SIZE;
        count--;
        
        printState(name, "SUKCES! Pobiera: " + std::to_string(item));

        mutex.v();    // odblokuj mozliwos dostepu do bufora
        empty.v(); // pobralismy element, wiec producent moze cos do bufora wstawic
        
        // zdjelismy juz element, wiec nie musimy robic full.v()
        return true;
    
    }
};

Buffer buffer;

void* watekProducent(void* arg)
{
    long id = (long)arg; 
    std::string name = (id == 1) ? "ProdA" : "ProdB";

    while(true)
    {
        int val;
        
        val = (rand() % 100) + 1;

        #ifdef _WIN32
        Sleep(800 + (id * 300)); 
        #else
        usleep(40000); 
        #endif
        
        buffer.put(val, name);
    }
    return NULL;
}

void* watekKonsumentA(void* arg)
{
    std::string name = "KonsA";
    
    while(true)
    {
        buffer.tryGet(true, name);
        
        #ifdef _WIN32
        Sleep(200);
        #else
        usleep(4000);
        #endif
        

    }
    return NULL;
}

void* watekKonsumentB(void* arg)
{
    std::string name = "KonsB";

    while(true)
    {
        buffer.tryGet(false, name);
        
        #ifdef _WIN32
        Sleep(500);
        #else
        usleep(400000);
        #endif
        
    }
    return NULL;
}

int main()
{
    srand(time(NULL));
    printf("Start: Prod A(Parzyste), Prod B(Nieparzyste), 2 Konsumentow.\n");

#ifdef _WIN32
    HANDLE tid[4];
    DWORD id;

    tid[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekProducent, (void*)1, 0, &id); 
    tid[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekProducent, (void*)2, 0, &id); 
    
    tid[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekKonsumentA, 0, 0, &id);
    tid[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watekKonsumentB, 0, 0, &id);

    WaitForSingleObject(tid[0], INFINITE); 
#else
    pthread_t p1, p2, kA, kB;

    pthread_create(&p1, NULL, watekProducent, (void*)1); 
    pthread_create(&p2, NULL, watekProducent, (void*)2); 
    pthread_create(&kA, NULL, watekKonsumentA, NULL);
    pthread_create(&kB, NULL, watekKonsumentB, NULL);

    pthread_join(p1, NULL);
#endif

    return 0;
}
