
/*
HybridHash.hpp - This file is part of BayesTyper (v0.9)


The MIT License (MIT)

Copyright (c) 2016 Jonas Andreas Sibbesen and Lasse Maretty

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#ifndef __bayesTyper__HybridHash_hpp
#define __bayesTyper__HybridHash_hpp

#include <vector>
#include <bitset>
#include <mutex>
#include <memory>
#include <unordered_map>

#include "LinearMap.hpp"
#include "BitsetCompare.hpp"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

template<typename ValueType, uchar root_hash_size, uchar leaf_hash_size> class HybridHashIterator;

template<typename ValueType, uchar root_hash_size, uchar leaf_hash_size>
class HybridHash {
	
	friend class HybridHashIterator<ValueType, root_hash_size, leaf_hash_size>;
	
	public:

		typedef HybridHashIterator<ValueType, root_hash_size, leaf_hash_size> iterator;

		HybridHash();
		~HybridHash();
		std::pair<iterator, bool> insert(const std::bitset<root_hash_size + leaf_hash_size> &, ValueType);
		iterator find(const std::bitset<root_hash_size + leaf_hash_size> &);
		iterator erase(iterator);
		iterator begin();
		iterator end();

		void writeBucketSizeDistribution(std::string);

	protected:
		
		ulong bitsubsetToUlong(const std::bitset<root_hash_size + leaf_hash_size> &, uchar, uchar);
		ulong bitsubsetToUlongReverse(const std::bitset<root_hash_size + leaf_hash_size> &, uchar, uchar);
		ulong bitsubsetToUlongSplit(const std::bitset<root_hash_size + leaf_hash_size> &, uchar, uchar, uchar, uchar);

		std::bitset<leaf_hash_size> bitsetToLeafBitsubset(const std::bitset<root_hash_size + leaf_hash_size> &, uchar, uchar);

		std::vector<ulong> root_hash_bit_values;

		typedef LinearMap<std::bitset<leaf_hash_size>, ValueType, BitsetLess<leaf_hash_size> > LeafHash;
		// typedef unordered_map<std::bitset<leaf_hash_size>, ValueType> LeafHash;

		std::vector<LeafHash*> root_hash;
};

template<typename ValueType, uchar root_hash_size, uchar leaf_hash_size>
class ThreadedHybridHash : public HybridHash<ValueType,root_hash_size,leaf_hash_size> {
	
	public:

		ThreadedHybridHash(uint);
		std::pair<std::unique_lock<std::mutex>, std::unique_lock<std::mutex> > lockKeyPair(const std::bitset<root_hash_size + leaf_hash_size> &, const std::bitset<root_hash_size + leaf_hash_size> &);
		std::unique_lock<std::mutex> lockKey(const std::bitset<root_hash_size + leaf_hash_size> &);

	private:

		std::mutex * getKeyMutex(const std::bitset<root_hash_size + leaf_hash_size> &);
		
		uint num_lock_bins;
		double hash_scaling_factor;
		std::vector<std::mutex> mutexes;
};


template<typename ValueType, uchar root_hash_size, uchar leaf_hash_size> 
class HybridHashIterator {

	friend class HybridHash<ValueType, root_hash_size, leaf_hash_size>;

	public:
		
		typedef typename HybridHash<ValueType, root_hash_size, leaf_hash_size>::LeafHash LeafHash;
		typedef typename std::vector<LeafHash*> RootHash;

		HybridHashIterator(HybridHash<ValueType,root_hash_size,leaf_hash_size> *, typename RootHash::iterator, typename LeafHash::iterator);
		HybridHashIterator(const HybridHashIterator &);
		HybridHashIterator & operator=(HybridHashIterator);
		ValueType & operator*();
		HybridHashIterator & operator++();
		HybridHashIterator operator++(int);
		bool operator==(HybridHashIterator<ValueType,root_hash_size,leaf_hash_size> const &);
		bool operator!=(HybridHashIterator<ValueType,root_hash_size,leaf_hash_size> const &);

	private:

		void swap(HybridHashIterator & first, HybridHashIterator & second);

		HybridHash<ValueType, root_hash_size, leaf_hash_size> * hybrid_hash;
		typename RootHash::iterator root_element_iter;
		typename LeafHash::iterator leaf_element_iter;  
};

#include "HybridHash.tpp"

#endif