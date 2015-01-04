#ifndef TAR_HPP
#define TAR_HPP

#include <iostream>
#include <string>

namespace tar
{
        typedef long long unsigned file_size_t;
        extern const int FILE_NAME_LENGTH;
        struct tar_header;

        ////////////////////////////////////////
        // Writing raw data
        ////////////////////////////////////////
        class writer {
                std::ostream& _dst;
        public:
                writer(std::ostream& dst) : _dst(dst) {}
                ~writer() { finish(); }

                // Append the data specified by |data| and |file_size| into the
                // tar file represented by |dst|.
                // In the tar file, the data will be available at |path_in_tar|.
                void put(std::string path_in_tar,
                         char const * const data,
                         const file_size_t data_size);

                // Call after everything has been added to the tar represented 
                // by |dst| to make it a valid tar file.
                void finish();
        };


        ////////////////////////////////////////
        // Reading raw data
        ////////////////////////////////////////
        class reader {
                std::istream& _inp;
                tar_header* _next_header;
                void _read_header();
        public:
                // Constructor, pass input stream |inp| pointing to a tar file.
                reader(std::istream& inp) : _inp(inp), _next_header(NULL) {}

                // Returns true iff another file can be read from |inp|.
                bool contains_another_file();

                // Returns file name of next file in |inp|.
                std::string get_next_file_name();
                // Returns file size of next file in |inp|. Use to allocate
                // memory for the |read_next_file()| call.
                file_size_t get_next_file_size();

                // Read next file in |inp| to |data|.
                void read_next_file(char * const data);
                // Skip next file in |inp|.
                void skip_next_file();

                // Returns number of files in tar at |inp|.
                int number_of_files();
        };
}

#endif // TAR_HPP

