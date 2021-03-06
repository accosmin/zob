#pragma once

#include <regex>
#include <memory>
#include "arch.h"
#include "tensor.h"
#include "stringi.h"

namespace nano
{
        class function_t;
        using ref_function_t = std::reference_wrapper<const function_t>;
        using rfunction_t = std::unique_ptr<function_t>;
        using rfunctions_t = std::vector<rfunction_t>;

        enum class convexity
        {
                yes,
                no,
                unknown,
        };

        ///
        /// \brief construct test functions having the number of dimensions within the given range.
        ///
        NANO_PUBLIC rfunctions_t get_functions(
                const tensor_size_t min_dims, const tensor_size_t max_dims,
                const std::regex& name_regex = std::regex(".+"));

        ///
        /// \brief construct convex test functions having the number of dimensions within the given range.
        ///
        NANO_PUBLIC rfunctions_t get_convex_functions(
                const tensor_size_t min_dims, const tensor_size_t max_dims,
                const std::regex& name_regex = std::regex(".+"));

        ///
        /// \brief generic multi-dimensional optimization problem.
        ///
        class NANO_PUBLIC function_t
        {
        public:
                ///
                /// \brief constructor
                ///
                function_t(const char* name,
                        const tensor_size_t size, const tensor_size_t min_size, const tensor_size_t max_size,
                        const convexity convex);

                ///
                /// \brief destructor
                ///
                virtual ~function_t() = default;

                ///
                /// \brief function name to identify it in tests and benchmarks
                ///
                string_t name() const;

                ///
                /// \brief number of dimensions and the range of valid dimensions
                ///
                tensor_size_t size() const { return m_size; }
                tensor_size_t min_size() const { return m_min_size; }
                tensor_size_t max_size() const { return m_max_size; }

                ///
                /// \brief check if the function is convex
                ///
                bool is_convex() const { return m_convex == convexity::yes; }

                ///
                /// \brief check if the function is convex along the [x1, x2] line
                ///
                bool is_convex(const vector_t& x1, const vector_t& x2, const int steps) const;

                ///
                /// \brief compute function value (and gradient if provided)
                ///
                virtual scalar_t vgrad(const vector_t& x, vector_t* gx = nullptr) const = 0;

                ///
                /// \brief compute the gradient accuracy (given vs. finite difference approximation)
                ///
                scalar_t grad_accuracy(const vector_t& x) const;

        private:

                // attributes
                const char*     m_name;                         ///<
                tensor_size_t   m_size, m_min_size, m_max_size; ///< #dimensions
                convexity       m_convex;                       ///<
        };
}
