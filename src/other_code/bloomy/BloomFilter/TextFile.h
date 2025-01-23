#ifndef __TEXT_FILE_H__
#define __TEXT_FILE_H__

#include "Str.h"
#include <fstream>
#include <sstream>

namespace alex
{

class TextFile
{
public:
    TextFile() {};

    bool open(const alex::Str& filename)
    {
        std::ifstream infile(filename.data().c_str());
        if (infile.fail())
        {
            return false;
        }

        this->filename = filename;
        std::stringstream ss;
        ss << infile.rdbuf();
        contents = alex::Str(ss.str());

        return true;
    };

    void close()
    {
        contents = filename = "";
    };

    bool save()
    {
        return save(this->filename);
    };

    bool save(const alex::Str& filename)
    {
        if (this->filename.empty())
        {
            throw std::exception("No File Opened");
        }

        std::ifstream infile(filename.data().c_str());
        if (infile.fail())
        {
            return false;
        }

        std::stringstream ss;
        ss << infile.rdbuf();

        contents = alex::Str(ss.str());
        return true;
    };

    alex::Str& getContents()
    {
        if (filename.empty())
        {
            throw std::exception("No File Opened");
        }

        return contents;
    };

private:
    alex::Str contents;
    alex::Str filename;
};

};

#endif