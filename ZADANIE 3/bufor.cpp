#include "monitor.h"

#define CONS_COUNT 20  //liczba konsumentow
#define ELEM_COUNT 1000  //liczba produkowanych elementow

//Bufor jednoelementowy
class Buffer: public Monitor
{
public:
	Buffer() : empty( true ), item( -1 ) { }

	void put( int item )
	{
		enter();

		if( ! empty )
			wait( emptyCond );

		empty = false;
		this->item = item;

		signal( fullCond );

		leave();
	}

	int get()
	{
		enter();

		int res;

		if( empty )
			wait( fullCond );

		empty = true;
		res = item;

		signal( emptyCond );

		leave();

		return res;
	}

protected:
	bool empty;
	int item;
	Condition emptyCond, fullCond;
};


Buffer buffer;


void * producent( void * )
{
  	printf( "Producent uruchomiony\n" );

	for( int i = 1; i <= ELEM_COUNT; ++ i )
	{
		buffer.put( i );
#ifdef _WIN32
		//Sleep( 3000 );
#else
		sleep( 3 );
#endif
	}

	//aby zakonczycz konsumentow
	for( int i = 0; i < CONS_COUNT; i ++ )
		buffer.put( 0 );

	return 0;
}



 void * konsument( void * nr )
{
	int el;
	
	printf( "Konsument %d uruchomiony\n", nr );
	do
	{
		el = buffer.get();

		printf( "Konsument nr:%d %d\n", nr, el );
	}
	while( el != 0 );

	return 0;
}

int main()
{
	int i;
#ifdef _WIN32
	HANDLE tid[CONS_COUNT+1];
	DWORD id;

	//utworz watek producenta
	tid[0] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)producent, 0, 0, & id );

	//utworz watki konsumentow
	for ( i = 1; i <= CONS_COUNT; i++)
		tid[i] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) konsument, (void*)i, 0, & id );

	for ( i = 1; i <= CONS_COUNT; i++)
		WaitForSingleObject( tid[i], INFINITE );

#else
	pthread_t tid[CONS_COUNT+1];

	//utworz watek producenta
	pthread_create(&(tid[0]), NULL, producent, 0 );

	//utworz watki konsumentow
	for ( i = 1; i <= CONS_COUNT; i++)
		pthread_create(&(tid[i]), NULL, konsument, (void*)i );


	//czekaj na zakonczenie watkow
	for ( i = 0; i < CONS_COUNT + 1; i++)
	  pthread_join(tid[i],(void **)NULL);
#endif
	return 0;
}

