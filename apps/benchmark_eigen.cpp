#include "tensor.h"
#include "logger.h"
#include "measure.hpp"
#include "text/table.h"
#include "text/cmdline.h"
#include "math/random.hpp"
#include "tensor/numeric.hpp"
#include "text/table_row_mark.h"
#ifdef NANO_WITH_OPENCL
#include "opencl/ocl.h"
#endif
#include <iostream>

namespace
{
        using namespace nano;

        auto rng_value = nano::make_rng(scalar_t(-1e-3), scalar_t(+1e-3));
        const size_t trials = 16;

        scalar_t make_scalar()
        {
                return rng_value();
        }

        vector_t make_vector(const tensor_size_t dims)
        {
                vector_t x(dims);
                tensor::set_random(rng_value, x);
                return x;
        }

        matrix_t make_matrix(const tensor_size_t rows, const tensor_size_t cols)
        {
                matrix_t x(rows, cols);
                tensor::set_random(rng_value, x);
                return x;
        }

        auto measure_vpc(const tensor_size_t dims)
        {
                auto x = make_vector(dims);
                auto c = make_scalar();
                auto z = make_vector(dims);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        z = x.array() + c;
                }, trials);

                return nano::gflops(dims, duration);
        }

#ifdef NANO_WITH_OPENCL
        auto measure_vpc_ocl(const tensor_size_t dims)
        {
                auto x = make_vector(dims);
                auto c = make_scalar();
                auto z = make_vector(dims);

                cl::Buffer xbuffer = ocl::make_buffer(x, CL_MEM_READ_WRITE);
                cl::Buffer zbuffer = ocl::make_buffer(z, CL_MEM_READ_ONLY);
                cl::Kernel kernel = ocl::make_kernel("vpc");

                ocl::set_args(kernel, xbuffer, c, zbuffer, dims);
                ocl::write(xbuffer, x);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        ocl::wait(ocl::enqueue(kernel, dims));
                }, trials);

                ocl::read(zbuffer, z);

                return nano::gflops(dims, duration);
        }
#endif

        auto measure_vpv(const tensor_size_t dims)
        {
                auto x = make_vector(dims);
                auto y = make_vector(dims);
                auto z = make_vector(dims);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        z = x + y;
                }, trials);

                return nano::gflops(dims, duration);
        }

#ifdef NANO_WITH_OPENCL
        auto measure_vpv_ocl(const tensor_size_t dims)
        {
                auto x = make_vector(dims);
                auto y = make_vector(dims);
                auto z = make_vector(dims);

                cl::Buffer xbuffer = ocl::make_buffer(x, CL_MEM_READ_WRITE);
                cl::Buffer ybuffer = ocl::make_buffer(y, CL_MEM_READ_WRITE);
                cl::Buffer zbuffer = ocl::make_buffer(z, CL_MEM_READ_ONLY);
                cl::Kernel kernel = ocl::make_kernel("vpv");

                ocl::set_args(kernel, xbuffer, ybuffer, zbuffer, dims);
                ocl::write(xbuffer, x);
                ocl::write(ybuffer, y);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        ocl::wait(ocl::enqueue(kernel, dims));
                }, trials);

                ocl::read(zbuffer, z);

                return nano::gflops(dims, duration);
        }
#endif

        auto measure_vcpvc(const tensor_size_t dims)
        {
                auto x = make_vector(dims); auto a = make_scalar();
                auto y = make_vector(dims); auto b = make_scalar();
                auto z = make_vector(dims);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        z = x.array() * a + y.array() * b;
                }, trials);

                return nano::gflops(3 * dims, duration);
        }

#ifdef NANO_WITH_OPENCL
        auto measure_vcpvc_ocl(const tensor_size_t dims)
        {
                auto x = make_vector(dims); auto a = make_scalar();
                auto y = make_vector(dims); auto b = make_scalar();
                auto z = make_vector(dims);

                cl::Buffer xbuffer = ocl::make_buffer(x, CL_MEM_READ_WRITE);
                cl::Buffer ybuffer = ocl::make_buffer(y, CL_MEM_READ_WRITE);
                cl::Buffer zbuffer = ocl::make_buffer(z, CL_MEM_READ_ONLY);
                cl::Kernel kernel = ocl::make_kernel("vcpvc");

                ocl::set_args(kernel, xbuffer, a, ybuffer, b, zbuffer, dims);
                ocl::write(xbuffer, x);
                ocl::write(ybuffer, y);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        ocl::wait(ocl::enqueue(kernel, dims));
                }, trials);

                ocl::read(zbuffer, z);

                return nano::gflops(3 * dims, duration);
        }
#endif

        auto measure_mv(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto x = make_vector(dims);
                auto z = make_vector(dims);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        z = A * x;
                }, trials);

                return nano::gflops(dims * dims, duration);
        }

