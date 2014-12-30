#ifndef TAR_HPP
#define TAR_HPP

#include <iostream>

namespace tar
{
        typedef long long unsigned file_size_t;
        extern const int FILE_NAME_LENGTH;

        // Append the data specified by |dataBegin| and |fileSize| into the tar file represented by |dst|.
        // In the tar file, the data will be available at |pathInTar|.
        void put(std::ostream& dst,
                 const char * const pathInTar,
                 char const * const dataBegin, const file_size_t fileSize);

        // Call after everything has been added to the tar to make it a valid tar file.
        void finish(std::ostream& dst);

        // Reads for the next file:
        // - the path in the tar (note: is at most FILE_NAME_LENGTH chars long)
        // - the size in bytes
        // Returns false if reading fails
        bool getNextFileInfo(std::istream& inp,
                             char* pathInTar,
                             file_size_t* fileSize);

        // Reads data of next file into |data|
        void getNextFileData(std::istream& inp,
                             char * const data, 
                             const file_size_t fileSize);

        // Skips the current file of size |fileSize|
        void skip(std::istream& inp, const file_size_t fileSize);
}

#endif // TAR_HPP

