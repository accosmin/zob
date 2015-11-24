#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_random"

#include <boost/test/unit_test.hpp>
#include "math/random.hpp"

BOOST_AUTO_TEST_CASE(test_random)
{
        const int32_t tests = 231;
        const int32_t test_size = 65;

        for (int32_t t = 0; t < tests; ++ t)
        {
                const int32_t min = 17 + t;
                const int32_t max = min + t * 25 + 4;

                // initialize (uniform) random number generator
                math::random_t<int32_t> rgen(min, max);

                // check generator
                for (int32_t tt = 0; tt < test_size; ++ tt)
                {
                        const int32_t v = rgen();
                        BOOST_CHECK_GE(v, min);
                        BOOST_CHECK_LE(v, max);
                }
        }
}