//
// Created by ZenderMario on 2023/4/30.
//

#include "Reader.h"
#include <cstdlib>
#include <algorithm>

namespace Reader
{
    void FileReader::ReWind()  {
        is.seekg( 0); //rewind it to beginning of file
    }

    void FileReader::Read( std::string& input, char end) {
        if( is.peek() == '?')
        {
            good = false;
            std::cout << "EOF reached" << std::endl;
            is.close();
            return;
        }

        std::getline( is, input, end);
        input.erase( std::remove( input.begin(), input.end(), ' '), input.end());

        if( !is){
            std::cout << "Error" << std::endl;
            throw std::runtime_error( "Failed to read more");
        }

    }

    void FileReader::Read( char gap) {
        char tmp = is.get();
        while( tmp != gap && is) {
            tmp = is.get();
        }
    }
}