#ifdef NANO_WITH_OPENCL
        auto measure_mv_ocl(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto x = make_vector(dims);
                auto z = make_vector(dims);

                cl::Buffer Abuffer = ocl::make_buffer(A, CL_MEM_READ_WRITE);
                cl::Buffer xbuffer = ocl::make_buffer(x, CL_MEM_READ_WRITE);
                cl::Buffer zbuffer = ocl::make_buffer(z, CL_MEM_READ_ONLY);
                cl::Kernel kernel = ocl::make_kernel("mv");

                ocl::set_args(kernel, Abuffer, xbuffer, zbuffer, dims, dims);
                ocl::write(Abuffer, A);
                ocl::write(xbuffer, x);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        ocl::wait(ocl::enqueue(kernel, dims));
                }, trials);

                ocl::read(zbuffer, z);

                return nano::gflops(dims * dims, duration);
        }
#endif

        auto measure_mvpc(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto x = make_vector(dims);
                auto c = make_scalar();
                auto z = make_vector(dims);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        z = (A * x).array() + c;
                }, trials);

                return nano::gflops(dims * dims + dims, duration);
        }

#ifdef NANO_WITH_OPENCL
        auto measure_mvpc_ocl(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto x = make_vector(dims);
                auto c = make_scalar();
                auto z = make_vector(dims);

                cl::Buffer Abuffer = ocl::make_buffer(A, CL_MEM_READ_WRITE);
                cl::Buffer xbuffer = ocl::make_buffer(x, CL_MEM_READ_WRITE);
                cl::Buffer zbuffer = ocl::make_buffer(z, CL_MEM_READ_ONLY);
                cl::Kernel kernel = ocl::make_kernel("mvpc");

                ocl::set_args(kernel, Abuffer, xbuffer, c, zbuffer, dims, dims);
                ocl::write(Abuffer, A);
                ocl::write(xbuffer, x);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        ocl::wait(ocl::enqueue(kernel, dims));
                }, trials);

                ocl::read(zbuffer, z);

                return nano::gflops(dims * dims + dims, duration);
        }
#endif

        auto measure_mvpv(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto x = make_vector(dims);
                auto y = make_vector(dims);
                auto z = make_vector(dims);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        z = A * x + y;
                }, trials);

                return nano::gflops(dims * dims + dims, duration);
        }

#ifdef NANO_WITH_OPENCL
        auto measure_mvpv_ocl(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto x = make_vector(dims);
                auto y = make_vector(dims);
                auto z = make_vector(dims);

                cl::Buffer Abuffer = ocl::make_buffer(A, CL_MEM_READ_WRITE);
                cl::Buffer xbuffer = ocl::make_buffer(x, CL_MEM_READ_WRITE);
                cl::Buffer ybuffer = ocl::make_buffer(y, CL_MEM_READ_WRITE);
                cl::Buffer zbuffer = ocl::make_buffer(z, CL_MEM_READ_ONLY);
                cl::Kernel kernel = ocl::make_kernel("mvpv");

                ocl::set_args(kernel, Abuffer, xbuffer, ybuffer, zbuffer, dims, dims);
                ocl::write(Abuffer, A);
                ocl::write(xbuffer, x);
                ocl::write(ybuffer, y);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        ocl::wait(ocl::enqueue(kernel, dims));
                }, trials);

                ocl::read(zbuffer, z);

                return nano::gflops(dims * dims + dims, duration);
        }
#endif

        auto measure_mm(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto B = make_matrix(dims, dims);
                auto Z = make_matrix(dims, dims);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        Z = A * B;
                }, trials);

                return nano::gflops(dims * dims * dims, duration);
        }

#ifdef NANO_WITH_OPENCL
        auto measure_mm_ocl(const tensor_size_t dims)
        {
                auto A = make_matrix(dims, dims);
                auto B = make_matrix(dims, dims);
                auto Z = make_matrix(dims, dims);

                cl::Buffer Abuffer = ocl::make_buffer(A, CL_MEM_READ_WRITE);
                cl::Buffer Bbuffer = ocl::make_buffer(B, CL_MEM_READ_WRITE);
                cl::Buffer Zbuffer = ocl::make_buffer(Z, CL_MEM_READ_ONLY);
                cl::Kernel kernel = ocl::make_kernel("mm");

                ocl::set_args(kernel, Abuffer, Bbuffer, Zbuffer, dims, dims, dims);
                ocl::write(Abuffer, A);
                ocl::write(Bbuffer, B);

                const auto duration = nano::measure_robustly_psec([&] ()
                {
                        ocl::wait(ocl::enqueue(kernel, dims, dims));
                }, trials);

                ocl::read(Zbuffer, Z);

                return nano::gflops(dims * dims * dims, duration);
        }
#endif
}

