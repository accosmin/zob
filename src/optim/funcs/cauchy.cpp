#include "cauchy.h"
#include "util.hpp"

namespace nano
{
        function_cauchy_t::function_cauchy_t(const tensor_size_t dims) :
                m_dims(dims)
        {
        }

        std::string function_cauchy_t::name() const
        {
                return "Cauchy" + std::to_string(m_dims) + "D";
        }

        problem_t function_cauchy_t::problem() const
        {
                const auto fn_size = [=] ()
                {
                        return m_dims;
                };

                const auto fn_fval = [=] (const vector_t& x)
                {
                        return std::log((1.0 + x.array().square()).prod());
                };

                const auto fn_grad = [=] (const vector_t& x, vector_t& gx)
                {
                        gx = (2 * x.array()) / (1 + x.array().square());

                        return fn_fval(x);
                };

                return {fn_size, fn_fval, fn_grad};
        }

        bool function_cauchy_t::is_valid(const vector_t&) const
        {
                return true;
        }

        bool function_cauchy_t::is_minima(const vector_t& x, const scalar_t epsilon) const
        {
                return util::distance(x, vector_t::Zero(m_dims)) < epsilon;
        }
}