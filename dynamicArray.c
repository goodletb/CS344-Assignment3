/***********************************************
** Program Filename: dynamicArray.c
** Author: Matthew Toro
** Date: 1-29-2017
** Description: Dynamic Array implementation for Assignment 2 CS 261
Includes assert.h, stdlib.h, and dynArray.h
** Input: ~
** Output: ~
************************************************/
#include <assert.h>
#include <stdlib.h>
#include "dynArray.h"

struct DynArr
{
	TYPE *data;		/* pointer to the data array */
	int size;		/* Number of elements in the array */
	int capacity;	/* capacity ofthe array */
};


/* ************************************************************************
	Dynamic Array Functions
************************************************************************ */


/*********************************************
** Function: initDynArr
** Description: Initialize (including allocation of data array) dynamic array.
** Parameters: pointer to the dynamic array, capacity of the dynamic array
** Pre-Condition: pointer should not be null
** Post-Condition: internal data array can hold cap elements, v->data is not null
** Return: ~
**********************************************/
void initDynArr(DynArr *v, int capacity)
{
	assert(capacity > 0);
	assert(v!= 0);
	v->data = (TYPE *) malloc(sizeof(TYPE) * capacity);
	assert(v->data != 0);
	v->size = 0;
	v->capacity = capacity;	
}


/*********************************************
** Function: newDynArr
** Description: Allocate and initialize dynamic array.
** Parameters: desired capacity for the dyn array
** Pre-Condition: ~
** Post-Condition: ~
** Return: a non-null pointer to a dynArr of cap capacity
and 0 elements in it.
**********************************************/
DynArr* newDynArr(int cap)
{
	assert(cap > 0);
	DynArr *r = (DynArr *)malloc(sizeof( DynArr));
	assert(r != 0);
	initDynArr(r,cap);
	return r;
}


/*********************************************
** Function: freeDynArr
** Description: Deallocate data array in dynamic array.
** Parameters: pointer to the dynamic array
** Pre-Condition: ~
** Post-Condition: d.data points to null, size and capacity are 0, 
the memory used by v->data is freed
** Return: ~
**********************************************/
void freeDynArr(DynArr *v)
{
	if(v->data != 0)
	{
		free(v->data); 	/* free the space on the heap */
		v->data = 0;   	/* make it point to null */
	}
	v->size = 0;
	v->capacity = 0;
}


/*********************************************
** Function: deleteDynArr
** Description: Deallocate data array and the dynamic array ure. 
** Parameters: pointer to the dynamic array
** Pre-Condition: ~
** Post-Condition: the memory used by v->data is freed, the memory used by d is freed
** Return: ~
**********************************************/
void deleteDynArr(DynArr *v)
{
	freeDynArr(v);
	free(v);
}


/*********************************************
** Function: _dynArrSetCapacity
** Description: Resizes the underlying array to be the size cap
** Parameters: pointer to the dynamic array, the new desired capacity
** Pre-Condition: pointer should not be null
** Post-Condition: v has capacity newCap
** Return: ~
**********************************************/
void _dynArrSetCapacity(DynArr *v, int newCap)
{
	int i;	
	TYPE *newArray = malloc(sizeof(TYPE) * newCap);
	assert(newArray != 0);
	
	for (i = 0; i < v->size; i++)
		newArray[i] = v->data[i];

	free(v->data);
	v->data = newArray;
	v->capacity = newCap;
}


/*********************************************
** Function: sizeDynArr
** Description: Get the size of the dynamic array
** Parameters: pointer to the dynamic array
** Pre-Condition: pointer should not be null
** Post-Condition: ~
** Return: the size of the dynamic array
**********************************************/
int sizeDynArr(DynArr *v)
{
	return v->size;
}


/*********************************************
** Function: addDynArr
** Description: Adds an element to the end of the dynamic array
** Parameters: pointer to the dynamic array, the value to add to the end of the dynamic array
** Pre-Condition: pointer should not be null
** Post-Condition: size increases by 1, if reached capacity, capacity is doubled,
val is in the last utilized position in the array
** Return: ~
**********************************************/
void addDynArr(DynArr *v, TYPE val)
{
	assert(v != 0);
	if (v->size >= v->capacity)
		_dynArrSetCapacity(v, 2 * v->capacity);

	v->data[v->size] = val;
	v->size++;
}


/*********************************************
** Function: getDynArr
** Description: Get an element from the dynamic array from a specified position
** Parameters: pointer to the dynamic array, integer index to get the element from
** Pre-Condition: pointer should not be null, v is not empty, 
pos < size of the dyn array and >= 0
** Post-Condition: no changes to the dyn Array
** Return: value stored at index pos
**********************************************/
TYPE getDynArr(DynArr *v, int pos)
{
	assert(v != 0);
	assert(isEmptyDynArr(v) == 0);
	assert(pos < v->size && pos >= 0);
	return v->data[pos];
}


