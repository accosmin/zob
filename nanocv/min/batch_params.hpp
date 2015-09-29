#pragma once

#include "params.hpp"
#include "linesearch.h"

namespace min
{
        ///
        /// \brief common parameters for batch optimization
        ///
        template
        <
                typename tproblem                       ///< optimization problem
        >
        struct batch_params_t : public params_t<tproblem>
        {
                typedef typename tproblem::tscalar      tscalar;
                typedef typename tproblem::tsize        tsize;
                typedef typename tproblem::tvector      tvector;
                typedef typename tproblem::tstate       tstate;
                typedef typename tproblem::top_ulog     top_ulog;

                ///
                /// \brief constructor
                ///
                batch_params_t( tsize max_iterations,
                                tscalar epsilon,
                                ls_initializer lsinit,
                                ls_strategy lsstrat,
                                const top_ulog& u = top_ulog())
                        :       params_t<tproblem>(u),
                                m_max_iterations(max_iterations),
                                m_epsilon(epsilon),
                                m_ls_initializer(lsinit),
                                m_ls_strategy(lsstrat)
                {
                }

                ///
                /// \brief destructor
                ///
                virtual ~batch_params_t()
                {
                }

                // attributes
                tsize           m_max_iterations;       ///< maximum number of iterations
                tscalar         m_epsilon;              ///< convergence precision

                ls_initializer  m_ls_initializer;       ///< line-search step length initialization strategy
                ls_strategy     m_ls_strategy;          ///< line-search step length selection strategy
        };
}

