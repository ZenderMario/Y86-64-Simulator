//
// Created by ZenderMario on 2023/4/30.
//

#ifndef Y86_64_SIMULATOR_READER_H
#include <fstream>
#include <map>
#include "Unit.h"

namespace Reader
{
    struct Ins
    {
        std::string ins;
        std::string first;
        std::string second;
    };

    class FileReader
    {
    private:
        std::ifstream is;
        bool good = true;

    public:
        explicit FileReader( const std::string& name) : is( name) {}
        void ReWind();
        bool IsOpen() { return is.is_open();}
        bool Good() { return good;}
        void Read( std::string& input, char end = '\n');
        void Read( char gap = '\n');
    };
}

#define Y86_64_SIMULATOR_READER_H

#endif //Y86_64_SIMULATOR_READER_H
