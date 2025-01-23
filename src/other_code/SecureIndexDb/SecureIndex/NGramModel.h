#ifndef __N_GRAM_MODEL_H__
#define __N_GRAM_MODEL_H__

#include <map>
#include <algorithm>
#include <string>
#include <ostream>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <sstream>
#include <functional>
#include "BinaryIO.h"
#include "Str.h"
#include <fstream>
#include "FileSystem.h"
#include "Entropy.h"

namespace alex { namespace ngrams
{
    class NGramModel;

    std::vector<std::string> tokenGenerator(const NGramModel& model, size_t numTokens);
    void parse(NGramModel& model, const std::vector<boost::filesystem::path>& dirs,
        const std::string& filePattern, size_t nGramSize, size_t recursionDepth,
        bool ignorePunctuation = true);

    class NGramModel
    {
    public:
        NGramModel()
        {
            _gramSize = 0;
            root = Node { 0, 0 };
            invTable.push_back("<DUMMY_NODE>");
        };

        void dump(std::ostream& os)
        {
            dumpHelper(root, 0, os);
        };

        void parse(std::istream& inStream, size_t nGramSize, bool ignorePunctuation = true)
        {
            std::ostringstream os;
            os << inStream.rdbuf();
            parse(std::move(os.str()), nGramSize, ignorePunctuation);
        };

        void parse(std::string s, size_t nGramSize, bool ignorePunctuation = true)
        {
            const auto& tokens = tokenize(std::move(s), nGramSize, ignorePunctuation);
            std::vector<std::string> ngram(nGramSize);
            
            for (size_t i = 0; i < tokens.size() - nGramSize + 1; ++i)
            {
                for (size_t j = i; j < i + nGramSize; ++j)
                    ngram[j - i] = tokens[j];
                addExample(ngram);
            }
        };

        struct Node
        {
            float p;
            uint32_t id;
            std::vector<Node> child;

            bool operator<(const Node& other) const { return id < other.id; };
        };

        size_t gramSize() const
        {
            return _gramSize;
        };

        size_t numUnigrams() const { return table.size(); };

        void addExample(const std::vector<std::string>& example)
        {
            std::vector<uint32_t> ids;
            ids.reserve(example.size());
            for (const auto& e : example)
            {
                if (table.count(e) == 0)
                {
                    const uint32_t id = table.size() + 1;
                    invTable.push_back(e);
                    table[e] = id;
                }

                ids.push_back(table[e]);
            }

            _gramSize = std::max(_gramSize, ids.size());
            addExampleHelper(root, ids, 0);
        };

        std::vector<Node> getUnconditional() const
        {
            return root.child;
        };

        std::vector<Node> getConditional(const std::vector<std::string>& given) const
        {
            std::vector<uint32_t> ids;
            ids.reserve(given.size());
            for (const auto& e : given)
            {
                if (table.count(e) == 0)
                    return std::vector<Node>();
                ids.push_back(table.at(e));
            }
            return getConditionalHelper(root, ids, 0);
        };

        void makePdf()
        {
            makePdfHelper(root);
        };

        void makeCdf()
        {
            makeCdfHelper(root);
        };

        // faster than trigramLI (linear interpolated) model. trigram LI is good for
        // sparse data. this works well when big data is provided. it also doesn't
        // assume anything about how what is given -- any arbitrary ngram can be given,
        // from nothing (in which case it will behave as a unigram model) to an n-gram,
        // n is any positive integer.
        std::function<std::string (float p, std::vector<std::string>)> inverseCdfFn() const
        {
            return [this](float p, std::vector<std::string> given) -> std::string
            {
                while (given.size() > this->gramSize())
                    given.erase(given.begin());

                std::vector<Node> results;
                do
                {
                    if (given.empty())
                    {
                        const auto& x = this->getUnconditional();
                        return this->invTable.at(NGramModel::inverseFind(x, 0, x.size(), p));
                    }
                    results = this->getConditional(given);
                    if (results.empty())
                        given.erase(given.begin());

                } while (results.empty());

                return this->invTable.at(
                    results[NGramModel::inverseFind(results, 0, results.size(), p)].id);
            };
        };

