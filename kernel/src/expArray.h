#pragma once

/**
 * Expandable array ("vector").
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
	const T& operator[](unsigned pos) const;
	void push_back(const T& val) {
		if(nPos >= phySize)
			realloc(2*phySize);
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
		if(phySize < size)
			realloc(size);
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
		T* nData = (T*)malloc(nSize * sizeof(T));
		for(unsigned pos=0; pos < nPos; pos++)
			nData[pos] = data[pos];
		free(data);
		data = nData;
		phySize = nSize;
	}
	bool inBounds(unsigned pos) {
		return pos >= 0 && pos < size();
	}

private:
	T* data;
	unsigned nPos;
	unsigned phySize;
};

