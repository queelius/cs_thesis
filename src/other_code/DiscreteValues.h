#include "CompactArray.h"
#include <map>

namespace alex { namespace collections
{
    class DiscreteValues
    {
    public:
        template <class Iter>
        static DiscreteValues make(Iter begin, Iter end)
        {
            std::map<uint32_t, uint32_t> m;

            size_t sz = std::distance(begin, end);

            DiscreteValues v;
            v._indices.initialize(sz, 0, sz-1);

            uint32_t id = 0, idx = 0, lo = std::numeric_limits<uint32_t>::max(), hi = std::numeric_limits<uint32_t>::min();
            for (auto i = begin; i != end; ++i)
            {
                if (m.count(*i) == 0)
                    m[*i] = id++;

                lo = std::min(lo, *i);
                hi = std::max(hi, *i);

                v._indices.set(idx++, m[*i]);
            }

            v._map.initialize(m.size(), lo, hi);
            idx = 0;
            for (auto i = m.begin(); i != m.end(); ++i)
                v._map.set(i->second, i->first);

            return v;
        };

        size_t size() const
        {
            return _indices.size();
        };

        uint32_t get(unsigned int idx) const
        {
            if (idx >= _indices.size())
                throw std::exception("Invalid Index");

            return _map.get(_indices.get(idx));
        };

        void write(std::ostream& file)
        {
            alex::io::binary::writeString(file, HEADER);
            alex::io::binary::writeU8(file, VERSION);
            _indices.write(file);
            _map.write(file);
        };

        void read(std::istream& file)
        {
            auto hdr = alex::io::binary::readString(file);
            if (hdr != HEADER)
                throw std::exception(("Unexpected Header: " + hdr).c_str());

            auto ver = alex::io::binary::readU8(file);
            if (ver != VERSION)
                throw std::exception(("Unexpected Version: " + std::to_string(ver)).c_str());

            _indices.read(file);
            _map.read(file);
        };

    private:
        static const std::string HEADER;
        static const uint8_t VERSION;
        CompactArray _indices;
        CompactArray _map;
    };

    const std::string DiscreteValues::HEADER = "dv";
    const uint8_t DiscreteValues::VERSION = 1;

}}