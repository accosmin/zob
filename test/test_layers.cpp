#include "nano.h"
#include "class.h"
#include "utest.hpp"
#include "math/random.hpp"
#include "math/numeric.hpp"
#include "math/epsilon.hpp"
#include "tensor/numeric.hpp"
#include "text/to_string.hpp"
#include "layers/make_layers.h"

using namespace nano;

const tensor_size_t cmd_idims = 3;
const tensor_size_t cmd_irows = 8;
const tensor_size_t cmd_icols = 8;
const tensor_size_t cmd_osize = 3;
const size_t cmd_tests = 7;

const string_t cmd_layer_output = make_output_layer(cmd_osize);

rloss_t get_loss()
{
        const strings_t loss_ids = get_losses().ids();

        const auto iloss = random_t<size_t>()();
        const auto loss_id = loss_ids[iloss % loss_ids.size()];

        return get_losses().get(loss_id);
}

rmodel_t get_model(const string_t& description)
{
        const auto model = get_models().get("forward-network", description + ";" + cmd_layer_output);
        model->resize(cmd_idims, cmd_irows, cmd_icols, cmd_osize, false);
        NANO_CHECK_EQUAL(model->idims(), cmd_idims);
        NANO_CHECK_EQUAL(model->irows(), cmd_irows);
        NANO_CHECK_EQUAL(model->icols(), cmd_icols);
        NANO_CHECK_EQUAL(model->osize(), cmd_osize);
        return model;
}

void make_random_config(tensor3d_t& inputs, vector_t& target)
{
        random_t<scalar_t> irgen(-scalar_t(1.0), +scalar_t(1.0));
        random_t<tensor_size_t> trgen(0, target.size() - 1);

        tensor::set_random(irgen, inputs);
        target = class_target(trgen(), target.size());
}

void test_model(const string_t& model_description, const scalar_t epsilon = epsilon2<scalar_t>())
{
        const auto model = get_model(model_description);
        const auto loss = get_loss();

        vector_t params(model->psize());
        vector_t target(model->osize());
        tensor3d_t inputs(model->idims(), model->irows(), model->icols());

        NANO_CHECK_EQUAL(model->osize(), cmd_osize);

        // optimization problem (wrt parameters & inputs): size
        auto fn_params_size = [&] ()
        {
                return model->psize();
        };

        // optimization problem (wrt parameters & inputs): function value
        auto fn_params_fval = [&] (const vector_t& x)
        {
                model->load_params(x);
                const vector_t output = model->output(inputs).vector();

                return loss->value(target, output);
        };

        // optimization problem (wrt parameters & inputs): function value & gradient
        auto fn_params_grad = [&] (const vector_t& x, vector_t& gx)
        {
                model->load_params(x);
                const vector_t output = model->output(inputs).vector();

                gx = model->gparam(loss->vgrad(target, output));
                return loss->value(target, output);
        };

        // optimization problem (wrt parameters & inputs): size
        auto fn_inputs_size = [&] ()
        {
                return model->isize();
        };

        // optimization problem (wrt parameters & inputs): function value
        auto fn_inputs_fval = [&] (const vector_t& x)
        {
                model->load_params(params);
                const auto output = model->output(tensor::map_tensor(x.data(), cmd_idims, cmd_irows, cmd_icols)).vector();

                return loss->value(target, output);
        };

        // optimization problem (wrt parameters & inputs): function value & gradient
        auto fn_inputs_grad = [&] (const vector_t& x, vector_t& gx)
        {
                model->load_params(params);
                const auto output = model->output(tensor::map_tensor(x.data(), cmd_idims, cmd_irows, cmd_icols)).vector();

                gx = model->ginput(loss->vgrad(target, output)).vector();
                return loss->value(target, output);
        };

        // construct optimization problem: analytic gradient vs finite difference approximation
        for (size_t t = 0; t < cmd_tests; ++ t)
        {
                make_random_config(inputs, target);
                model->random_params();
                NANO_CHECK(model->save_params(params));

                {
                        const problem_t problem(fn_params_size, fn_params_fval, fn_params_grad);
                        NANO_CHECK_LESS(problem.grad_accuracy(params), epsilon);
                }
                {
                        const problem_t problem(fn_inputs_size, fn_inputs_fval, fn_inputs_grad);
                        NANO_CHECK_LESS(problem.grad_accuracy(inputs.vector()), epsilon);
                }
        }
}

