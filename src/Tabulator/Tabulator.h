#ifndef __TABULATOR_H__
#define __TABULATOR_H__

#include "FileSystem.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <set>

namespace alex { namespace corpus
{
    class Tabulator
    {
    public:
        Tabulator() {};

        void tabulate(const std::vector<std::string>& dirs, unsigned int minTokens, unsigned int maxTokens)
        {
            this->minTokens = minTokens;
            this->maxTokens = maxTokens;
            std::vector<boost::filesystem::path> files;
            for (const auto& dir : dirs)
            {
                //std::cout << "processing: " << dir << std::endl;
                process(dir);
            }
        }

        struct Record
        {
            std::string type;

            int min_query_terms;
            int max_query_terms;
            int min_tokens_term;
            int max_tokens_term;

            int min_tokens_doc;
            int max_tokens_doc;

            int corpus_size;

            int loc_uncertainty;
            int si_index_size;
            int build_time;
            int load_time;

            int obfuscations;
            int num_docs;
            int num_secrets;

            double mindist_lag = -1;
            double mindist_map = -1;

            double bm25_lag = -1;
            double bm25_map = -1;

            double contains_lag = -1;
            double recall = -1;
            double precision = -1;

            double fp_rate;

            void print(std::ostream& outs, bool csv = true)
            {
                if (csv)
                {
                    //type, num_docs, corpus_size, min_tokens_doc, max_tokens_doc, min_terms_query, max_terms_query, min_tokens_term, max_tokens_term, secrets, obfuscations, loc_uncertainty, fp_rate, index_size, build_time, load_time, contains_lag, precision, recall, bm25_lag, bm25_map, min_dist_lag, min_dist_map
                    outs << type<< ',' <<num_docs<< ',' <<corpus_size<< ',' <<min_tokens_doc<< ',' <<max_tokens_doc<< ',' <<min_query_terms<< ',' <<max_query_terms<< ',' <<min_tokens_term<< ',' <<max_tokens_term<< ',' <<num_secrets<< ',' <<obfuscations<< ',' <<loc_uncertainty<< ',' <<fp_rate << ','
                        << si_index_size<< ',' <<build_time<< ',' <<load_time<< ',' <<contains_lag<< ',' <<precision<< ',' <<recall<< ',' <<bm25_lag<< ',' <<bm25_map<< ',' <<mindist_lag<< ',' <<mindist_map
                        << std::endl;
                }
                else
                {
                    outs << "type: " << type << std::endl;
                    outs << "num_secrets: " << num_secrets << std::endl;
                    outs << "min_query_terms: " << min_query_terms << std::endl;
                    outs << "max_query_terms: " << max_query_terms << std::endl;
                    outs << "min_tokens_term: " << min_tokens_term << std::endl;
                    outs << "max_tokens_term: " << max_tokens_term << std::endl;
                    outs << "min_tokens_doc: " << min_tokens_doc << std::endl;
                    outs << "max_tokens_doc: " << max_tokens_doc << std::endl;
                    outs << "obfuscations: " << obfuscations << std::endl;
                    outs << "corpus_size: " << corpus_size << std::endl;
                    outs << "loc_uncertainty: " << loc_uncertainty << std::endl;
                    outs << "si_index_size: " << si_index_size << std::endl;
                    outs << "build_time: " << build_time << std::endl;
                    outs << "load_time: " << load_time << std::endl;
                    outs << "num_docs: " << num_docs << std::endl;
                    outs << "mindist_lag: " << mindist_lag << std::endl;
                    outs << "mindist_map: " << mindist_map << std::endl;
                    outs << "bm25_lag: " << bm25_lag << std::endl;
                    outs << "bm25_map: " << bm25_map << std::endl;
                    outs << "contains_lag: " << contains_lag << std::endl;
                    outs << "recall: " << recall << std::endl;
                    outs << "precision: " << precision << std::endl;
                    outs << "fp_rate: " << fp_rate << std::endl;
                    outs << "---\n" << std::endl;
                }
            };
        };

        std::vector<Record> records;

