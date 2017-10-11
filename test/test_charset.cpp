#include "task.h"
#include "utest.h"
#include "text/config.h"
#include "vision/color.h"
#include "math/epsilon.h"
#include "tasks/charset.h"

using namespace nano;

NANO_BEGIN_MODULE(test_charset)

NANO_CASE(construction)
{
        // <charset, color mode, number of outputs/classes/characters>
        std::vector<std::tuple<charset_type, color_mode, tensor_size_t>> configs;
        configs.emplace_back(charset_type::digit,            color_mode::luma,       tensor_size_t(10));
        configs.emplace_back(charset_type::lalpha,           color_mode::rgba,       tensor_size_t(26));
        configs.emplace_back(charset_type::ualpha,           color_mode::luma,       tensor_size_t(26));
        configs.emplace_back(charset_type::alpha,            color_mode::luma,       tensor_size_t(52));
        configs.emplace_back(charset_type::alphanum,         color_mode::rgba,       tensor_size_t(62));

        for (const auto& config : configs)
        {
                const auto type = std::get<0>(config);
                const auto mode = std::get<1>(config);
                const auto irows = tensor_size_t(17);
                const auto icols = tensor_size_t(16);
                const auto osize = std::get<2>(config);
                const auto count = size_t(2 * osize);
                const auto fsize = size_t(1);   // folds

                const auto idims = tensor3d_dims_t{(mode == color_mode::rgba) ? 4 : 1, irows, icols};
                const auto odims = tensor3d_dims_t{osize, 1, 1};

                auto task = get_tasks().get("synth-charset", to_params(
                        "type", type, "color", mode, "irows", irows, "icols", icols, "count", count));

                NANO_CHECK_EQUAL(task->load(), true);
                NANO_CHECK_EQUAL(task->idims(), idims);
                NANO_CHECK_EQUAL(task->odims(), odims);
                NANO_CHECK_EQUAL(task->fsize(), fsize);
                NANO_CHECK_EQUAL(task->size(), count);
        }
}

NANO_CASE(from_params)
{
        auto task = get_tasks().get("synth-charset", "type=alpha,color=rgb,irows=18,icols=17,count=102");
        NANO_CHECK(task->load());

        const auto idims = tensor3d_dims_t{3, 18, 17};
        const auto odims = tensor3d_dims_t{52, 1, 1};
        const auto target_sum = scalar_t(2) - static_cast<scalar_t>(nano::size(odims));

        NANO_CHECK_EQUAL(task->idims(), idims);
        NANO_CHECK_EQUAL(task->odims(), odims);
        NANO_CHECK_EQUAL(task->fsize(), size_t(1));
        NANO_CHECK_EQUAL(task->size(), size_t(102));

        NANO_CHECK_EQUAL(
                task->size({0, protocol::train}) +
                task->size({0, protocol::valid}) +
                task->size({0, protocol::test}),
                size_t(102));

        for (const auto p : {protocol::train, protocol::valid, protocol::test})
        {
                const auto size = task->size({0, p});
                for (size_t i = 0; i < size; ++ i)
                {
                        const auto sample = task->get({0, p}, i);
                        const auto& input = sample.m_input;
                        const auto& target = sample.m_target;

                        NANO_CHECK_EQUAL(input.dims(), idims);
                        NANO_CHECK_EQUAL(target.dims(), odims);
                        NANO_CHECK_CLOSE(target.vector().sum(), target_sum, epsilon0<scalar_t>());
                }
        }

        const size_t max_duplicates = 0;
        NANO_CHECK_LESS_EQUAL(nano::check_duplicates(*task), max_duplicates);
        NANO_CHECK_LESS_EQUAL(nano::check_intersection(*task), max_duplicates);
}

NANO_CASE(shuffle)
{
        auto task = get_tasks().get("synth-charset", "type=alpha,color=rgb,irows=18,icols=17,count=102");
        NANO_CHECK(task->load());

        for (const auto p : {protocol::train, protocol::valid, protocol::test})
        {
                const auto fold = fold_t{0, p};
                const auto size = task->size(fold);

                std::map<size_t, size_t> iohashes;
                for (size_t i = 0; i < size; ++ i)
                {
                        iohashes[task->ihash(fold, i)] = task->ohash(fold, i);
                }

                for (auto t = 0; t < 8; ++ t)
                {
                        task->shuffle(fold);
                        NANO_REQUIRE_EQUAL(task->size(fold), size);

                        for (size_t i = 0; i < size; ++ i)
                        {
                                const auto ihash = task->ihash(fold, i);
                                const auto ohash = task->ohash(fold, i);
                                const auto it = iohashes.find(ihash);

                                NANO_REQUIRE(it != iohashes.end());
                                NANO_CHECK_EQUAL(it->second, ohash);
                        }
                }
        }
}

NANO_END_MODULE()
