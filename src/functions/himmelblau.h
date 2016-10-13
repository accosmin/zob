#pragma once

#include "function.h"

namespace nano
{
        ///
        /// \brief create Himmelblau test functions
        ///
        /// https://en.wikipedia.org/wiki/Test_functions_for_optimization
        ///
        struct function_himmelblau_t : public function_t
        {
                virtual std::string name() const override final;
                virtual problem_t problem() const override final;
                virtual bool is_valid(const vector_t&) const override final;
                virtual bool is_minima(const vector_t& x, const scalar_t epsilon) const override final;
                virtual bool is_convex() const override final;
                virtual tensor_size_t min_dims() const override final;
                virtual tensor_size_t max_dims() const override final;
        };
}
