#pragma once

/**
 * Custom hash table implementation.
 **/

#include <cstdint>
#include "expArray.h"
#include "pair.h"

template <typename T> class StdToInt {
	public:
	uint32_t operator()(const T& v) const {
		return (uint32_t)v;
	}
};

template <typename Key, typename Value, class ToInt = StdToInt<Key> >
class HashTable {
public:
	class NotFound {};

	HashTable() {
		init();
	}
	HashTable(unsigned nbBuckets) {
		init(nbBuckets);
	}
	~HashTable() {
		for(unsigned buck=0; buck < nbBuckets; buck++)
			buckets[buck].~Bucket();
		free(buckets);
	}

	void insert(const Key& k, const Value& v) {
		unsigned bucket = hash(k);

		for(unsigned pos=0; pos < buckets[bucket].size(); pos++) {
			// If k is already bound, replace the binding
			if(buckets[bucket][pos].first == k) {
				buckets[bucket][pos].second = v;
				return;
			}
		}
		
		// Else, add it, potentially triggering a rehash
		Assoc assoc = Pair<Key,Value>(k,v);
		buckets[bucket].push_back(assoc);
		nbAssoc++;
		return;
		if(nbAssoc > nbBuckets*4/5)
			rehash(nbBuckets * 2);
	}

	const Value& find(const Key& k) const {
		unsigned buck = hash(k);
		for(unsigned pos=0; pos < buckets[buck].size(); pos++) {
			if(buckets[buck][pos].first == k)
				return buckets[buck][pos].second;
		}
		throw NotFound();
	};

	void init(unsigned nbBuck=7) {
		nbBuckets = nbBuck;
		nbAssoc=0;
		buckets = (Bucket*) malloc(nbBuckets*sizeof(Bucket));
		for(unsigned buck=0; buck < nbBuckets; buck++)
			buckets[buck].init();
	}

private: //meth
	unsigned hash(const Key& k) const {
		return (ToInt()(k)) % nbBuckets;
	}

	void rehash(unsigned nNbBuckets) {
		Bucket* nBuckets = (Bucket*) malloc(nNbBuckets * sizeof(Bucket));
		for(unsigned buck=0; buck < nNbBuckets; buck++)
			nBuckets[buck].init();
		unsigned prevBuckets = nbBuckets;
		nbBuckets = nNbBuckets;

		for(unsigned buck = 0; buck < prevBuckets; buck++) {
			for(unsigned asso = 0; asso < buckets[buck].size(); asso++) {
				Assoc& assoc = buckets[buck][asso];
				nBuckets[hash(assoc.first)].push_back(assoc);
			}
			buckets[buck].~Bucket();
		}
		free(buckets);
		buckets = nBuckets;
	}

private:
	typedef Pair<Key,Value> Assoc;
	typedef ExpArray<Assoc> Bucket;
	Bucket* buckets;
	unsigned nbBuckets, nbAssoc;
};

