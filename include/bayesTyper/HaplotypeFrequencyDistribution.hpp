
/*
HaplotypeFrequencyDistribution.hpp - This file is part of BayesTyper (v1.1)


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


#ifndef __bayesTyper__HaplotypeFrequencyDistribution_hpp
#define __bayesTyper__HaplotypeFrequencyDistribution_hpp

#include "Utils.hpp"
#include "FrequencyDistribution.hpp"

class HaplotypeFrequencyDistribution {

	public:

		HaplotypeFrequencyDistribution(FrequencyDistribution *);
		~HaplotypeFrequencyDistribution();

		void reset();

		pair<bool, double> getElementFrequency(const ushort);
		void incrementObservationCount(const ushort);

		uint sumHaplotypeCount();
		uint sumMissingCount();

		void sampleFrequencies();

	private:

		FrequencyDistribution * frequency_distribution;

		uint sum_haplotype_count;
		uint sum_missing_count;
};

#endif	