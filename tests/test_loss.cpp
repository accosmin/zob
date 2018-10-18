#include "loss.h"
#include "utest.h"
#include "cortex.h"
#include "function.h"
#include "core/random.h"
#include "tensor/numeric.h"

using namespace nano;

struct loss_function_t final : public function_t
{
        loss_function_t(const rloss_t& loss, const tensor_size_t xmaps) :
                function_t("loss", xmaps, xmaps, xmaps, convexity::no),
                m_loss(loss), m_target(xmaps, 1, 1)
        {
                m_target.vector() = class_target(11 % xmaps, xmaps);
        }

        scalar_t vgrad(const vector_t& x, vector_t* gx = nullptr) const override
        {
                assert(x.size() == m_target.size());
                const auto output = map_tensor(x.data(), m_target.dims());

                if (gx)
                {
                        const auto grads = m_loss->vgrad(m_target, output);
                        assert(gx->size() == grads.size());
                        assert(grads.array().isFinite().all());

                        *gx = grads.vector();
                }

                const auto value = m_loss->value(m_target, output);
                assert(std::isfinite(value));
                return value;
        }

        const rloss_t&          m_loss;
        tensor3d_t              m_target;
};

NANO_BEGIN_MODULE(test_loss)

NANO_CASE(gradient)
{
        const tensor_size_t cmd_min_dims = 2;
        const tensor_size_t cmd_max_dims = 8;
        const size_t cmd_tests = 128;

        // evaluate the analytical gradient vs. the finite difference approximation
        for (const auto& loss_id : get_losses().ids())
        {
                for (tensor_size_t cmd_dims = cmd_min_dims; cmd_dims <= cmd_max_dims; ++ cmd_dims)
                {
                        const auto loss = get_losses().get(loss_id);
                        const auto function = loss_function_t(loss, cmd_dims);

                        for (size_t t = 0; t < cmd_tests; ++ t)
                        {
                                vector_t x = vector_t::Random(cmd_dims) / 10;

                                NANO_CHECK_GREATER(function.vgrad(x), scalar_t(0));
                                NANO_CHECK_LESS(function.grad_accuracy(x), epsilon2<scalar_t>());
                        }
                }
        }
}

NANO_CASE(single_class)
{
        for (const auto& loss_id : {"classnll", "s-logistic", "s-exponential", "s-square", "s-cauchy"})
        {
                const auto loss = get_losses().get(loss_id);
                NANO_REQUIRE(loss);

                const auto n_classes = 13;
                tensor3d_t target(n_classes, 1, 1);
                tensor3d_t output(n_classes, 1, 1);

                {
                        target.vector() = class_target(11, n_classes);
                        output.vector() = class_target(11, n_classes);

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(0), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(11, n_classes);
                        output.vector() = class_target(12, n_classes);

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(11, n_classes);
                        output.vector() = class_target(11, n_classes);
                        output.vector()(7) = pos_target() + 1;

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(11, n_classes);
                        output.vector() = class_target(-1, n_classes);

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
                }
        }
}

NANO_CASE(multi_class)
{
        for (const auto& loss_id : {"m-logistic", "m-exponential", "m-square", "m-cauchy"})
        {
                const auto loss = get_losses().get(loss_id);
                NANO_REQUIRE(loss);

                const auto n_classes = 13;
                tensor3d_t target(n_classes, 1, 1);
                tensor3d_t output(n_classes, 1, 1);

                {
                        target.vector() = class_target(n_classes);
                        output.vector() = class_target(n_classes);

                        target.vector()(7) = target.vector()(9) = pos_target();
                        output.vector()(7) = output.vector()(9) = pos_target();

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(0), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(n_classes);
                        output.vector() = class_target(n_classes);

                        target.vector()(7) = target.vector()(9) = pos_target();

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(2), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(n_classes);
                        output.vector() = class_target(n_classes);

                        target.vector()(7) = target.vector()(9) = pos_target();
                        output.vector()(5) = pos_target();

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(3), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(n_classes);
                        output.vector() = class_target(n_classes);

                        target.vector()(7) = target.vector()(9) = pos_target();
                        output.vector()(7) = pos_target();

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(n_classes);
                        output.vector() = class_target(n_classes);

                        target.vector()(7) = target.vector()(9) = pos_target();
                        output.vector()(5) = output.vector()(9) = pos_target();

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(2), epsilon0<scalar_t>());
                }
                {
                        target.vector() = class_target(n_classes);
                        output.vector() = class_target(n_classes);

                        target.vector()(7) = target.vector()(9) = pos_target();
                        output.vector()(7) = output.vector()(9) = output.vector()(11) = pos_target();

                        const auto error = loss->error(target, output);
                        NANO_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
                }
        }
}

NANO_CASE(regression)
{
        for (const auto& loss_id : {"square", "cauchy"})
        {
                const auto loss = get_losses().get(loss_id);
                NANO_REQUIRE(loss);

                tensor3d_t target(4, 1, 1);
                target.random();

                tensor3d_t output = target;

                const auto error = loss->error(target, output);
                NANO_CHECK_LESS(error, epsilon0<scalar_t>());
        }
}

NANO_END_MODULE()
