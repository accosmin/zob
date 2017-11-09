#include "utest.h"
#include "text/json_reader.h"
#include "text/json_writer.h"

using namespace nano;

std::ostream& operator<<(std::ostream& os, const json_tag tag)
{
        return os << to_string(tag);
}

NANO_BEGIN_MODULE(test_json)

NANO_CASE(writer_simple)
{
        const size_t tabsize = 0;
        const size_t spacing = 0;
        const bool newline = false;

        auto writer = json_writer_t(tabsize, spacing, newline);
        writer.begin_object();
                writer.pair("param1", "v").next();
                writer.pair("param2", -42).next();
                writer.name("array1").array(1, 2, 3).next();
                writer.pair("param3", +42).next();
                writer.name("null").null();
        writer.end_object();

        NANO_CHECK_EQUAL(writer.get(),
        "{\"param1\":\"v\",\"param2\":-42,\"array1\":[1,2,3],\"param3\":42,\"null\":null}");
}

NANO_CASE(writer_complex)
{
        const size_t tabsize = 0;
        const size_t spacing = 0;
        const bool newline = false;

        auto writer = json_writer_t(tabsize, spacing, newline);
        writer.begin_object();
                writer.pair("param1", "v").next();
                writer.name("object1").begin_object();
                        writer.name("array1").begin_array();
                                writer.begin_object();
                                        writer.pair("name11", 11).next();
                                        writer.pair("name12", 12);
                                writer.end_object().next();
                                writer.begin_object();
                                        writer.pair("name21", 21).next();
                                        writer.pair("name22", 22);
                                writer.end_object();
                        writer.end_array().next();
                        writer.pair("field1", "v1").next();
                        writer.pair("field2", "v2");
                writer.end_object();
        writer.end_object();

        NANO_CHECK_EQUAL(writer.get(),
        "{\"param1\":\"v\",\"object1\":{\"array1\":[{\"name11\":11,\"name12\":12},{\"name21\":21,\"name22\":22}],"\
        "\"field1\":\"v1\",\"field2\":\"v2\"}}");
}

NANO_CASE(reader_complex)
{
        const string_t json = R"XXX(
{
        "string":       "value1",
        "integer":      42,
        "array":        [1, 2, 3],
        "null":         null,
        "object_array": [
        {
                "name": "name1",
                "int":  1
        },
        {
                "name": "name2",
                "int:   2
        }]
}
)XXX";

        const std::vector<std::pair<string_t, json_tag>> calls = {
                { "", json_tag::begin_object },
                { "string", json_tag::name },
                { "value1", json_tag::value },
                { "integer", json_tag::name },
                { "42", json_tag::value },
                { "array", json_tag::name },
                { "", json_tag::begin_array },
                        { "1", json_tag::value },
                        { "2", json_tag::value },
                        { "3", json_tag::value },
                { "", json_tag::end_array },
                { "null", json_tag::name },
                { "null", json_tag::null },
                { "object_array", json_tag::name },
                { "", json_tag::begin_array },
                { "", json_tag::begin_object },
                        { "name", json_tag::name },
                        { "name1", json_tag::value },
                        { "int", json_tag::name },
                        { "1", json_tag::value },
                { "", json_tag::end_object },
                { "", json_tag::begin_object },
                        { "name", json_tag::name },
                        { "name2", json_tag::value },
                        { "int", json_tag::name },
                        { "2", json_tag::value },
                { "", json_tag::end_object },
                { "", json_tag::end_array },
                { "", json_tag::end_object }
        };

        size_t index = 0;
        const auto callback = [&] (const char* name, const size_t size, const json_tag tag)
        {
                NANO_REQUIRE_LESS(index, calls.size());
                NANO_CHECK_EQUAL(calls[index].first, string_t(name, size));
                NANO_CHECK_EQUAL(calls[index].second, tag);
                ++ index;

                std::cout << "token = [" << string_t(name, size) << "], tag = " << to_string(tag) << std::endl;
        };

        json_reader_t reader(json);
        reader.parse(callback);

        NANO_CHECK_EQUAL(index, calls.size());
}

NANO_END_MODULE()
