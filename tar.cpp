#include "tar.hpp"

#include <cstring>  // for strlen and memset
#include <cstdio>   // for sprintf, snprintf and sscanf
#include <cstdlib>  // for rand
#include <ctime>    // for time

#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
#  define LOG printf
#else
#  define LOG(fmt, args ...) ((void)0)
#endif

namespace tar
{
        const char FILL_CHAR = '\0';
        const int FILE_NAME_LENGTH = 100;

        // From http://en.wikipedia.org/wiki/Tar_(computing)#UStar_format
        typedef enum 
        {
                tar_file_type_normal = 0,              
                tar_file_type_hard_link = 1,                  
                tar_file_type_soft_link = 2,
                tar_file_type_directory = 5                  
        } tar_file_type;

        struct tar_header                
        {
                char name[FILE_NAME_LENGTH];  // file name
                char mode[8];                 // file mode
                char uid[8];                  // Owner's numeric user ID
                char gid[8];                  // Group's numeric user ID
                char size[12];                // File size in bytes (octal base)
                char mtime[12];               // Last modification time in
                                              // numeric Unix time format (octal)
                char checksum[8];             // Checksum for header record
                char typeflag[1];             // file type, see tar_file_type
                char linkname[100];           // Name of linked file
                char magic[6];                // UStar indicator "ustar"
                char version[2];              // UStar version "00"
                char uname[32];               // Owner user name
                char gname[32];               // Owner group name
                char devmajor[8];             // Device major number
                char devminor[8];             // Device minor number
                char prefix[155];             // Filename prefix
                char pad[12];                 // padding
        };

        void header_setMetadata(tar_header* header)
        {
                std::memset(header, 0, sizeof(tar_header));

                std::sprintf(header->magic, "ustar");
                std::sprintf(header->mtime, "%011lo", std::time(NULL));
                std::sprintf(header->mode, "%07o", 0644);
                //TODO
                std::sprintf(header->uname, "unkown");  // ... a bit random
                std::sprintf(header->gname, "users");
                header->typeflag[0] = 0;  // always just a normal file
        }

        /* From Wikipedia: The checksum is calculated by taking the sum of the
         * unsigned byte values of the header record with the eight checksum
         * bytes taken to be ascii spaces. */
        void header_setChecksum(tar_header* header)
        {
                unsigned int sum = 0;

                char *pointer = (char *) header;
                char *end = pointer + sizeof(tar_header);

                // Iterate over header struct until we are at checksum field.
                while (pointer < header->checksum) {
                        sum += *pointer & 0xff;
                        pointer++;
                }

                // ... then add eight 'ascii spaces' ...
                sum += ' ' * 8;
                pointer += 8;

                // ... and go until the end.
                while (pointer < end) {
                        sum += *pointer & 0xff;
                        pointer++;
                }

                std::sprintf(header->checksum, "%06o", sum);
        }

        void header_setFilesize(tar_header* header, file_size_t fileSize)
        {
                std::sprintf(header->size, "%011llo", fileSize);
        }

        void header_getFilesize(tar_header* header, file_size_t* fileSize)
        {
                std::sscanf(header->size, "%011llo", fileSize);
        }

        void header_setFilename(tar_header* header, const char* fileName)
        {
                size_t len = std::strlen(fileName);

                // len > 0 also ensures that the header does not start with \0
                if (len == 0 || len >= FILE_NAME_LENGTH)  
                {
                        LOG("Invalid file name for tar: %s", fileName);
                        std::sprintf(header->name, "INVALID_%d", std::rand());
                }
                else
                {
                        std::sprintf(header->name, "%s", fileName);
                }
        }

        void header_getFilename(tar_header* header, char* fileName)
        {
                std::sscanf(header->name, "%s", fileName);
        }

        /* Every file in a tar file starts with the tar header */
        void writeHeader(std::ostream& dst, const char* fileName, file_size_t fileSize)
        {
                tar_header header;
                header_setMetadata(&header);
                header_setFilename(&header, fileName);
                header_setFilesize(&header, fileSize);
                header_setChecksum(&header);

                dst.write((const char*)&header, sizeof(tar_header));
        }

        void readHeader(std::istream& inp, tar_header* header)
        {
                inp.read((char*)header, sizeof(tar_header));
        }

        /* The length of the data after the header must be rounded up to a
           multiple of 512 bytes, the length of the header. */
        void fill(std::ostream& dst, unsigned long fileSize)
        {
                while (fileSize % sizeof(tar_header) != 0)
                {
                        dst.put(FILL_CHAR);
                        fileSize++;
                }
        }

        void put(std::ostream& dst,
                 const char * const pathInTar,
                 char const * const dataBegin,
                 const file_size_t fileSize)
        {
                writeHeader(dst, pathInTar, fileSize);
                dst.write(dataBegin, fileSize);
                fill(dst, fileSize);
        }

        /* The end of an tar is marked by at least two consecutive zero-filled 
         * records, a record having the size of the header. */
        void finish(std::ostream& dst)
        {
                unsigned long i = 0;
                while (i < 2 * sizeof(tar_header))
                {
                        dst.put(FILL_CHAR);
                        i++;
                }
        }

        bool checkIfHeaderIsNext(std::istream& inp)
        {
                if (inp.eof() || inp.peek() == EOF)
                {
                        LOG("Can not read next file info, istream is at EOF.");
                        return false;
                }

                if (inp.peek() == FILL_CHAR)
                {
                        LOG("Can not read next file info, istream is pointing"
                            "to %d, which a tar header can not start with.",
                            FILL_CHAR);
                        return false;
                }

                return true;
        }

        bool getNextFileInfo(std::istream& inp,
                             char* pathInTar,
                             file_size_t* fileSize)
        {
                if (!checkIfHeaderIsNext(inp))
                        return false;

                tar_header header;
                readHeader(inp, &header);

                header_getFilesize(&header, fileSize);
                header_getFilename(&header, pathInTar);

                return true;
        }

        void getNextFileData(std::istream& inp,
                             char * const data,
                             const file_size_t fileSize)
        {
                if (data == NULL)  // NULL gets passed from skip()
                        // seek |fileSize| bytes from CURrent position, to skip
                        inp.seekg(fileSize, std::ios::cur);  
                else
                        inp.read(data, fileSize);

                // Advance to start of next header or to end of file
                // Works because
                // - header never starts with FILL_CHAR
                // - at end of file, peek() returns EOF.
                // - FILL_CHAR != EOF
                while (inp.peek() == FILL_CHAR)
                        inp.get();
        }

        void skip(std::istream& inp, const file_size_t fileSize)
        {
                getNextFileData(inp, NULL, fileSize);
        }

}  // namespace tar

