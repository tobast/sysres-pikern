#pragma once

#include "malloc.h"

template <typename T> class Queue {
public:
	class Empty {};

	Queue() : phySize(1), beg(0),end(0),nbElem(0) {
		data = (T*) malloc(sizeof(T)*1);
	}
	~Queue() {
		free(data);
	}

	void push(const T& v) {
		if(nbElem >= phySize) {
			realloc(2*phySize);
		}
		data[end] = v;
		nbElem++;
		end = (end+1)%phySize;
	}
	
	void push_front(const T& v) {
		if(nbElem >= phySize)
			realloc(2*phySize);
		beg = (beg-1+phySize)%phySize;
		data[beg] = v;
		nbElem++;
	}

	T pop() {
		if(nbElem == 0)
			throw Empty();
		T val = data[beg];
		beg = (beg+1)%phySize;
		nbElem--;
		if(phySize/3 > nbElem)
			realloc(phySize/2);

		return val;
	}

	T& front() {
		if(nbElem == 0)
			throw Empty();
		return data[beg];
	}

	unsigned size() const {
		return nbElem;
	}
	bool empty() const {
		return size() == 0;
	}
	unsigned capacity() const {
		return phySize;
	}

private://meth
	void realloc(unsigned nSize) {
		T* nData = (T*)malloc(sizeof(T)*nSize);
		unsigned nPos=0;
		for(unsigned pos=beg; nPos < nbElem; pos = (pos+1)%phySize) {
			nData[nPos] = data[pos];
			nPos++;
		}
		free(data);
		data = nData;
		beg=0;
		end=nbElem;
		phySize = nSize;
	}

private:
	T* data;
	unsigned phySize;
	unsigned beg;
	unsigned end;
	unsigned nbElem;
};

