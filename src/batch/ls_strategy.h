#pragma once

#include "ls_backtrack.h"
#include "ls_cgdescent.h"
#include "ls_interpolate.h"

namespace nano
{
        class ls_strategy_t
        {
        public:

                ///
                /// \brief constructor
                ///
                ls_strategy_t(  const ls_strategy strategy,
                                const scalar_t c1 = scalar_t(1e-4), const scalar_t c2 = scalar_t(0.1));

                ///
                /// \brief update the current state
                ///
                bool operator()(const problem_t& problem, const scalar_t t0, state_t& state) const;

        private:

                bool setup(const problem_t&, const ls_step_t& step0, const ls_step_t& step, state_t& state) const;
                bool setup(const problem_t&, const ls_step_t& step, state_t& state) const;

        private:

                // attributes
                ls_strategy             m_strategy;     ///<
                scalar_t                m_c1;           ///< sufficient decrease rate
                scalar_t                m_c2;           ///< sufficient curvature

                ls_cgdescent_t          m_ls_cgdescent;
                ls_backtrack_t          m_ls_backtrack;
                ls_interpolate_t        m_ls_interpolate;
        };
}
