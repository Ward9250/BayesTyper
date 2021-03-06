
/*
merge.cpp - This file is part of BayesTyper (v1.1)


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


#include <memory>
#include <algorithm>
#include <math.h>
#include <regex>
#include <set>

#include "JoiningString.hpp"
#include "Auxiliaries.hpp"

#include "merge.hpp"

namespace Merge {

	void merge(const vector<string> & in_vcf_filenames, const string & outfile_prefix) {

		assert(in_vcf_filenames.size() > 1);
		GenotypedVcfFileReader tmpl_vcf(in_vcf_filenames.front(), true);

		// Prepare output metadata
		VcfMetaData output_meta_data = tmpl_vcf.metaData();
		uint num_samples = tmpl_vcf.metaData().numSamples();

		vector<unique_ptr<GenotypedVcfFileReader> > in_vcfs;
		
		for (uint in_vcf_idx = 1; in_vcf_idx < in_vcf_filenames.size(); in_vcf_idx++) {

			in_vcfs.push_back(unique_ptr<GenotypedVcfFileReader> (new GenotypedVcfFileReader(in_vcf_filenames.at(in_vcf_idx), true)));
			num_samples += in_vcfs.back()->metaData().numSamples();

			for (auto & smpl_id : in_vcfs.back()->metaData().sampleIds()) {

				output_meta_data.addSample(smpl_id);
			}

			assert(tmpl_vcf.metaData().contigs() == in_vcfs.back()->metaData().contigs());
			assert(tmpl_vcf.metaData().infoDescriptors() == in_vcfs.back()->metaData().infoDescriptors());
			assert(tmpl_vcf.metaData().filterDescriptors() == in_vcfs.back()->metaData().filterDescriptors());
			assert(tmpl_vcf.metaData().formatDescriptors() == in_vcfs.back()->metaData().formatDescriptors());
		}

		assert(num_samples == output_meta_data.numSamples());

		cout << "[" << Utils::getLocalTime() << "] Running BayesTyperTools (" << BT_VERSION << ") merge on " << in_vcf_filenames.size() << " files containing " << num_samples << " samples in total ...\n" << endl;

		assert(output_meta_data.infoDescriptors().erase("HC"));

		VcfFileWriter output_vcf(outfile_prefix + ".vcf", output_meta_data, true);

		vector<string> var_value_assert_keys = {"VCS", "VCI", "VCGS", "VCGI", "AE", "ACO"};

        auto sample_ids = output_meta_data.sampleIds();

		uint num_variants = 0;
		uint cache_size = 10000;

		vector<Variant*> tmpl_var_cache(cache_size, nullptr);
		vector<vector<Variant*> > in_var_caches(in_vcfs.size(), vector<Variant*>(cache_size, nullptr));

		bool reached_last_var = false;

		while (!reached_last_var) {

			/*
				Fill cache
			*/
			for (uint cache_idx = 0; cache_idx < cache_size; cache_idx++) {

				reached_last_var = !tmpl_vcf.getNextVariant(&tmpl_var_cache.at(cache_idx));

				if (reached_last_var) {

					cache_size = cache_idx;
					tmpl_var_cache.resize(cache_size);
				}
			}

			for (uint in_vcf_idx = 0; in_vcf_idx < in_vcfs.size(); in_vcf_idx++) {

				for (uint cache_idx = 0; cache_idx < cache_size; cache_idx++) {

					assert(in_vcfs.at(in_vcf_idx)->getNextVariant(&in_var_caches.at(in_vcf_idx).at(cache_idx)));
				}
			}

			/*
				Merge vars in cache
			*/

			for (uint cache_idx = 0; cache_idx < cache_size; cache_idx++) {

				num_variants++;

				Variant * cur_tmpl_var = tmpl_var_cache.at(cache_idx);
				assert(cur_tmpl_var);

				set<ushort> alleles_not_covered;

				auto cur_tmpl_var_anc_values = cur_tmpl_var->info().getValue<string>("ANC");

				if (cur_tmpl_var_anc_values.second) {

					auto cur_tmpl_var_anc_values_split = Utils::splitString(cur_tmpl_var_anc_values.first, ',');

					for (auto &anc_value: cur_tmpl_var_anc_values_split) {

						alleles_not_covered.insert(stoi(anc_value));
					}
				}

				for (uint in_vcf_idx = 0; in_vcf_idx < in_vcfs.size(); in_vcf_idx++) {

					Variant * cur_in_var = in_var_caches.at(in_vcf_idx).at(cache_idx);
					assert(cur_in_var);

					assert(cur_tmpl_var->chrom() == cur_in_var->chrom());
					assert(cur_tmpl_var->pos() == cur_in_var->pos());
					assert(cur_tmpl_var->ids() == cur_in_var->ids());
					assert(cur_tmpl_var->numAlts() == cur_in_var->numAlts());
					assert(cur_tmpl_var->filters() == cur_in_var->filters());

					for (uint all_idx = 0; all_idx < cur_tmpl_var->numAlls(); all_idx++) {

						assert(cur_tmpl_var->allele(all_idx) == cur_in_var->allele(all_idx));
					}

					for (auto & var_value_assert_key : var_value_assert_keys) {

						assert(cur_tmpl_var->info().getValue(var_value_assert_key) == cur_in_var->info().getValue(var_value_assert_key));
					}

					if (cur_in_var->info().getValue("HRS").second) {

						cur_tmpl_var->info().addFlag("HRS");
					}

					auto cur_in_var_anc_values = cur_in_var->info().getValue<string>("ANC");

					if (cur_in_var_anc_values.second) {

						auto cur_in_var_anc_values_split = Utils::splitString(cur_in_var_anc_values.first, ',');

						for (auto &anc_value: cur_in_var_anc_values_split) {

							alleles_not_covered.insert(stoi(anc_value));
						}
					}

					for (auto & smpl_id : in_vcfs.at(in_vcf_idx)->metaData().sampleIds()) {

						cur_tmpl_var->addSample(smpl_id, cur_in_var->getSample(smpl_id));
					}

					delete cur_in_var;
				}

				if (!(alleles_not_covered.empty())) {

					JoiningString anc_elements(',');

					for (auto &allele: alleles_not_covered) {

						anc_elements.join(to_string(allele));
					}

					cur_tmpl_var->info().setValue<string>("ANC", anc_elements.str());
				}

				Auxiliaries::updateAlleleStatsAndCallProb(cur_tmpl_var);
			    
				output_vcf.write(cur_tmpl_var);
				delete cur_tmpl_var;

				if (num_variants % 100000 == 0) {

					cout << "[" << Utils::getLocalTime() << "] Merged " << num_variants << " variant(s)" << endl;
				}
			}

			tmpl_var_cache = vector<Variant*>(cache_size, nullptr);
			in_var_caches = vector<vector<Variant*> >(in_vcfs.size(), vector<Variant*>(cache_size, nullptr));
		}

		Variant * dummy_var;

		for (auto & in_vcf : in_vcfs) {

			assert(!in_vcf->getNextVariant(&dummy_var));
		}

		cout << "\n[" << Utils::getLocalTime() << "] Completed merge of " << num_variants << " variant(s)" << endl;
		cout << endl;
	}
}