        // slow, especially for large models.

        // assumes ngram model is in pdf form
        std::function<std::string (float p, std::string, std::string)> trigramLIFn(float w1, float w2, float w3) const
        {
            return [w1, w2, w3, this](float p, std::string x1, std::string x2) -> std::string
            {
                auto& unigrams = this->getUnconditional();
                auto& bigrams = this->getConditional({x2});
                auto& trigrams = this->getConditional({x1, x2});
                std::vector<float> cdf(unigrams.size() + 2);

                cdf[0] = 0;
                for (auto& x : unigrams)
                    cdf[x.id] = w1 * x.p;

                for (auto& x : bigrams)
                    cdf[x.id] += w2 * x.p;

                for (auto& x : trigrams)
                    cdf[x.id] += w3 * x.p;

                float total = 0;
                for (auto& x : cdf)
                {
                    total += x;
                    x = total;
                }

                for (auto& x : cdf)
                    x /= total;

                cdf[cdf.size()-1] = 1;
                int idx = NGramModel::inverseFind(cdf, 0, cdf.size(), p);
                return this->invTable.at(idx);
            };
        };

        std::function<float (const std::string)> getConditionalFn(const std::vector<std::string>& given) const
        {
            auto ns = getConditional(given);
            return [ns, this](const std::string& word) -> float
            {
                uint32_t id = this->table.at(word);
                const auto newNode = Node{0, id};
                const auto i = std::lower_bound(ns.begin(), ns.end(), newNode);
                return (i == ns.end() || i->id != id ? 0 : i->p);
            };
        };

        std::function<float (const std::string&)> getUnconditionalFn() const
        {
            auto ns = getUnconditional();
            return [ns, this](const std::string& word) -> float
            {
                uint32_t id = this->table.at(word);
                const auto newNode = Node{0, id};
                const auto i = std::lower_bound(ns.begin(), ns.end(), newNode);
                return (i == ns.end() || i->id != id ? 0 : i->p);
            };
        };

        void read(std::istream& file)
        {
            table.clear();
            invTable.clear();

            invTable.push_back("<DUMMY_NODE>");
            readHelper(file, root, -1);
        };

        void write(std::ostream& os) const
        {

            writeHelper(root, 0, os);
        };

        void readBinary(std::istream& file)
        {
            table.clear();
            invTable.clear();

            uint32_t n = alex::io::binary::readVarU32(file);
            invTable.resize(n+1);
            invTable[0] = "<DUMMY_NODE>";
            _gramSize = alex::io::binary::readVarU32(file);

            // make table/invtable
            for (uint32_t i = 1; i <= n; ++i)
            {
                invTable[i] = alex::io::binary::readString(file);
                table[invTable[i]] = i;
            }

            readBinaryHelper(file, root);
        };

        void writeBinary(std::ostream& file)
        {
            alex::io::binary::writeVarU32(file, (uint32_t)invTable.size());
            alex::io::binary::writeVarU32(file, (uint32_t)_gramSize);

            for (size_t i = 1; i <= invTable.size(); ++i)
                alex::io::binary::writeString(file, invTable[i]);

            writeBinaryHelper(file, root);
        };

        std::string getToken(uint32_t id) const
        {
            if (id > invTable.size())
                throw std::exception("Id Not Found");

            return invTable[id];
        };

    private:
        // function assumes makeCdf() has been called
        static int inverseFind(const std::vector<Node>& nodes, int low, int high, float u)
        {
            if (low > high)
                return (int)nodes.size() - 1;

            int mid = (low + high) / 2;
            if (u <= nodes[mid].p)
                return inverseFind(nodes, low, mid - 1, u);
            else if (u > nodes[mid + 1].p)
                return inverseFind(nodes, mid + 1, high, u);
            else
                return mid + 1;
        };

