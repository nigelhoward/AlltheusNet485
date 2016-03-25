/*
 *    2014-02-03  Brian Fletcher  <brian.jf.fletcher@gmail.com>
 *
 *      - added enqueue(), dequeue() and front().
 *
 *    2010-09-29  Efstathios Chatzikyriakidis  <contact@efxa.org>
 */ 
// Header file contains the header and the cpp

// All credit please to the above authors for the excellent QueueArray library for Arduino
// Details of the orginal library can be found here - http://playground.arduino.cc/Code/QueueArray
// Which I have chopped and modified to suit my needs.
// AllQueue Version 1.2 09-03-2016

#ifndef _ALLQUEUE_H
#define _ALLQUEUE_H

// Particle boards have a PLATFORM_ID Defined but Arduino boards don't. Below includes relevant header.
#ifdef PLATFORM_ID
#include "application.h" // This is Particle.io specific
#else
#include "Arduino.h"
#endif

// the definition of the queue class.
template<typename T>
class AllQueue {
  public:
    // Queue (constructor).
    AllQueue ();

	// initialize the size of the queue.
	void init(const int s);

    // clear the queue (destructor).
    ~AllQueue ();

    // add an item to the queue.
    bool enqueue (const T i);
    
    // remove an item from the queue.
    T dequeue ();

    // get the front of the queue.
    T front () const;

    // get an item from the queue.
    T peek () const;

    // check if the queue is empty.
    bool isEmpty () const;

    // get the number of items in the queue.
    int count () const;

    // check if the queue is full.
    bool isFull () const;

    // set the printer of the queue.
    void setPrinter (Print & p);

	// resize the size of the queue.
	bool resize(const int s);

	// Find a queue object by it's Id if it is an int
	int findByIntId(int);

	// Find a queue object by it's Id if it is a long
	int findByLongId(long);

	// Find a queue object by it's Id if it is a byte
	int findByByteId(byte);
	
	// Return an object by it's queue position
	T getByQueuePosition(int);
	
	// Delete by Long Id
	bool deleteByLongId(unsigned long Id);
	
	// Delete by Int Id
	bool deleteByIntId(unsigned int Id);
	
	// Delete by Byte Id
	bool deleteByByteId(byte Id);

	int resizeQueueBy = 1; // How many to add to existing size
	int maxQueueSize = 1; // Max queue size
	int queueOverflow = 0; // Queue had an overflow

	bool debug = false;

//private:

    // exit report method in case of error.
    void exit (const char * m) const;

    // led blinking method in case of error.
    // void blink () const;

    // the initial size of the queue.
    static const int initialSize = 1;

    // the pin number of the on-board led.
    static const int ledPin = 13;

    Print * printer; // the printer of the queue.
    T * contents;    // the array of the queue.

    int size;        // the size of the queue.
    int items;       // the number of items of the queue.

    int head;        // the head of the queue.
    int tail;        // the tail of the queue.

};

// queue (constructor).
template<typename T>
AllQueue<T>::AllQueue () {
  size = 0;       // set the size of queue to zero.
  items = 0;      // set the number of items of queue to zero.

  head = 0;       // set the head of the queue to zero.
  tail = 0;       // set the tail of the queue to zero.

  printer = NULL; // set the printer of queue to point nowhere.

}

// Initialize the size of the queue
template<typename T>
void AllQueue<T>::init(const int s) 
{
	// allocate enough memory for the array.

	contents = (T *)malloc(sizeof(T) * s);

	// set the initial size of the queue.
	size = s;
	maxQueueSize = s;

}


// clear the queue (destructor).
template<typename T>
AllQueue<T>::~AllQueue () {
  free (contents); // deallocate the array of the queue.

  contents = NULL; // set queue's array pointer to nowhere.
  printer = NULL;  // set the printer of queue to point nowhere.

  size = 0;        // set the size of queue to zero.
  items = 0;       // set the number of items of queue to zero.

  head = 0;        // set the head of the queue to zero.
  tail = 0;        // set the tail of the queue to zero.
}

template<typename T>
bool AllQueue<T>::resize (int resizeBy) {

  int s = size + resizeBy;


  if (s > maxQueueSize) 
  {
	  // Can't add any more so this enqueue will be discarded
	  queueOverflow++;
	  if (debug)
	  {
		  Serial.print("Queue overflow: Q=");
		  Serial.println(size);
	  }
	  return false;
  }

  if (debug)
  {
	  Serial.println();
	  Serial.print("Queue is :");
	  Serial.print(size);
	  Serial.print(" Resized by:");
	  Serial.print(resizeBy);
	  Serial.print(" to:");
	  Serial.println(s);

  }

  // allocate enough memory for the temporary array.
  T * temp = (T *) malloc (sizeof (T) * s);
  
  // copy the items from the old queue to the new one.
  for (int i = 0; i < items; i++)
    temp[i] = contents[(head + i) % size];

  // deallocate the old array of the queue.
  free (contents);

  // copy the pointer of the new queue.
  contents = temp;

  // set the head and tail of the new queue.
  head = 0; tail = items;

  // set the new size of the queue.
  size = s;

  return true;
}

