#include "json.h"
#include <cstdio>

static void make_indent(char* dst, int depth)
{
    int i = 0;
    while (i < depth)
        dst[i++] = '\t';
    dst[i] = 0;
}

static void json_object_dump(const JSonObject& obj, int depth);
static void json_array_dump(const JSonArray& array, int depth);

static void json_value_dump(const JSonValue& val, int depth)
{
    switch (val.type()) {
    case JSonValueType::Bool:
        printf("%s", (val.boolean() ? "true" : "false"));
        break;
    case JSonValueType::Number:
        printf("%d", val.number());
        break;
    case JSonValueType::String:
        printf("\"%s\"", val.string().c_str());
        break;
    case JSonValueType::Object:
        json_object_dump(val.object(), depth);
        break;
    case JSonValueType::Array:
        json_array_dump(val.array(), depth);
        break;
    default:
        break;
    }
}

static void json_array_dump(const JSonArray& array, int depth)
{
    char indent[256];
    make_indent(indent, depth);

    printf("[\n");
    for (size_t i = 0; i < array.size(); ++i) {
        printf("%s\t", indent);
        json_value_dump(array[i], depth + 1);
        if (i < array.size()-1)
            printf(",");
        printf("\n");
    }
    printf("%s]", indent);
}

static void json_object_dump(const JSonObject& obj, int depth)
{
    char indent[256];
    make_indent(indent, depth);

    printf("{\n");
    size_t n = obj.size();
    size_t i = 0;
    for (const auto& p : obj) {
        printf("%s\t\"%s\": ", indent, p.first.c_str());
        json_value_dump(p.second, depth + 1);
        if (++i != n)
            printf(",");
        printf("\n");
    }
    printf("%s}", indent);
}

void json_dump(const JSonObject& obj)
{
    json_object_dump(obj, 0);
    printf("\n");
}