        static int inverseFind(const std::vector<float>& ns, int low, int high, float u)
        {
            if (low > high)
                return (int)ns.size() - 1;

            int mid = (low + high) / 2;
            if (u <= ns[mid])
                return inverseFind(ns, low, mid - 1, u);
            else if (u > ns[mid + 1])
                return inverseFind(ns, mid + 1, high, u);
            else
                return mid + 1;
        };

        void makeCdfHelper(Node& cur)
        {
            float sum = 0;
            for (auto& child : cur.child)
            {
                sum += child.p;
                child.p = sum;
            }
            for (auto& child : cur.child)
            {
                child.p /= sum;
                makeCdfHelper(child);
            }
        };

        void makePdfHelper(Node& cur)
        {
            float sum = 0;
            for (const auto& child : cur.child)
                sum += child.p;
            for (auto& child : cur.child)
            {
                child.p /= sum;
                makePdfHelper(child);
            }
        };

        std::vector<Node> getConditionalHelper(const Node& cur, const std::vector<uint32_t>& ngram, size_t index) const
        {
            if (index == ngram.size())
                return cur.child;

            const auto& ns = cur.child;
            const auto newNode = Node { 0, ngram[index] };
            const auto i = std::lower_bound(ns.begin(), ns.end(), newNode);

            if (i == ns.end() || i->id != ngram[index])
                return std::vector<Node>();
            else
                return getConditionalHelper(*i, ngram, index + 1);
        };

        void addExampleHelper(Node& parent, const std::vector<uint32_t>& ngram, size_t ngramIdx)
        {
            if (ngramIdx < ngram.size())
                addExampleHelper(addChild(parent, ngram[ngramIdx]), ngram, ngramIdx + 1);
        };

        Node& addChild(Node& parent, uint32_t id, float p = 1)
        {
            auto& ns = parent.child;
            const auto& newNode = Node { p, id };
            auto i = std::lower_bound(ns.begin(), ns.end(), newNode);

            if (i != ns.end() && i->id == id)
            {
                ++i->p;
                return *i;
            }
            else
                return *(ns.insert(i, newNode));
        };

        void writeHelper(const Node& n, int depth, std::ostream& os) const
        {
            if (depth > 0)
            {
                for (int i = 1; i < depth; ++i)
                    os.put('\t');
                os << invTable.at(n.id);
                os.put('\t');
                os << n.p;
                os.put('\n');
            }

            for (const auto& c : n.child)
                writeHelper(c, depth + 1, os);
        };

        void dumpHelper(Node& n, int depth, std::ostream& os)
        {
            if (depth > 0)
            {
                for (int i = 1; i < depth; ++i)
                    os << "\t";
                os << invTable.at(n.id) << " => " << n.id << " : " << n.p << std::endl;
            }

            for (auto& c : n.child)
                dumpHelper(c, depth + 1, os);
        };

        void writeBinaryHelper(std::ostream& file, Node& cur)
        {
            alex::io::binary::writeVarU32(file, (uint32_t)cur.child.size());
            if (cur.child.empty())
                return;

            for (auto& child : cur.child)
            {
                alex::io::binary::writeVarU32(file, (uint32_t)child.id);
                alex::io::binary::write754F(file, child.p);
                writeBinaryHelper(file, child);
            }
        };

        void readBinaryHelper(std::istream& file, Node& cur)
        {
            uint32_t n = alex::io::binary::readVarU32(file);
            if (n == 0)
                return;

            cur.child.reserve(n);
            for (uint32_t i = 0; i < n; ++i)
            {
                uint32_t id = alex::io::binary::readVarU32(file);
                float p = alex::io::binary::read754F(file);
                readBinaryHelper(file, addChild(cur, id, p));
            }
        };    