// add an item to the queue.
template<typename T>
bool AllQueue<T>::enqueue (const T i) {
  
	if (debug)
	{
		Serial.println();
		Serial.print("Q:");
		Serial.print(items);
		Serial.print("/");
		Serial.print(size);
	}

	bool okToEnqueue = true;
	// check if the queue is full.
	if (isFull())
	{
		// try and increase size of array.
		okToEnqueue = resize(resizeQueueBy);
	}
	if (okToEnqueue)
	{
		// store the item to the array.
		contents[tail++] = i;

		// wrap-around index.
		if (tail == size) tail = 0;

		// increase the items.
		items++;

		return true;
	}
	return false;
}

// remove an item from the queue.
template<typename T>
T AllQueue<T>::dequeue () {
  // check if the queue is empty.
  if (isEmpty ())
    exit ("QUEUE: can't pop item from queue: queue is empty.");

  // fetch the item from the array.
  T item = contents[head++];

  // decrease the items.
  items--;

  // wrap-around index.
  if (head == size) head = 0;

  // shrink size of array if necessary.
  //if (!isEmpty () && (items <= size / 4))
  //  resize (size / 2);

  // return the item from the array.
  return item;
}

// get the front of the queue.
template<typename T>
T AllQueue<T>::front () const {
  // check if the queue is empty.
  if (isEmpty ())
    exit ("QUEUE: can't get the front item of queue: queue is empty.");
    
  // get the item from the array.
  return contents[head];
}

// get an item from the queue.
template<typename T>
T AllQueue<T>::peek () const {
  return front();
}

// check if the queue is empty.
template<typename T>
bool AllQueue<T>::isEmpty () const {
  return items == 0;
}

// check if the queue is full.
template<typename T>
bool AllQueue<T>::isFull () const {
  return items == size;
}

// get the number of items in the queue.
template<typename T>
int AllQueue<T>::count () const {
  return items;
}

// set the printer of the queue.
template<typename T>
void AllQueue<T>::setPrinter (Print & p) {
  printer = &p;
}

// Find an item by int T.Id
template<typename T>
int AllQueue<T>::findByIntId(int Id)
{
	for (int i = 0; i <= this->count(); i++)
	{
		T thing = contents[i];
		if (thing.Id == Id) return i;
	}
	return -9;
}

// Find an item by long T.Id
template<typename T>
int AllQueue<T>::findByLongId(long Id)
{
	for (int i = 0; i <= this->count(); i++)
	{
		T thing = contents[i];
		if (thing.Id == Id) return i;
	}
	return -9;
}

// Find an item by byte T.Id
template<typename T>
int AllQueue<T>::findByByteId(byte Id)
{
	for (int i = 0; i <= this->count(); i++)
	{
		T thing = contents[i];
		if (thing.Id == Id) return i;
	}
	return -9;
}

// Get an item by queue position
template<typename T>
T AllQueue<T>::getByQueuePosition(int position)
{
	T thing = contents[position];
	return thing;	
}

// Delete an item by LongIdId
template<typename T>
bool AllQueue<T>::deleteByLongId(unsigned long Id)
{	
	int countOfQueue = this->count();
	
	for (int i = 0; i < countOfQueue ; i++)
	{
		T thing = dequeue();
		if(thing.Id == Id)
		{
			return true;
		}
		else
		{
			enqueue(thing);
		}
	}
	
	return false;
}

// Delete an item by IntId
template<typename T>
bool AllQueue<T>::deleteByIntId(unsigned int Id)
{
	int countOfQueue = this->count();
	
	for (int i = 0; i < countOfQueue ; i++)
	{
		T thing = dequeue();
		if(thing.Id == Id)
		{
			return true;
		}
		else
		{
			enqueue(thing);
		}
	}
	
	return false;
}

// Delete an item by ByteId
template<typename T>
bool AllQueue<T>::deleteByByteId(byte Id)
{
	int countOfQueue = this->count();
	
	for (int i = 0; i < countOfQueue ; i++)
	{
		T thing = dequeue();
		if(thing.Id == Id)
		{
			return true;
		}
		else
		{
			enqueue(thing);
		}
	}
	
	return false;
}

template<typename T>
void AllQueue<T>::exit(const char * m) const {
	// print the message if there is a printer.
	if (printer)
		printer->println(m);

}

#endif // _ALLQUEUE_H
