#include "tar.hpp"

#include <string>
#include <fstream>

void test_tar_lib();
void test_folders();
void test_streq();

int main() {
        test_streq();
        test_tar_lib();
        test_folders();
        return 0;
}

bool _tassert(bool condition, std::string condition_str)
{
        if (!(condition)) {
                std::cout << "FAILED: " << condition_str << std::endl;
        } else {
                std::cout << "PASSED: " << condition_str << std::endl;
        }

        return condition;
}

#define tassert(_COND_) _tassert(_COND_, #_COND_)

bool streq(char const * a, size_t len_a,
           char const * b, size_t len_b)
{
        if (a == b) return true;
        if (len_a != len_b) return false;

        for (size_t i = 0; i < len_a; i++)
                if (a[i] != b[i])
                        return false;

        return true;
}

void test_streq()
{
        tassert(streq("hello", 6, "hello", 6) == true);
        tassert(streq("hello", 6, "helloo", 7) == false);
        tassert(streq("hello", 6, "heooo", 6) == false);
}

void test_tar_lib()
{
        std::ofstream o("test.tar", std::ios::out | std::ios::binary);
        tar::writer w(o);

        char test_string[] = {'h', '\n'};
        w.put("simple.txt", test_string, 2);

        const size_t LONG_STRING_LENGTH = 10000;
        char long_string[LONG_STRING_LENGTH];

        for (size_t i = 0; i < LONG_STRING_LENGTH - 1; i++) {
                long_string[i] = 'a';
        }
        long_string[LONG_STRING_LENGTH - 1] = '\n';

        w.put("folder/long.txt", long_string, LONG_STRING_LENGTH);
        w.finish();
        o.close();

        std::cout << "Writing finished..." << std::endl;

        std::ifstream i("test.tar", std::ios::in | std::ios::binary);

        tar::reader r(i);
        tassert(r.number_of_files() == 2);
        tassert(r.contains_another_file());

        tar::file_size_t size;
        char * target;

        tassert(r.get_next_file_name() == "simple.txt");

        size = r.get_next_file_size();
        tassert(size == 2);

        target = (char *)malloc(sizeof(char) * size);
        r.read_next_file(target);

        tassert(streq(target, size, test_string, size));
        free(target);

        // test whether seeking works
        tassert(r.number_of_files() == 2);

        tassert(r.get_next_file_name() == "folder/long.txt");

        size = r.get_next_file_size();
        tassert(size == LONG_STRING_LENGTH);

        target = (char *)malloc(sizeof(char) * size);
        r.read_next_file(target);

        tassert(streq(target, size, long_string, LONG_STRING_LENGTH));
        free(target);

        tassert(r.contains_another_file() == false);
}

void test_folders()
{
        std::ofstream o("folder_test.tar", std::ios::out | std::ios::binary);
        tar::writer w(o);

        w.put_directory("test_folder");
        w.finish();
}

