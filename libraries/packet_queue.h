/**
* @file packet_queue.h
* @brief defines the PacketQueue class
*
* Copyright (c) 2016 Heads Up Display, Inc
*/
#ifndef __PACKET_QUEUE__
#define __PACKET_QUEUE__

#include "mbed.h"
#include "rtos.h"
#include "hump.h"

/**
* The PacketQueue class will manage the packets in a queue
*/
class PacketQueue
{
public:
	PacketQueue(Packet* a_Buffer, int a_Count);
	virtual ~PacketQueue(void);

	void reset(void);
	bool put(Packet* a_Item);
	bool putIrq(Packet* a_Item);
	void producerCommit(void);
	bool get(Packet** a_Item);
	bool getIrq(Packet** a_Item);
	Packet* retrieveNextMessage(Packet * a_Item);
	void consumerRelease(void);
	bool peek(Packet* a_Item);

	/**
	* Get the number of items in the queue
	* 
	* @return the number of items in the queue
	*/
	int getNumberOfItems(void) { return m_ProdCount; }

	/**
	* Indicates if the queue is empty
	*
	*	@return bool indicating empty
	*/
	bool isEmpty(void) { return m_ProdCount == 0; }

protected:
	Mutex m_Mutex;					// lock for the queue data structures
	int m_Count;						// number of items in the queue
	Packet* m_Head;					// pointer to the head of the queue
	Packet* m_Tail;					// pointer to the tail of the queue
	Packet* m_ProdRdIndex;	// producer read index
	Packet* m_ProdWrIndex;	// producer write index
	Packet* m_ConRdIndex;		// consumer read index
	Packet* m_ConWrIndex;		// consumer write index
	int m_ProdCount;				// number of items in queue for producer
	int m_ConCount;					// number of items in queue for consumer
	int m_NumPendingPuts;		// Number of puts that have not yet been committed
	int m_NumPendingGets;		// Number of gets that have not yet been released
};
#endif
