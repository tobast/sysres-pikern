#pragma once

/**
 * Expendable array ("vector").
 **/

#include "malloc.h"

template <typename T> class ExpArray {
public:
	class OutOfBounds{};

	ExpArray() : nPos(0), phySize(2) {
		data = (T*)malloc(2*sizeof(T));
	}
	~ExpArray() {
		free(data);
	}

	T& operator[](unsigned pos) {
		if(inBounds(pos))
			return data[pos];
		throw OutOfBounds();
	}

	void operator=(const ExpArray<T>& oth) {
		nPos=0;
		realloc(oth.size());
		nPos=oth.size();
		for(unsigned pos=0; pos < oth.size(); pos++)
			data[pos] = oth[pos];
	}

	const T& operator[](unsigned pos) const {
		if(inBounds(pos))
			return data[pos];
		throw OutOfBounds();
	}
	void push_back(const T& val) {
		if(nPos >= phySize)
			realloc(2*phySize+1);
		data[nPos] = val;
		nPos++;
	}
	void pop_back() {
		if(nPos == 0)
			throw OutOfBounds();
		nPos--;
		if(nPos < phySize / 3)
			realloc(phySize/2);
	}
	void reserve(unsigned size) {
		if(phySize < size) {
			unsigned nSize = 1;
			while(nSize < size)
				nSize <<= 1;
			realloc(nSize);
		}
	}
	unsigned size() const {
		return nPos;
	}
	unsigned capacity() const {
		return phySize;
	}
	void clear() {
		nPos=0;
		realloc(2);
	}
	bool empty() const {
		return nPos == 0;
	}

private: //meth
	void realloc(unsigned nSize) {
		if(nSize <= 0)
			nSize=1;
		assert(nSize >= nPos, 0xf7);


		T* nData = (T*)malloc(nSize * sizeof(T));
		for(unsigned pos=0; pos < nPos; pos++)
			nData[pos] = data[pos];
		free(data);
		data = nData;
		phySize = nSize;
	}
	bool inBounds(unsigned pos) const {
		return pos < size();
	}

private:
	T* data;
	unsigned nPos;
	unsigned phySize;
};

