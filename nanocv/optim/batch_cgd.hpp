#pragma once

#include "ls_init.hpp"
#include "ls_strategy.hpp"
#include "batch_params.hpp"
#include "batch_cgd_steps.hpp"

namespace ncv
{
        namespace optim
        {
                ///
                /// \brief conjugate gradient descent
                ///
                template
                <
                        typename tcgd_update,                   ///< CGD step update
                        typename tproblem                       ///< optimization problem
                >
                struct batch_cgd_t
                {
                        typedef batch_params_t<tproblem>        param_t;
                        typedef typename param_t::tscalar       tscalar;
                        typedef typename param_t::tsize         tsize;
                        typedef typename param_t::tvector       tvector;
                        typedef typename param_t::tstate        tstate;
                        typedef typename param_t::twlog         twlog;
                        typedef typename param_t::telog         telog;
                        typedef typename param_t::tulog         tulog;

                        ///
                        /// \brief constructor
                        ///
                        batch_cgd_t(    tsize max_iterations,
                                        tscalar epsilon,
                                        ls_initializer lsinit,
                                        ls_strategy lsstrat,
                                        const twlog& wlog = twlog(),
                                        const telog& elog = telog(),
                                        const tulog& ulog = tulog())
                                :       m_param(max_iterations, epsilon, lsinit, lsstrat, wlog, elog, ulog)
                        {
                        }

                        ///
                        /// \brief minimize starting from the initial guess x0
                        ///
                        tstate operator()(const tproblem& problem, const tvector& x0) const
                        {
                                assert(problem.size() == static_cast<tsize>(x0.size()));

                                tstate cstate(problem, x0);     // current state
                                tstate pstate = cstate;         // previous state

                                // line-search initial step length
                                linesearch_init_t<tstate> ls_init(m_param.m_ls_initializer);

                                // line-search step
                                linesearch_strategy_t<tproblem> ls_step(m_param.m_ls_strategy, 1e-4, 0.1);

                                const tcgd_update op_update;

                                // iterate until convergence
                                for (tsize i = 0; i < m_param.m_max_iterations && m_param.ulog(cstate); i ++)
                                {
                                        // check convergence
                                        if (cstate.converged(m_param.m_epsilon))
                                        {
                                                break;
                                        }

                                        // descent direction
                                        if (i == 0)
                                        {
                                                cstate.d = -cstate.g;
                                        }
                                        else
                                        {
                                                const tscalar beta = op_update(pstate, cstate);
                                                cstate.d = -cstate.g + beta * pstate.d;
                                        }

                                        if (cstate.d.dot(cstate.g) > tscalar(0))
                                        {
                                                cstate.d = -cstate.g;
                                                m_param.wlog("not a descent direction (CGD)!");
                                        }

                                        // line-search
                                        pstate = cstate;

                                        const tscalar t0 = ls_init(cstate);
                                        if (!ls_step.update(problem, t0, cstate))
                                        {
                                                m_param.elog("line-search failed (CGD)!");
                                                break;
                                        }
                                }

                                return cstate;
                        }

                        // attributes
                        param_t         m_param;
                };

                // create various CGD algorithms
                template <typename tproblem>
                using batch_cgd_hs_t = batch_cgd_t<cgd_step_HS<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_fr_t = batch_cgd_t<cgd_step_FR<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_prp_t = batch_cgd_t<cgd_step_PRP<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_cd_t = batch_cgd_t<cgd_step_CD<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_ls_t = batch_cgd_t<cgd_step_LS<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_dy_t = batch_cgd_t<cgd_step_DY<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_n_t = batch_cgd_t<cgd_step_N<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_dycd_t = batch_cgd_t<cgd_step_DYCD<typename tproblem::tstate>, tproblem>;

                template <typename tproblem>
                using batch_cgd_dyhs_t = batch_cgd_t<cgd_step_DYHS<typename tproblem::tstate>, tproblem>;
        }
}
