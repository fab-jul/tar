#ifndef TAR_HPP
#define TAR_HPP

#include <iostream>

namespace tar
{
        typedef long long unsigned file_size_t;
        extern const int FILE_NAME_LENGTH;

        ////////////////////////////////////////
        // Writing raw data
        ////////////////////////////////////////

        // Append the data specified by |data| and |file_size| into the tar 
        // file represented by |dst|.
        // In the tar file, the data will be available at |path_in_tar|.
        void put(std::ostream& dst,
                 char const * const path_in_tar,
                 char const * const data, const file_size_t data_size);

        // Call after everything has been added to the tar represented by |dst|
        // to make it a valid tar file.
        void finish(std::ostream& dst);

        ////////////////////////////////////////
        // Reading raw data
        ////////////////////////////////////////

        // Reads for the next file:
        // - the path in the tar (note: is at most FILE_NAME_LENGTH chars long)
        // - the size in bytes
        // Returns false if reading fails
        bool getNextFileInfo(std::istream& inp,
                             char * path_in_tar,
                             file_size_t * file_size);

        // Reads data of size |file_size| of the next file into |data|.
        void getNextFileData(std::istream& inp,
                             char * const data, const file_size_t file_size);

        // Skips the current file of size |file_size|.
        void skip(std::istream& inp, const file_size_t file_size);
}

#endif // TAR_HPP

