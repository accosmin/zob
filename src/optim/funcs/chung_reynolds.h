#pragma once

#include "function.h"

namespace nano
{
        ///
        /// \brief create Chung-Reynolds test functions
        ///
        struct NANO_PUBLIC function_chung_reynolds_t : public function_t
        {
                explicit function_chung_reynolds_t(const tensor_size_t dims);

                virtual std::string name() const override;
                virtual problem_t problem() const override;
                virtual bool is_valid(const vector_t&) const override;
                virtual bool is_minima(const vector_t& x, const scalar_t epsilon) const override;
                virtual bool is_convex() const override;
                virtual tensor_size_t min_dims() const override;
                virtual tensor_size_t max_dims() const override;

                tensor_size_t   m_dims;
        };
}