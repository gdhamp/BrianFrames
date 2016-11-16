/**
* @file packet_queue.cpp
* @brief implements the Easing and LedStateMachine objects
*
* Copyright (c) 2016 Heads Up Display, Inc
*/
#include "defines.h"
#include "packet_queue.h"

/**
* Create a PacketQueue object with an array of packets
*
* @param a_Buffer - an array of Packets
* @param a_Count - number of items packet array
*/
PacketQueue::PacketQueue(Packet* a_Buffer, int a_Count)
{
	// Set up the fixed stuff
	
	// store pointers to keep track of the queue
	m_Head = a_Buffer;
	// set the tail/clear the indices/set the size and length
	// to be clear, m_Tail points to the "item" that is 1 past
	// the last item
	m_Tail = m_Head + a_Count;
	m_Count = a_Count;

	//Reset the dynamic stuff
	reset();
}

/**
* Destructor
*/
PacketQueue::~PacketQueue()
{
}

/**
* Reset the queue
*/
void PacketQueue::reset(void)
{
	m_ProdRdIndex = m_Head;
	m_ProdWrIndex = m_Head;
	m_ConRdIndex = m_Head;
	m_ConWrIndex = m_Head;
	m_ProdCount = 0;
	m_ConCount = 0;
	m_NumPendingPuts = 0;
	m_NumPendingGets = 0;
}

/**
* Add item to queue
*
* @param a_Item - item to add
* @return true if item added, false if queue full
*/
bool PacketQueue::put(Packet* a_Item)
{
	bool a_Result = false;

	m_Mutex.lock();
	a_Result = putIrq(a_Item);
	m_Mutex.unlock();

	// return the status
	return a_Result;
}

/**
* Add item to queue from an IRQ handler
*
* @param a_Item - item to add
* @return true if item added, false if queue full
*/
bool PacketQueue::putIrq(Packet* a_Item)
{
	bool a_Result = false;

	// check for room
	if (m_ProdCount < m_Count)
	{
		// copy the item/adjust the pointer/check for overflow
		*m_ProdWrIndex++ =*a_Item;
		if (m_ProdWrIndex >= m_Tail)
		{
			m_ProdWrIndex = m_Head;
		}

		// increment the count
		m_ProdCount++;
		m_NumPendingPuts++;

		// set the result to 0k
		a_Result = true;
	}

	// return the status
	return a_Result;
}

/**
* Update the consumer data to indicate that an entire group
* has been written so that it can be seen by the consumer
*/
void PacketQueue::producerCommit(void)
{
	m_Mutex.lock();
	m_ConWrIndex = m_ProdWrIndex;
	m_ConCount += m_NumPendingPuts;
	m_NumPendingPuts = 0;
	m_Mutex.unlock();
}

/**
* Get an item from the queue
*
* @param a_Item - pointer to the item to get
* @return true if item fetched, false if queue is empty
*/
bool PacketQueue::get(Packet** a_Item)
{
	bool a_Result;

	m_Mutex.lock();
	a_Result = getIrq(a_Item);
	m_Mutex.unlock();

	// return the status
	return a_Result;
}

/**
* Get an item from the queue in an IRQ handler
*
* @param a_Item - pointer to the item to get
* @return true if item fetched, false if queue is empty
*/
bool PacketQueue::getIrq(Packet** a_Item)
{
	bool a_Result = false;

	// check for room
	if (m_ConCount != 0)
	{
		// copy the item/adjust the pointer/check for overflow
		*a_Item = m_ConRdIndex++;
		if (m_ConRdIndex >= m_Tail)
		{
			m_ConRdIndex = m_Head;
		}

		// decrement the count
		m_ConCount--;
		m_NumPendingGets++;

		// set the result to 0k
		a_Result = true;
	}

	// return the status
	return a_Result;
}

/**
* Just retrieve the next packet without modifying
* queue data. Returns a pointer to the next packet
*
* @note This is bit complicated.
*  m_ProdRdIndex is the first packet of the group
*  m_ConRdIndex is the packet past the last packet of the group
*
* @param packet pointer to the current packet
* @return pointer to the next packet (possibly wrapped around)
*/
Packet* PacketQueue::retrieveNextMessage(Packet* a_Item)
{
	a_Item++;

	// Do we wrap on the size of the whole array
	if (a_Item >= m_Tail)
	{
		// The wrap and check to see if we wrap
		// base on the group
		a_Item = m_Head;
		if (a_Item >= m_ConRdIndex)
		{
			a_Item = m_ProdRdIndex;
		}
	}
	else if (m_ConRdIndex > m_ProdRdIndex)
	{
		// if the end index isn't wrapped
		// then just check if the group wraps
		if (a_Item >= m_ConRdIndex)
		{
			a_Item = m_ProdRdIndex;
		}
	}
	else if (a_Item < m_ProdRdIndex)
	{
		if (a_Item >= m_ConRdIndex)
		{
			a_Item = m_ProdRdIndex;
		}
	}
	// else - Here the Read Index has wrapped and the current index
	// hasn't so we are good and packet points to the correct item
	return a_Item;
}


/**
* Update the producer data to indicate that an entire group
*  is completed so that the slots are freed to write to
*/
void PacketQueue::consumerRelease(void)
{
	m_Mutex.lock();
	m_ProdRdIndex = m_ConRdIndex;
	m_ProdCount -= m_NumPendingGets;
	m_NumPendingGets = 0;
	m_Mutex.unlock();
}

/**
* Peek at the entry at the top of the queue
*
* @param [out] a_Item the item to copy the top queue item into
* @return the entry at the top of the queue
*/
bool PacketQueue::peek(Packet* a_Item)
{
	bool a_Result = false;

	// check for room
	if (m_ProdCount != 0)
	{
		// copy the item
		*a_Item = *m_ProdRdIndex;

		// set the result to 0k
		a_Result = true;
	}

	// return the status
	return a_Result;
}

