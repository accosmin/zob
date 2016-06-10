#include "batch.h"
#include "loop.hpp"
#include "cortex/model.h"
#include "math/clamp.hpp"
#include "optim/batch.hpp"
#include "cortex/logger.h"
#include "text/to_string.hpp"
#include "text/from_params.hpp"

namespace nano
{
        batch_trainer_t::batch_trainer_t(const string_t& parameters) :
                trainer_t(parameters)
        {
        }

        trainer_result_t batch_trainer_t::train(
                const task_t& task, const size_t fold, const size_t nthreads,
                const loss_t& loss, const criterion_t& criterion,
                model_t& model) const
        {
                // initialize the model
                model.resize(task, true);
                model.random_params();

                // parameters
                const auto iterations = clamp(from_params<size_t>(configuration(), "iters", 1024), 4, 4096);
                const auto epsilon = clamp(from_params<scalar_t>(configuration(), "eps", scalar_t(1e-6)), scalar_t(1e-8), scalar_t(1e-3));
                const auto optimizer = from_params<batch_optimizer>(configuration(), "opt", batch_optimizer::LBFGS);
                const auto policy = from_params<trainer_policy>(configuration(), "policy", trainer_policy::stop_early);
                const auto verbose = true;

                // train the model
                const auto op = [&] (const accumulator_t& lacc, const accumulator_t& gacc, const vector_t& x0)
                {
                        return train(task, fold, lacc, gacc, x0, optimizer, iterations, epsilon, policy, verbose);
                };

                const auto result = trainer_loop(model, nthreads, loss, criterion, op);
                log_info() << "<<< batch-" << to_string(optimizer) << ": " << result << ".";

                // OK
                if (result.valid())
                {
                        model.load_params(result.optimum_params());
                }
                return result;
        }

        trainer_result_t batch_trainer_t::train(
                const task_t& task, const size_t fold,
                const accumulator_t& lacc, const accumulator_t& gacc, const vector_t& x0,
                const batch_optimizer optimizer, const size_t iterations, const scalar_t epsilon,
                const trainer_policy policy, const bool verbose) const
        {
                const timer_t timer;

                const auto train_fold = fold_t{fold, protocol::train};
                const auto valid_fold = fold_t{fold, protocol::valid};
                const auto test_fold = fold_t{fold, protocol::test};

                size_t iteration = 0;
                trainer_result_t result;

                // construct the optimization problem
                const auto fn_size = [&] ()
                {
                        return lacc.psize();
                };

                const auto fn_fval = [&] (const vector_t& x)
                {
                        lacc.set_params(x);
                        lacc.update(task, train_fold);
                        return lacc.value();
                };

                const auto fn_grad = [&] (const vector_t& x, vector_t& gx)
                {
                        gacc.set_params(x);
                        gacc.update(task, train_fold);
                        gx = gacc.vgrad();
                        return gacc.value();
                };

                auto fn_ulog = [&] (const state_t& state)
                {
                        // evaluate the current state
                        lacc.set_params(state.x);

                        lacc.update(task, train_fold);
                        const auto train = trainer_measurement_t{lacc.value(), lacc.avg_error(), lacc.var_error()};

                        lacc.update(task, valid_fold);
                        const auto valid = trainer_measurement_t{lacc.value(), lacc.avg_error(), lacc.var_error()};

                        lacc.update(task, test_fold);
                        const auto test = trainer_measurement_t{lacc.value(), lacc.avg_error(), lacc.var_error()};

                        // OK, update the optimum solution
                        const auto milis = timer.milliseconds();
                        const auto config = trainer_config_t{{"lambda", lacc.lambda()}};
                        const auto ret = result.update(state.x, {milis, ++iteration, train, valid, test}, config);

                        if (verbose)
                        {
                                log_info()
                                        << "[" << iteration << "/" << iterations
                                        << ": train=" << train
                                        << ", valid=" << valid << "|" << nano::to_string(ret)
                                        << ", test=" << test
                                        << ", " << config << ",calls=" << state.m_fcalls << "/" << state.m_gcalls
                                        << "] " << timer.elapsed() << ".";
                        }

                        return !nano::is_done(ret, policy);
                };

                // assembly optimization problem & optimize the model
                nano::minimize(
                        problem_t(fn_size, fn_fval, fn_grad), fn_ulog,
                        x0, optimizer, iterations, epsilon);

                return result;
        }
}