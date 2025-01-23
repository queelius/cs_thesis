#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "BitStream.h"
#include "BinaryIO.h"
#include "Huffman.h"
#include "CompactArray.h"
#include "PerfectHashFn.h"
#include "Entropy.h"
#include "PHFilter.h"
//#include "Psi.h"
//#include "PsiBuilder.h"
#include "TextProcessor.h"

void main()
{
    try
    {
        alex::text::StemmerStream stemmer;
        proc.readJson(std::ifstream("text_processor_config.json"));

        proc.writeStopWords(std::cout);
        //proc.setStemming(true);
        //proc.setWildCards(0);

        //proc.parse(std::ifstream("sample.txt"));
    }
    catch (const char* err)
    {
        std::cerr << err << std::endl;
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[ERROR] unknown exception" << std::endl;
    }

    exit(0);
}