        bool processMakerLog(std::string type, Record& rec, boost::filesystem::path p)
        {
            std::string ignore;
            p += p.preferred_separator;
            p += "corpus_indexed";
            p += p.preferred_separator;
            p += "maker_" + type + ".log";

            if (!boost::filesystem::exists(p))
                return false;

            std::ifstream infile(p.string());
            infile >> ignore;
            infile >> rec.fp_rate;
            
            infile >> ignore;
            while (ignore != type)
                infile >> ignore;

            infile >> ignore >> rec.si_index_size;
            infile >> ignore >> rec.build_time;
            infile >> ignore >> rec.num_docs;
            
            infile >> ignore;
            while (ignore != "load_time")
                infile >> ignore;

            infile >> rec.load_time;

            return true;
        }

        void process(const boost::filesystem::path& p)
        {
            //std::cout << p << std::endl;
            if (boost::filesystem::exists(p))
            {
                auto types = {"bsib", "psib", "psim", "psip", "psif"};

                if (boost::filesystem::is_regular_file(p) && alex::str::hasMatch(p.filename().string(), "results"))
                {
                    auto tmp = p.parent_path();
                    tmp += p.preferred_separator;
                    tmp += "corpus_indexed";
                    tmp += p.preferred_separator;
                    tmp += "maker.log";

                    if (boost::filesystem::exists(tmp))
                    {
                        std::ifstream makerStream(tmp.string());
                        std::string ignore;
                        std::getline(makerStream, ignore);
                        makerStream >> ignore;
                        int corpusSize;
                        makerStream >> corpusSize;
                        makerStream >> ignore >> ignore >> ignore;
                    
                        int locUncertainty;
                        makerStream >> locUncertainty;

                        makerStream >> ignore;
                        while (ignore != "secrets")
                            makerStream >> ignore;

                        int numSecrets;
                        makerStream >> numSecrets;
                    
                        makerStream.close();

                        if (boost::filesystem::exists(tmp))
                        {
                            for (auto t : types)
                            {
                                Record rec;
                                rec.max_tokens_doc = maxTokens;
                                rec.min_tokens_doc = minTokens;

                                if (!processMakerLog(t, rec, p.parent_path()))
                                    continue;

                                std::string ignore;
                                std::ifstream results(p.string());

                                rec.corpus_size = corpusSize;
                                rec.type = t;
                                rec.loc_uncertainty = locUncertainty;
                                rec.num_secrets = numSecrets;
                                results >> ignore;
                                results >> rec.min_query_terms;
                                results >> ignore;
                                results >> rec.max_query_terms;
                                results >> ignore;
                                results >> rec.min_tokens_term;
                                results >> ignore;
                                results >> rec.max_tokens_term;
                                results >> ignore;
                                results >> rec.obfuscations;

                                results >> ignore;
                                if (ignore == "mindist")
                                {
                                    results >> ignore;
                                    while (ignore == "label")
                                    {
                                        results >> ignore;
                                        if (ignore == t)
                                        {
                                            results >> ignore >> rec.mindist_lag;
                                            results >> ignore >> rec.mindist_map;
                                        }
                                        else
                                        {
                                            results >> ignore >> ignore >> ignore >> ignore;
                                        }
                                        results >> ignore;
                                    }
                                }
                                if (ignore == "bm25")
                                {
                                    results >> ignore;
                                    while (ignore == "label")
                                    {
                                        results >> ignore;
                                        if (ignore == t)
                                        {
                                            results >> ignore >> rec.bm25_lag;
                                            results >> ignore >> rec.bm25_map;
                                        }
                                        else
                                        {
                                            results >> ignore >> ignore >> ignore >> ignore;
                                        }
                                        results >> ignore;
                                    }
                                }
                                if (ignore == "recall_precision")
                                {
                                    results >> ignore;
                                    while (ignore == "label")
                                    {
                                        results >> ignore;
                                        if (ignore == t)
                                        {
                                            results >> ignore >> rec.contains_lag;
                                            results >> ignore >> rec.precision;
                                            results >> ignore >> rec.recall;
                                        }
                                        else
                                        {
                                            results >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore;
                                        }
                                        results >> ignore;
                                    }
                                }

                                rec.print(std::cout);
                            }
                        }
                    }
                }
                else if (boost::filesystem::is_directory(p))
                {
                    for (auto it = boost::filesystem::directory_iterator(p), it_end = boost::filesystem::directory_iterator(); it != it_end; ++it)
                    {
                        process(*it);
                    }
                }
            }
        }
        private:
            unsigned int minTokens, maxTokens;

    };
}}

#endif