int main(int argc, const char* argv[])
{
        using namespace nano;

        // parse the command line
        cmdline_t cmdline("benchmark linear algebra operations using Eigen and OpenCL (if available)");
        cmdline.add("", "min-dims",     "minimum number of dimensions [1, 1024]", "8");
        cmdline.add("", "max-dims",     "maximum number of dimensions [1, 4096]", "1024");
        cmdline.add("", "level1",       "benchmark level1 operations (vector-vector)");
        cmdline.add("", "level2",       "benchmark level2 operations (matrix-vector)");
        cmdline.add("", "level3",       "benchmark level3 operations (matrix-matrix)");

        cmdline.process(argc, argv);

        // check arguments and options
        const auto min_dims = clamp(cmdline.get<tensor_size_t>("min-dims"), tensor_size_t(1), tensor_size_t(1024));
        const auto max_dims = clamp(cmdline.get<tensor_size_t>("max-dims"), min_dims, tensor_size_t(4096));
        const auto level1 = cmdline.has("level1");
        const auto level2 = cmdline.has("level2");
        const auto level3 = cmdline.has("level3");

        if (!level1 && !level2 && !level3)
        {
                cmdline.usage();
        }

#ifdef NANO_WITH_OPENCL
        try
        {
        // initialize OpenCL context
        ocl::select(CL_DEVICE_TYPE_GPU);
#endif

        const auto foreach_dims = [&] (const auto min, const auto max, const auto& op)
        {
                for (tensor_size_t dims = min; dims <= max; dims *= 2)
                {
                        op(dims);
                }
        };
        const auto fillrow = [&] (const auto min, const auto max, auto&& row, const auto& op)
        {
                foreach_dims(min, max, [&] (const tensor_size_t dims)
                {
                        row << op(dims);
                });
        };
        const auto fillheader = [&] (const auto min, const auto max, table_t& table)
        {
                table.header() << "platform";
                foreach_dims(min, max, [&] (const tensor_size_t dims)
                {
                        const auto kilo = tensor_size_t(1) << 10;
                        const auto mega = tensor_size_t(1) << 20;
                        const auto value = (dims < kilo) ? dims : (dims < mega ? (dims / kilo) : (dims / mega));
                        const auto units = (dims < kilo) ? string_t("") : (dims < mega ? string_t("K") : string_t("M"));
                        table.header() << (to_string(value) + units + " [GFLOPS]");
                });
        };

        if (level1)
        {
                const auto min = 1024 * min_dims;
                const auto max = 1024 * max_dims;

                table_t table("operation");
                fillheader(min, max, table);
                fillrow(min, max, table.append("z = x + c") << "CPU", measure_vpc);
#ifdef NANO_WITH_OPENCL
                fillrow(min, max, table.append("z = x + c") << "OpenCL", measure_vpc_ocl);
#endif
                fillrow(min, max, table.append("z = x + y") << "CPU", measure_vpv);
#ifdef NANO_WITH_OPENCL
                fillrow(min, max, table.append("z = x + y") << "OpenCL", measure_vpv_ocl);
#endif
                fillrow(min, max, table.append("z = a * x + b * y") << "CPU", measure_vcpvc);
#ifdef NANO_WITH_OPENCL
                fillrow(min, max, table.append("z = a * x + b * y") << "OpenCL", measure_vcpvc_ocl);
#endif
                table.print(std::cout);
        }
        if (level2)
        {
                const auto min = min_dims;
                const auto max = max_dims;

                table_t table("operation");
                fillheader(min, max, table);
                fillrow(min, max, table.append("z = A * x") << "CPU", measure_mv);
#ifdef NANO_WITH_OPENCL
                fillrow(min, max, table.append("z = A * x") << "OpenCL", measure_mv_ocl);
#endif
                fillrow(min, max, table.append("z = A * x + c") << "CPU", measure_mvpc);
#ifdef NANO_WITH_OPENCL
                fillrow(min, max, table.append("z = A * x + c") << "OpenCL", measure_mvpc_ocl);
#endif
                fillrow(min, max, table.append("z = A * x + y") << "CPU", measure_mvpv);
#ifdef NANO_WITH_OPENCL
                fillrow(min, max, table.append("z = A * x + y") << "OpenCL", measure_mvpv_ocl);
#endif
                table.print(std::cout);
        }
        if (level3)
        {
                const auto min = min_dims;
                const auto max = max_dims;

                table_t table("operation");
                fillheader(min, max, table);
                fillrow(min, max, table.append("Z = A * B") << "CPU", measure_mm);
#ifdef NANO_WITH_OPENCL
                fillrow(min, max, table.append("z = A * B") << "OpenCL", measure_mm_ocl);
#endif
                table.print(std::cout);
        }

#ifdef NANO_WITH_OPENCL
        }
        catch (cl::Error& e)
        {
                log_error() << "OpenCL fatal error: <" << e.what() << "> (" << ocl::error_string(e.err()) << ")!";
                return EXIT_FAILURE;
        }
#endif

        return EXIT_SUCCESS;
}