/*********************************************
** Function: putDynArr
** Description: Put an item into the dynamic array at the specified location,
overwriting the element that was there
** Parameters: pointer to the dynamic array, the index to put the value into,
the value to insert
** Pre-Condition: pointer should not be null, v is not empty,
pos >= 0 and pos < size of the array
** Post-Condition: index pos contains new value, val
** Return: ~
**********************************************/
void putDynArr(DynArr *v, int pos, TYPE val)
{
	assert(v != 0);
	assert(isEmptyDynArr(v) == 0);
	assert(pos < v->size && pos >= 0);
	v->data[pos] = val;
}


/*********************************************
** Function: swapDynArr
** Description: Swap two specified elements in the dynamic array
** Parameters: pointer to the dynamic array, i,j the elements to be swapped
** Pre-Condition: pointer should not be null, v is not empty,
i, j >= 0 and i,j < size of the dynamic array
** Post-Condition: index i now holds the value at j and index j now holds the value at i
** Return: ~
**********************************************/
void swapDynArr(DynArr *v, int i, int  j)
{
	assert(v != 0);
	assert(isEmptyDynArr(v) == 0);
	assert(i < v->size && j < v->size);
	assert(i >= 0 && j >= 0);
	TYPE temp = v->data[i];
	v->data[i] = v->data[j];
	v->data[j] = temp;
}


/*********************************************
** Function: removeAtDynArr
** Description: Remove the element at the specified location from the array,
shifts other elements back one to fill the gap
** Parameters: pointer to the dynamic array, location of element to remove
** Pre-Condition: pointer should not be null, v is not empty,
idx < size and idx >= 0
** Post-Condition: the element at idx is removed, the elements past idx are moved back one
** Return: ~
**********************************************/
void removeAtDynArr(DynArr *v, int idx)
{
	int i;
	assert(v != 0);
	assert(isEmptyDynArr(v) == 0);
	assert(idx < v->size && idx >= 0);

	for (i = idx; i < v->size - 1; i++)
		v->data[i] = v->data[i + 1];

	v->size--;
}



/* ************************************************************************
	Stack Interface Functions
************************************************************************ */


/*********************************************
** Function: isEmptyDynArr
** Description: Returns boolean (encoded in an int) demonstrating whether or not the 
dynamic array stack has an item on it.
** Parameters: pointer to the dynamic array
** Pre-Condition: pointer should not be null
** Post-Condition: ~
** Return: 1 if empty, otherwise 0
**********************************************/
int isEmptyDynArr(DynArr *v)
{
	assert(v != 0);
	if (v->size == 0)
		return 1;
	else
		return 0;
}


/*********************************************
** Function: pushDynArr
** Description: Push an element onto the top of the stack
** Parameters: pointer to the dynamic array, the value to push onto the stack
** Pre-Condition: pointer should not be null
** Post-Condition: size increases by 1, if reached capacity, capacity is doubled,
val is on the top of the stack
** Return: ~
**********************************************/
void pushDynArr(DynArr *v, TYPE val)
{
	addDynArr(v, val);
}


/*********************************************
** Function: topDynArr
** Description: Returns the element at the top of the stack 
** Parameters: pointer to the dynamic array
** Pre-Condition: pointer should not be null, v is not empty
** Post-Condition: no changes to the stack
** Return: ~
**********************************************/
TYPE topDynArr(DynArr *v)
{
	return getDynArr(v, v->size - 1);
}


/*********************************************
** Function: popDynArr
** Description: Removes the element on top of the stack 
** Parameters: pointer to the dynamic array
** Pre-Condition: pointer should not be null, v is not empty
** Post-Condition: size is decremented by 1, the top has been removed
** Return: ~
**********************************************/
void popDynArr(DynArr *v)
{
	removeAtDynArr(v, v->size - 1);
}

/* ************************************************************************
	Bag Interface Functions
************************************************************************ */


/*********************************************
** Function: containsDynArr
** Description: Returns boolean (encoded as an int) demonstrating whether or not
	the specified value is in the collection
	true = 1
	false = 0
UPDATED: returns index position if found or -1 if not found
** Parameters: pointer to the dynamic array, the value to look for in the bag
** Pre-Condition: pointer should not be null, v is not empty
** Post-Condition: no changes to the bag
** Return: ~
**********************************************/
int containsDynArr(DynArr *v, TYPE val)
{
	int i;
	assert(v != 0);
	assert(isEmptyDynArr(v) == 0);
	for (i = 0; i < v->size; i++)
		if (EQ(v->data[i], val))
			return i;
	return -1;
}


/*********************************************
** Function: removeDynArr
** Description: Removes the first occurrence of the specified value from the collection
if it occurs
** Parameters: pointer to the dynamic array, the value to remove from the array
** Pre-Condition: pointer should not be null, v is not empty
** Post-Condition: val has been removed, size of the bag is reduced by 1
** Return: ~
**********************************************/
void removeDynArr(DynArr *v, TYPE val)
{
	int i;
	for (i = 0; i < v->size; i++)
		if (EQ(v->data[i], val))
		{
			removeAtDynArr(v, i);
			return;
		}
}
