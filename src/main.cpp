#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "json_parser.h"

static std::vector<uint8_t> load_file(FILE* f) 
{
    std::vector<uint8_t> data;
    while (1) {
        char buf[256];
        if (!fgets(buf, sizeof(buf), f))
            break;        
        data.insert(data.end(), (uint8_t*)&buf[0], (uint8_t*)&buf[strlen(buf)]);
    }
    return data;
}

static std::vector<uint8_t> get_input(int argc, const char** argv)
{
    if (argc > 1) {
        const char* file_name = argv[1];
        FILE* f = fopen(file_name, "r");
        if (!f) {
            printf("cannot open file \"%s\"\n", file_name);
            exit(-1);
        }

        std::vector<uint8_t> data = load_file(f);
        fclose(f);
        return data;
    }

    return load_file(stdin);
}

int main(int argc, const char** argv)
{
    auto p = json_object();
    auto r = run_parser(p, ParseStream(get_input(argc, argv)));

    if (r.first)
        json_dump(r.second);
    else
        printf("parse error\n");

    return 0;
}
