#include "nanocv.h"
#include "common/dot.hpp"
#include "common/mad.hpp"
#include "common/unroll.hpp"
#include <boost/format.hpp>

using namespace ncv;

ncv::thread_pool_t pool;

typedef double test_scalar_t;
typedef ncv::tensor::vector_types_t<test_scalar_t>::tvector     test_vector_t;
typedef ncv::tensor::vector_types_t<test_scalar_t>::tvectors    test_vectors_t;

namespace ncv
{
        template
        <
                typename tscalar,
                typename tsize
        >
        tscalar dot_eig(const tscalar* vec1, const tscalar* vec2, tsize size)
        {
                return tensor::make_vector(vec1, size).dot(tensor::make_vector(vec2, size));
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        void mad_eig(const tscalar* idata, tscalar weight, tsize size, tscalar* odata)
        {
                tensor::make_vector(odata, size) += weight * tensor::make_vector(idata, size);
        }
}

template
<
        typename tvector
>
void rand_vector(size_t size, tvector& vector)
{
        vector.resize(size);
        vector.setRandom();
}

template
<
        typename tvector
>
void zero_vector(tvector& vector)
{
        vector.setZero();
}

template
<
        typename top,
        typename tvector,
        typename tscalar = typename tvector::Scalar
>
tscalar test_dot(
        top op, const char* name, size_t n_tests,
        const tvector& vec1, const tvector& vec2)
{
        const size_t vsize = vec1.size() / n_tests;

        const ncv::timer_t timer;

        // run multiple tests
        tscalar ret = 0;
        const tscalar* pvec1 = vec1.data();
        const tscalar* pvec2 = vec2.data();
        for (size_t t = 0; t < n_tests; t ++, pvec1 += vsize, pvec2 += vsize)
        {
                ret += op(pvec1, pvec2, vsize);
        }
        
        const size_t milis = static_cast<size_t>(timer.miliseconds());
        std::cout << name << "= " << text::resize(text::to_string(milis), 4, align::right) << "ms  ";
        
        return ret;
}

template
<
        typename top,
        typename tvector,
        typename tscalar = typename tvector::Scalar
>
tscalar test_mad(
        top op, const char* name, size_t n_tests,
        const tvector& vec1, const tvector& vec2, tscalar wei)
{
        vector_t cvec1 = vec1;
        vector_t cvec2 = vec2;

        const size_t vsize = vec1.size() / n_tests;

        const ncv::timer_t timer;

        // run multiple tests
        const tscalar* pvec1 = cvec1.data();
        tscalar* pvec2 = cvec2.data();
        for (size_t t = 0; t < n_tests; t ++, pvec1 += vsize, pvec2 += vsize)
        {
                op(pvec1, wei, vsize, pvec2);
        }

        const size_t milis = static_cast<size_t>(timer.miliseconds());
        std::cout << name << "= " << text::resize(text::to_string(milis), 4, align::right) << "ms  ";

        return cvec2.sum();
}

template
<
        typename tscalar
>
void check(tscalar result, tscalar baseline, const char* name)
{
        const tscalar err = math::abs(result - baseline);
        if (!math::almost_equal(err, tscalar(0)))
        {
                std::cout << name << " FAILED (diff = " << err << ")!" << std::endl;
        }
}

void test_dot(size_t size, size_t n_tests)
{
        test_vector_t vec1, vec2;
        rand_vector(size * n_tests, vec1);
        rand_vector(size * n_tests, vec2);

        const string_t header = (boost::format("%1% x (%2%): ") % n_tests % size).str();
        std::cout << text::resize(header, 20);

        typedef decltype(vec1.size()) test_size_t;
        
        const test_scalar_t dot    = test_dot(ncv::dot<test_scalar_t, test_size_t>, "dot", n_tests, vec1, vec2);
        const test_scalar_t dotul2 = test_dot(ncv::dot_unroll2<test_scalar_t, test_size_t>, "dotul2", n_tests, vec1, vec2);
        const test_scalar_t dotul4 = test_dot(ncv::dot_unroll4<test_scalar_t, test_size_t>, "dotul4", n_tests, vec1, vec2);
        const test_scalar_t dotul8 = test_dot(ncv::dot_unroll8<test_scalar_t, test_size_t>, "dotul8", n_tests, vec1, vec2);
        const test_scalar_t doteig = test_dot(ncv::dot_eig<test_scalar_t, test_size_t>, "doteig", n_tests, vec1, vec2);
        std::cout << std::endl;

        check(dot,      dot, "dot");
        check(dotul2,   dot, "dotul2");
        check(dotul4,   dot, "dotul4");
        check(dotul8,   dot, "dotul8");
        check(doteig,   dot, "doteig");
}

void test_mad(size_t size, size_t n_tests)
{
        test_vector_t vec1, vec2;
        rand_vector(size * n_tests, vec1);
        rand_vector(size * n_tests, vec2);

        test_scalar_t wei;

        wei = vec1(0) + vec2(3);

        const string_t header = (boost::format("%1% x (%2%): ") % n_tests % size).str();
        std::cout << text::resize(header, 20);

        typedef decltype(vec1.size()) test_size_t;

        const test_scalar_t mad    = test_mad(ncv::mad<test_scalar_t, test_size_t>, "mad", n_tests, vec1, vec2, wei);
        const test_scalar_t madul2 = test_mad(ncv::mad_unroll2<test_scalar_t, test_size_t>, "madul2", n_tests, vec1, vec2, wei);
        const test_scalar_t madul4 = test_mad(ncv::mad_unroll4<test_scalar_t, test_size_t>, "madul4", n_tests, vec1, vec2, wei);
        const test_scalar_t madul8 = test_mad(ncv::mad_unroll8<test_scalar_t, test_size_t>, "madul8", n_tests, vec1, vec2, wei);
        const test_scalar_t madeig = test_mad(ncv::mad_eig<test_scalar_t, test_size_t>, "madeig", n_tests, vec1, vec2, wei);
        std::cout << std::endl;

        check(mad,      mad, "mad");
        check(madul2,   mad, "madul2");
        check(madul4,   mad, "madul4");
        check(madul8,   mad, "madul8");
        check(madeig,   mad, "madeig");
}

int main(int argc, char* argv[])
{
        static const size_t min_size = 4;
        static const size_t max_size = 64;
        static const size_t n_tests = 1023 * 991 * 7;

        for (size_t size = min_size; size <= max_size; size *= 2)
        {
                test_dot(size, n_tests);
        }
        std::cout << std::endl;

        for (size_t size = min_size; size <= max_size; size *= 2)
        {
                test_mad(size, n_tests);
        }

	return EXIT_SUCCESS;
}