        void readHelper(std::istream& file, Node& parent, int parentDepth)
        {
            while (file)
            {
                int curDepth = 0;
                while (file.peek() == '\r' || file.peek() == '\n')
                    file.get();
                while (file.peek() == '\t')
                {
                    file.get();
                    ++curDepth;
                }
                if (curDepth <= parentDepth)
                {
                    for (int i = 0; i < curDepth; ++i)
                        file.putback('\t');
                    break;
                }

                int tmp = curDepth;
                curDepth = parentDepth+1;

                // grab next one
                std::string token; file >> token;
                float p; file >> p;

                // is token in table already?
                if (table.count(token) == 0)
                {
                    uint32_t id = table.size() + 1;
                    table[token] = id;
                    invTable.push_back(token);
                }

                uint32_t id = table[token];
                auto& n = addChild(parent, id, p);

                _gramSize = std::max(_gramSize, (size_t)curDepth+1);
                readHelper(file, n, curDepth);
            }
        };    

        Node root;
        size_t _gramSize;

        std::unordered_map<std::string, uint32_t> table;
        std::vector<std::string> invTable;

        // faster version, but not as fancy
        virtual std::vector<std::string> tokenize(const std::string& message, size_t nGramSize, bool ignorePunctuation)
        {
            std::vector<std::string> tokens;
            for (size_t i = 1; i < nGramSize; ++i)
                tokens.push_back("<START>");
            
            std::string token;
            for (size_t i = 0; i < message.size(); ++i)
            {
                char c = message[i];
                if (isalnum(c))
                {
                    c = tolower(c);
                    token += c;
                }
                else if (c == '\'')
                    continue;
                else
                {
                    if (!token.empty())
                    {
                        tokens.push_back(std::move(token));
                        token = "";
                    }
                    
                    if (!ignorePunctuation)
                    {
                        if (c == '.' || c == ',' || c == '!' || c == '?' || c == ';' || c == ':' || c == '-')
                            tokens.push_back(std::string() + c);
                    }
                }
            }

            for (size_t i = 1; i < nGramSize; ++i)
                tokens.push_back("<END>");

            return tokens;
        };

        // this is a slow, but may be acceptable
        virtual std::vector<std::string> tokenize2(std::string message, size_t nGramSize)
        {
            message = alex::str::lower(std::move(message));

            for (size_t i = 1; i < nGramSize; ++i)
                message = "<START> " + std::move(message) + " <END>";
            
            // message = alex::str::replace(std::move(message), "(https?:\\/\\/)?([\\da-z\\.-]+)\\.([a-z\\.]{2,6})([\\/\\w \\.-]*)*\\/?", "<URL>");
            message = alex::str::replace(std::move(message), "[a-z0-9]+@[a-z0-9]+\\.[a-z]{2,4}", "<EMAIL>");
            message = alex::str::replace(std::move(message), "([0-1]?[0-9]|[2][0-3]):([0-5][0-9])(am|pm)?", "<TIME>");
            message = alex::str::replace(std::move(message), "\\$(\\d{1,3}(\\,\\d{3})*|(\\d+))(\\.\\d{2})?", "<CURRENCY>");
            //message = alex::str::replace(std::move(message), "\\b#?([a-f0-9]{6}|[a-f0-9]{3})\\b", "<HEX>");
            message = alex::str::replace(std::move(message), "(1[0-2]|0?[1-9])/(3[01]|[12][0-9]|0?[1-9])/(?:[0-9]{2})?[0-9]{2}", "<DATE>");

            message = alex::str::replace(std::move(message), "(\\\"|')", "");
            message = alex::str::replace(std::move(message), "\\b[-+]?[0-9]*\\.[0-9]+\\b", "<DECIMAL>");
            message = alex::str::replace(std::move(message), "\\b[-+]?[0-9]+\\b", "<INTEGER>");

            message = alex::str::replace(std::move(message), "--+", "<DASHES>");           
            message = alex::str::replace(std::move(message), "(\\.\\s*)+", "<PERIOD>");
            message = alex::str::replace(std::move(message), "(\\!\\s*)+", "<EXCLAMATION>");
            message = alex::str::replace(std::move(message), "(\\:\\s*)+", "<COLON>");
            message = alex::str::replace(std::move(message), "(\\;\\s*)+", "<SEMICOLON>");
            message = alex::str::replace(std::move(message), "(\\?\\s*)+", "<QUESTION>");
            message = alex::str::replace(std::move(message), "(,\\s*)+", "<COMMA>");
            message = alex::str::replace(std::move(message), "((\\n\\r)(\\n\\r)+|\\n\\n+|\\r\\r+)", "<PARAGRAPH>");

            const std::regex r("(<[A-Z]+>|[a-z0-9\\-]+)");
            //const std::regex r("(<A-Z>|\\.|!|:|;|\\?|,|[a-z0-9\\-]+)");
            const std::regex_iterator<std::string::iterator> end;
            const std::regex_iterator<std::string::iterator> begin(message.begin(), message.end(), r);
            std::vector<std::string> tokens;
            for (auto i = begin; i != end; ++i)
                tokens.push_back(std::move(std::string((*i)[0])));
            return tokens;

            /*
                for (auto tok : tokens)
                {
                    if (tok == "<DASHES>")
                        std::cout << "--";
                    else if (tok == "<START>")
                        continue;
                    else if (tok == "<PARAGRAPH>")
                    {
                        n = 0;
                        std::cout << std::endl << std::endl;
                        continue;
                    }
                    else if (tok == "<PERIOD>")
                        std::cout << ".";
                    else if (tok == "<EXCLAMATION>")
                        std::cout << "!";
                    else if (tok == "<QUESTION>")
                        std::cout << "?";
                    else if (tok == "<SEMICOLON>")
                        std::cout << ";";
                    else if (tok == "<COLON>")
                        std::cout << ":";
                    else if (tok == "<COMMA>")
                        std::cout << ",";
                    else if (tok == "<END>")
                    {
                        n = 0;
                        std::cout << std::endl << std::endl << std::endl;
                        continue;
                    }
                    else
                        std::cout << tok;

                    ++n;
                    if (n > 10)
                    {
                        n = 0;
                        std::cout << std::endl;
                    }
                    else
                        std::cout << " ";
                }
            */
        }
    };

