#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_average"

#include <boost/test/unit_test.hpp>
#include "math/abs.hpp"
#include "math/average.hpp"
#include "math/epsilon.hpp"
#include <eigen3/Eigen/Core>

namespace test
{
        template
        <
                typename tsize
        >
        tsize sign(const tsize index)
        {
                return (index % 2 == 0) ? tsize(+1) : tsize(-1);
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        tscalar average1(const tsize range)
        {
                return static_cast<tscalar>(range + 1) / static_cast<tscalar>(2);
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        tscalar average2(const tsize range)
        {
                return static_cast<tscalar>((range + 1) * sign(range + 1)) / static_cast<tscalar>(2);
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        void check_average(const tsize range)
        {
                math::average_scalar_t<tscalar> avg1, avg2;
                for (tsize i = 1; i <= range; ++ i)
                {
                        avg1.update(tscalar(i));
                        avg2.update(tscalar(sign(i + 1) * i) * tscalar(i));
                }

                const auto epsilon = math::epsilon1<tscalar>();
                const auto base1 = average1<tscalar>(range);
                const auto base2 = average2<tscalar>(range);

                BOOST_CHECK_LE(math::abs(avg1.value() - base1), epsilon);
                BOOST_CHECK_LE(math::abs(avg2.value() - base2), epsilon);
        }

        template
        <
                typename tvector,
                typename tscalar = typename tvector::Scalar,
                typename tsize = typename tvector::Index
        >
        void check_average(const tsize dims, const tsize range)
        {
                math::average_vector_t<tvector> avg1(dims), avg2(dims);
                for (tsize i = 1; i <= range; ++ i)
                {
                        avg1.update(tvector::Constant(dims, tscalar(i)));
                        avg2.update(tvector::Constant(dims, tscalar(sign(i + 1) * i) * tscalar(i)));
                }

                const auto epsilon = math::epsilon1<tscalar>();
                const auto base1 = tvector::Constant(dims, average1<tscalar>(range));
                const auto base2 = tvector::Constant(dims, average2<tscalar>(range));

                BOOST_CHECK_LE((avg1.value() - base1).template lpNorm<Eigen::Infinity>(), epsilon);
                BOOST_CHECK_LE((avg2.value() - base2).template lpNorm<Eigen::Infinity>(), epsilon);
        }
}

BOOST_AUTO_TEST_CASE(test_average_scalar)
{
        test::check_average<double>(1);
        test::check_average<double>(5);
        test::check_average<double>(17);
        test::check_average<double>(85);
        test::check_average<double>(187);
        test::check_average<double>(1561);
        test::check_average<double>(14332);
        test::check_average<double>(123434);
}

BOOST_AUTO_TEST_CASE(test_average_vector)
{
        test::check_average<Eigen::VectorXd>(13, 1);
        test::check_average<Eigen::VectorXd>(17, 5);
        test::check_average<Eigen::VectorXd>(11, 17);
        test::check_average<Eigen::VectorXd>(21, 85);
        test::check_average<Eigen::VectorXd>(27, 187);
        test::check_average<Eigen::VectorXd>(15, 1561);
        test::check_average<Eigen::VectorXd>(19, 14332);
        test::check_average<Eigen::VectorXd>(18, 123434);
}