void compare_models(const string_t& model_description1, const string_t& model_description2,
        const scalar_t epsilon = epsilon1<scalar_t>())
{
        const auto model1 = get_model(model_description1);
        const auto model2 = get_model(model_description2);

        NANO_REQUIRE_EQUAL(model1->psize(), model2->psize());

        vector_t params(model1->psize());
        vector_t target(model1->psize());
        tensor3d_t inputs(model1->idims(), model1->irows(), model1->icols());

        for (size_t t = 0; t < cmd_tests; ++ t)
        {
                make_random_config(inputs, target);

                model1->random_params();
                model1->save_params(params);

                NANO_CHECK(model1->load_params(params));
                NANO_CHECK(model2->load_params(params));

                const auto output1 = model1->output(inputs).vector();
                const auto output2 = model2->output(inputs).vector();

                NANO_CHECK_EIGEN_CLOSE(output1, output2, epsilon);
        }
}

NANO_BEGIN_MODULE(test_layers)

NANO_CASE(activation)
{
        for (const auto& activation_id : { "act-unit", "act-tanh", "act-snorm", "act-splus" })
        {
                test_model(activation_id);
        }
}

NANO_CASE(affine)
{
        test_model(make_affine_layer(7));
}

NANO_CASE(convk2d)
{
        test_model(make_conv_layer("conv-k2d", 3, 3, 3, 1, "act-unit"));
        test_model(make_conv_layer("conv-k2d", 3, 3, 3, 1, "act-snorm"));
        test_model(make_conv_layer("conv-k2d", 3, 3, 3, 1, "act-splus"));
        test_model(make_conv_layer("conv-k2d", 3, 3, 3, 1, "act-tanh"));
}

NANO_CASE(convtoe)
{
        test_model(make_conv_layer("conv-toe", 3, 3, 3, 1, "act-unit"));
        test_model(make_conv_layer("conv-toe", 3, 3, 3, 1, "act-snorm"));
        test_model(make_conv_layer("conv-toe", 3, 3, 3, 1, "act-splus"));
        test_model(make_conv_layer("conv-toe", 3, 3, 3, 1, "act-tanh"));
}

NANO_CASE(conv_compare)
{
        const auto make_model = [] (const string_t& conv_type, const auto conn)
        {
                return  make_conv_layer(conv_type, 9, 3, 3, 1, "act-snorm") +
                        make_conv_layer(conv_type, 6, 3, 3, conn, "act-splus") +
                        make_affine_layer(5, "act-splus");
        };

        compare_models(make_model("conv-k2d", 1), make_model("conv-toe", 1));
        compare_models(make_model("conv-k2d", 3), make_model("conv-toe", 3));
}

NANO_CASE(multi_layer)
{
        test_model(
                make_affine_layer(7, "act-snorm") +
                make_affine_layer(5, "act-splus"));

        test_model(
                make_conv_layer("conv-toe", 7, 3, 3, 1, "act-snorm") +
                make_conv_layer("conv-toe", 4, 3, 3, 1, "act-splus"));

        test_model(
                make_conv_layer("conv-k2d", 7, 3, 3, 1, "act-snorm") +
                make_conv_layer("conv-k2d", 5, 3, 3, 1, "act-splus") +
                make_affine_layer(5, "act-splus"));

        test_model(
                make_conv_layer("conv-toe", 8, 3, 3, 1, "act-snorm") +
                make_conv_layer("conv-toe", 6, 3, 3, 2, "act-splus") +
                make_affine_layer(5, "act-splus"));

        test_model(
                make_conv_layer("conv-k2d", 8, 3, 3, 1, "act-snorm") +
                make_conv_layer("conv-k2d", 6, 3, 3, 2, "act-splus") +
                make_affine_layer(5, "act-splus"));

        test_model(
                make_conv_layer("conv-toe", 9, 3, 3, 1, "act-snorm") +
                make_conv_layer("conv-toe", 6, 3, 3, 3, "act-splus") +
                make_affine_layer(5, "act-splus"));
}

NANO_END_MODULE()