    std::vector<std::string> tokenGenerator(const NGramModel& model, size_t numTokens)
    {
        std::vector<std::string> evidence;
        for (size_t i = 1; i < model.gramSize(); ++i)
            evidence.push_back("<START>");

        auto entropy = alex::stochastic::Entropy<>();

        std::vector<std::string> tokens;

        tokens.reserve(numTokens);
        std::string token;

        //auto inverseCdf = model.inverseCdfLIFn({0.05f, 0.15f, 0.8f});
        model.write(std::cout);
        auto inverseCdf = model.inverseCdfFn();
        for (size_t i = 0; i < numTokens; ++i)
        {
            token = inverseCdf((float)entropy.get0_1(), evidence);
            tokens.push_back(token);

            if (token == "<END>")
            {
                evidence.clear();
                for (size_t i = 1; i < model.gramSize(); ++i)
                    evidence.push_back("<START>");
            }
            else
            {
                evidence.push_back(token);
                evidence.erase(evidence.begin());
            }
        }

        return tokens;
    }

    void parse(NGramModel& model, const std::vector<boost::filesystem::path>& dirs, const std::string& filePattern,
        size_t nGramSize, size_t recursionDepth, bool ignorePunctuation)
    {
        try
        {
            for (const auto dir : dirs)
            {
                const auto& files = alex::io::filesystem::getFiles(dir, filePattern, recursionDepth);
                for (const auto& file : files)
                {
                    if (file.stem().string() == "README" ||
                        file.stem().string() == "NFO" ||
                        file.stem().string() == "DESCRIPTION")
                    {
                        continue;
                    }

                    std::ifstream infile(file.string());
                    if (!infile.is_open())
                    {
                        std::cout << "\t* Failed to open " << file << std::endl;
                        continue;
                    }

                    std::cout << "\t* Processing: " << file << std::endl;
                    model.parse(infile, nGramSize, ignorePunctuation);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}}

#endif
