#include "tensor/momentum.h"
#include "text/json_writer.h"
#include "solver_stoch_adam.h"

using namespace nano;

solver_state_t stoch_adam_t::minimize(const stoch_params_t& param, const function_t& function, const vector_t& x0) const
{
        const auto beta1s = make_finite_space(scalar_t(0.90));
        const auto beta2s = make_finite_space(scalar_t(0.99));
        return tune(this, param, function, x0, make_alpha0s(), make_epsilons(), beta1s, beta2s);
}

solver_state_t stoch_adam_t::minimize(const stoch_params_t& param, const function_t& function, const vector_t& x0,
        const scalar_t alpha0, const scalar_t epsilon, const scalar_t beta1, const scalar_t beta2)
{
        // first-order momentum of the gradient
        momentum_t<vector_t> m(beta1, x0.size());

        // second-order momentum of the gradient
        momentum_t<vector_t> v(beta2, x0.size());

        // assembly the solver
        const auto solver = [&] (solver_state_t& cstate, const solver_state_t&)
        {
                // learning rate
                const scalar_t alpha = alpha0;

                // descent direction
                m.update(cstate.g);
                v.update(cstate.g.array().square());

                cstate.d = -m.value().array() / (epsilon + v.value().array().sqrt());

                // update solution
                function.stoch_next();
                cstate.stoch_update(function, alpha);
        };

        const auto snapshot = [&] (const solver_state_t& cstate, solver_state_t& sstate)
        {
                sstate.update(function, cstate.x);
        };

        return  loop(param, function, x0, solver, snapshot,
                json_writer_t().object("alpha0", alpha0, "epsilon", epsilon, "beta1", beta1, "beta2", beta2).str());
}
