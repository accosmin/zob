#include "model.h"
#include "loop.hpp"
#include "stochastic.h"
#include "optim/stoch.h"
#include "task_iterator.h"
#include "math/clamp.hpp"
#include "math/numeric.hpp"
#include "text/to_string.hpp"
#include "text/from_params.hpp"

#include "logger.h"

namespace nano
{
        stochastic_trainer_t::stochastic_trainer_t(const string_t& parameters) :
                trainer_t(parameters)
        {
        }

        trainer_result_t stochastic_trainer_t::train(
                const task_t& task, const size_t fold, const size_t nthreads,
                const loss_t& loss, const criterion_t& crition,
                model_t& model) const
        {
                if (model != task)
                {
                        throw std::runtime_error("stochastic trainer: mis-matching model and task");
                }

                // parameters
                const auto epochs = clamp(from_params<size_t>(configuration(), "epochs", 16), 1, 1024);
                const auto optimizer = from_params<stoch_optimizer>(configuration(), "opt", stoch_optimizer::SG);
                const auto policy = from_params<trainer_policy>(configuration(), "policy", trainer_policy::stop_early);
                const auto verbose = true;

                // train the model
                const auto op = [&] (const accumulator_t& lacc, const accumulator_t& gacc, const vector_t& x0)
                {
                        return train(task, fold, lacc, gacc, x0, optimizer, epochs, policy, verbose);
                };

                const auto result = trainer_loop(model, nthreads, loss, crition, op);
                log_info() << "<<< stoch-" << to_string(optimizer) << ": " << result << ".";

                // OK
                if (result.valid())
                {
                        model.load_params(result.optimum_params());
                }
                return result;
        }

        trainer_result_t stochastic_trainer_t::train(
                const task_t& task, const size_t fold,
                const accumulator_t& lacc, const accumulator_t& gacc, const vector_t& x0,
                const stoch_optimizer optimizer, const size_t epochs,
                const trainer_policy policy, const bool verbose) const
        {
                const timer_t timer;

                const auto train_fold = fold_t{fold, protocol::train};
                const auto valid_fold = fold_t{fold, protocol::valid};
                const auto test_fold = fold_t{fold, protocol::test};

                const auto train_size = task.n_samples(train_fold);
                const auto batch_size = 16 * nano::logical_cpus();
                const auto epoch_size = idiv(train_size, batch_size);

                size_t epoch = 0;
                trainer_result_t result;

                task_iterator_t it(task, train_fold, batch_size);

                // construct the optimization problem
                const auto fn_size = [&] ()
                {
                        return lacc.psize();
                };

                const auto fn_fval = [&] (const vector_t& x)
                {
                        it.next();
                        lacc.set_params(x);
                        lacc.update(task, it.fold(), it.begin(), it.end());
                        return lacc.value();
                };

                const auto fn_grad = [&] (const vector_t& x, vector_t& gx)
                {
                        it.next();
                        gacc.set_params(x);
                        gacc.update(task, it.fold(), it.begin(), it.end());
                        gx = gacc.vgrad();
                        return gacc.value();
                };

                auto fn_tlog = [&] (const state_t& state, const trainer_config_t& sconfig)
                {
                        lacc.set_params(state.x);
                        lacc.update(task, train_fold);
                        const auto train = trainer_measurement_t{lacc.value(), lacc.avg_error(), lacc.var_error()};

                        const auto config = nano::append(sconfig, "lambda", lacc.lambda());

                        if (verbose)
                        {
                                log_info()
                                        << "tune: train=" << train
                                        << ", " << config << ",batch=" << batch_size
                                        << "] " << timer.elapsed() << ".";
                        }

                        return train.m_value;
                };

                auto fn_ulog = [&] (const state_t& state, const trainer_config_t& sconfig)
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
                        const auto config = nano::append(sconfig, "lambda", lacc.lambda());
                        const auto ret = result.update(state, {milis, ++epoch, train, valid, test}, config);

                        if (verbose)
                        {
                                log_info()
                                        << "[" << epoch << "/" << epochs
                                        << ": train=" << train
                                        << ", valid=" << valid << "|" << nano::to_string(ret)
                                        << ", test=" << test
                                        << ", " << config << ",batch=" << batch_size
                                        << "] " << timer.elapsed() << ".";
                        }

                        return !nano::is_done(ret, policy);
                };

                // assembly optimization problem & optimize the model
                nano::minimize(
                        stoch_params_t(epochs, epoch_size, optimizer, fn_ulog, fn_tlog),
                        problem_t(fn_size, fn_fval, fn_grad), x0);

                return result;
        }
}
