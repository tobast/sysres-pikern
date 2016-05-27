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

	class Iterator {
		friend class HashTable<Key,Value,ToInt>;
		public:
		Iterator& operator++() {
			if(buck >= hashtbl->nbBuckets)
				return *this;
			bPos++;
			if(bPos >= hashtbl->buckets[buck].size()) {
				buck++;
				bPos=0;
			}
			return *this;
		}

		const Value& operator*() const {
			return hashtbl->buckets[buck][bPos].second;
		}
		const Key& key() const {
			return hashtbl->buckets[buck][bPos].first;
		}

		bool operator==(const Iterator& oth) const {
			return (buck == oth.buck && bPos == oth.bPos);
		}
		bool operator!=(const Iterator& oth) const {
			return !(*this == oth);
		}
		
		protected:
		Iterator(HashTable<Key,Value,ToInt>* hashtbl,
				unsigned buck, unsigned bPos) :
			hashtbl(hashtbl), buck(buck), bPos(bPos) {}

		private:
		HashTable<Key,Value,ToInt>* hashtbl;
		unsigned buck, bPos;
	};


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

	Iterator begin() {
		return Iterator(this, 0, 0);
	}
	Iterator end() {
		return Iterator(this, nbBuckets, 0);
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

