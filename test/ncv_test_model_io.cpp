#include "nanocv.h"
#include "tasks/task_dummy.h"

int main(int argc, char *argv[])
{
        ncv::init();

        using namespace ncv;

        dummy_task_t task;
        task.set_rows(28);
        task.set_cols(28);
        task.set_color(color_mode::luma);
        task.set_outputs(10);
        task.set_folds(1);
        task.set_size(1000);
        task.setup();

        const size_t cmd_outputs = task.n_outputs();

        const string_t lmodel0;
        const string_t lmodel1 = lmodel0 + "linear:dims=100;act-snorm;";
        const string_t lmodel2 = lmodel1 + "linear:dims=100;act-snorm;";
        const string_t lmodel3 = lmodel2 + "linear:dims=100;act-snorm;";
        const string_t lmodel4 = lmodel3 + "linear:dims=100;act-snorm;";
        const string_t lmodel5 = lmodel4 + "linear:dims=100;act-snorm;";
        
        string_t cmodel;
        cmodel = cmodel + "conv:dims=16,rows=6,cols=6;act-snorm;pool-max;";
        cmodel = cmodel + "conv:dims=32,rows=5,cols=5;act-snorm;pool-max;";
        cmodel = cmodel + "conv:dims=64,rows=4,cols=4;act-snorm;";

        string_t rcmodel;
        rcmodel = rcmodel + "rconv:dims=16,rows=6,cols=6;act-snorm;pool-max;";
        rcmodel = rcmodel + "rconv:dims=32,rows=5,cols=5;act-snorm;pool-max;";
        rcmodel = rcmodel + "rconv:dims=64,rows=4,cols=4;act-snorm;";

        string_t mcmodel;
        mcmodel = mcmodel + "mconv:dims=16,rows=6,cols=6;act-snorm;pool-max;";
        mcmodel = mcmodel + "mconv:dims=32,rows=5,cols=5;act-snorm;pool-max;";
        mcmodel = mcmodel + "mconv:dims=64,rows=4,cols=4;act-snorm;";
        
        const string_t outlayer = "linear:dims=" + text::to_string(cmd_outputs) + ";";

        strings_t cmd_networks =
        {
                lmodel0 + outlayer,
                lmodel1 + outlayer,
                lmodel2 + outlayer,
                lmodel3 + outlayer,
                lmodel4 + outlayer,
                lmodel5 + outlayer,

                cmodel + outlayer,
                rcmodel + outlayer,
                mcmodel + outlayer
        };

        const rloss_t loss = loss_manager_t::instance().get("logistic");
        assert(loss);

        for (const string_t& cmd_network : cmd_networks)
        {
                log_info() << "<<< running network [" << cmd_network << "] ...";

                // create feed-forward network
                const rmodel_t model = model_manager_t::instance().get("forward-network", cmd_network);
                assert(model);
                model->resize(task, true);

                // test random networks
                const size_t n_tests = 64;
                for (size_t t = 0; t < n_tests; t ++)
                {
                        model->random_params();

                        const fold_t fold = {0, protocol::test };

                        const string_t header = "test [" + text::to_string(t + 1) + "/" + text::to_string(n_tests) + "] ";
                        const string_t path = "./test_model_io.test";

                        // test error before saving
                        scalar_t lvalue_before, lerror_before;
                        const size_t lcount_before = ncv::test(task, fold, *loss, *model, lvalue_before, lerror_before);

                        ncv::measure_critical_call(
                                [&] () { return model->save(path); },
                                header + "model saved",
                                header + "failed to save model to <" + path + ">!");

                        ncv::measure_critical_call(
                                [&] () { return model->load(path); },
                                header + "model loaded",
                                header + "failed to load model from <" + path + ">!");

                        // test error after loading
                        scalar_t lvalue_after, lerror_after;
                        const size_t lcount_after = ncv::test(task, fold, *loss, *model, lvalue_after, lerror_after);

                        const bool ok =
                                lcount_before == lcount_after &&
                                lvalue_before == lvalue_after &&
                                lerror_before == lerror_before;

                        (ok ? log_info() : log_error())
                                << header << "IO " << (ok ? "passed" : "failed")
                                << ": count = " << lcount_before << "/" << lcount_after
                                << ", value = " << lvalue_before << "/" << lvalue_after
                                << ", error = " << lerror_before << "/" << lerror_after
                                << (ok ? "." : "!");
                }

                log_info();
        }

        // OK
        log_info() << done;
        return EXIT_SUCCESS;
}
