#include "layer_convolution.h"
#include "common/logger.h"
#include "common/math.hpp"
#include "common/random.hpp"
#include "common/conv2d.hpp"
#include "tensor/util.hpp"

namespace ncv
{
        template
        <
                typename tscalar,
                typename tsize
        >
        static void _output(
                const tscalar* idata, tsize idims,
                const tscalar* kdata, tsize krows, tsize kcols,
                tscalar* odata, tsize odims, tsize orows, tsize ocols)
        {
                const tsize irows = orows + krows - 1;
                const tsize icols = ocols + kcols - 1;
                const tsize isize = irows * icols;

                const tsize osize = orows * ocols;                
                const tsize ksize = krows * kcols;

                // output
                for (tsize o = 0; o < odims; o ++)
                {
                        auto omap = tensor::make_matrix(odata + o * osize, orows, ocols);

                        omap.setZero();
                        for (tsize i = 0; i < idims; i ++)
                        {
                                auto imap = tensor::make_matrix(idata + i * isize, irows, icols);
                                auto kmap = tensor::make_matrix(kdata + (o * idims + i) * ksize, krows, kcols);
                                
                                ncv::conv_dot(imap, kmap, omap);
                        }
                }
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        static void _igrad(
                tscalar* gidata, tsize idims,
                const tscalar* kdata, tsize krows, tsize kcols,
                const tscalar* odata, tsize odims, tsize orows, tsize ocols)
        {
                const tsize irows = orows + krows - 1;
                const tsize icols = ocols + kcols - 1;
                const tsize isize = irows * icols;

                const tsize osize = orows * ocols;
                const tsize ksize = krows * kcols;
                
                // input gradient
                tensor::make_vector(gidata, idims * isize).setZero();
                for (tsize o = 0; o < odims; o ++)
                {
                        auto omap = tensor::make_matrix(odata + o * osize, orows, ocols);
                        
                        for (tsize i = 0; i < idims; i ++)
                        {
                                auto gimap = tensor::make_matrix(gidata + i * isize, irows, icols);
                                auto kmap = tensor::make_matrix(kdata + (o * idims + i) * ksize, krows, kcols);                                     

                                ncv::iconv_mad(omap, kmap, gimap);
                        }
                }
        }

        template
        <
                typename tscalar,
                typename tsize
        >
        static void _pgrad(
                const tscalar* idata, tsize idims,
                tscalar* gkdata, tsize krows, tsize kcols,
                const tscalar* odata, tsize odims, tsize orows, tsize ocols)
        {
                const tsize irows = orows + krows - 1;
                const tsize icols = ocols + kcols - 1;
                const tsize isize = irows * icols;

                const tsize osize = orows * ocols;
                const tsize ksize = krows * kcols;

                for (tsize o = 0; o < odims; o ++)
                {
                        auto omap = tensor::make_matrix(odata + o * osize, orows, ocols);

                        for (tsize i = 0; i < idims; i ++)
                        {
                                auto imap = tensor::make_matrix(idata + i * isize, irows, icols);
                                auto gkmap = tensor::make_matrix(gkdata + (o * idims + i) * ksize, krows, kcols);

                                gkmap.setZero();
                                ncv::conv_dot(imap, omap, gkmap);
                        }
                }
        }

#ifdef NANOCV_HAVE_OPENCL
        static const string_t ocl_conv_source = R"xxx(

        #pragma OPENCL EXTENSION cl_amd_fp64 : enable
        #pragma OPENCL EXTENSION cl_khr_fp64 : enable

        __kernel void conv_forward(
                __global const double* idata, int idims,
                __global const double* kdata, int krows, int kcols,
                __global double* odata)
        {
                const int odims = get_global_size(0);
                const int orows = get_global_size(1);
                const int ocols = get_global_size(2);
                const int osize = orows * ocols;

                const int icols = ocols + kcols - 1;
                const int irows = orows + krows - 1;
                const int isize = irows * icols;

                const int ksize = krows * kcols;

                const int o = get_global_id(0);
                const int r = get_global_id(1);
                const int c = get_global_id(2);

                double sum_conv = 0;
                for (int i = 0; i < idims; i ++)
                {
                        double sum = 0;
                        for (int kr = 0; kr < krows; kr ++)
                        {
                                for (int kc = 0; kc < kcols; kc ++)
                                {
                                        const double iv = idata[i * isize + (r + kr) * icols + (c + kc)];
                                        const double kv = kdata[(o * idims + i) * ksize + kr * kcols + kc];

                                        sum += iv * kv;
                                }
                        }

                        sum_conv += sum;
                }

                odata[o * osize + r * ocols + c] = sum_conv;
        }

        __kernel void conv_ibackward(
                __global const double* odata, int odims,
                __global const double* kdata, int krows, int kcols,
                __global double* gidata)
        {
                const int idims = get_global_size(0);
                const int irows = get_global_size(1);
                const int icols = get_global_size(2);
                const int isize = irows * icols;

                const int ocols = icols - kcols + 1;
                const int orows = irows - krows + 1;
                const int osize = orows * ocols;

                const int ksize = krows * kcols;

                const int i = get_global_id(0);
                const int r = get_global_id(1);
                const int c = get_global_id(2);

                const int krmin = max(0,     r - orows + 1);
                const int krmax = min(krows, r + 1);

                const int kcmin = max(0,     c - ocols + 1);
                const int kcmax = min(kcols, c + 1);

                double sum_conv = 0;
                for (int o = 0; o < odims; o ++)
                {
                        __global const double* podata = odata + o * osize;
                        __global const double* pkdata = kdata + (o * idims + i) * ksize;

                        double sum = 0;
                        for (int kr = krmin; kr < krmax; kr ++)
                        {
                                for (int kc = kcmin; kc < kcmax; kc ++)
                                {
                                        const double ov = podata[(r - kr) * ocols + (c - kc)];
                                        const double kv = pkdata[kr * kcols + kc];

                                        sum += ov * kv;
                                }
                        }

                        sum_conv += sum;
                }

                gidata[i * isize + r * icols + c] = sum_conv;
        }

        __kernel void conv_kbackward(
                __global const double* odata, int orows, int ocols,
                __global const double* idata, int idims,
                __global double* gkdata)
        {
                const int odims = get_global_size(0);
                const int krows = get_global_size(1);
                const int kcols = get_global_size(2);
                const int ksize = krows * kcols;

                const int irows = orows + krows - 1;
                const int icols = ocols + kcols - 1;
                const int isize = irows * icols;

                const int osize = orows * ocols;

                const int o = get_global_id(0);
                const int kr = get_global_id(1);
                const int kc = get_global_id(2);

                for (int i = 0; i < idims; i ++)
                {
                        __global const double* podata = odata + o * osize;
                        __global const double* pidata = idata + i * isize;

                        double sum = 0;
                        for (int r = 0; r < orows; r ++)
                        {
                                for (int c = 0; c < ocols; c ++)
                                {
                                        const double ov = podata[r * ocols + c];
                                        const double iv = pidata[(r + kr) * icols + (c + kc)];

                                        sum += ov * iv;
                                }
                        }

                        gkdata[(o * idims + i) * ksize + kr * kcols + kc] = sum;
                }
        }

        )xxx";
#endif

        conv_layer_t::conv_layer_t(const string_t& parameters)
                :       layer_t(parameters, "convolution layer, parameters: dims=16[1,256],rows=8[1,32],cols=8[1,32]")
        {
        }

#ifdef NANOCV_HAVE_OPENCL
        conv_layer_t::conv_layer_t(const conv_layer_t& other)
                :       conv_layer_t(other.configuration())
        {
                if (other.krows() > 0 && other.kcols() > 0)
                {
                        resize(other.m_idata);
                        m_kdata = other.m_kdata;
                        params_changed();
                }
        }

        conv_layer_t& conv_layer_t::operator=(const conv_layer_t& other)
        {
                if (this != &other && other.krows() > 0 && other.kcols() > 0)
                {
                        resize(other.m_idata);
                        m_kdata = other.m_kdata;
                        params_changed();
                }

                return *this;
        }

#endif

        size_t conv_layer_t::resize(const tensor_t& tensor)
        {
                const size_t idims = tensor.dims();
                const size_t irows = tensor.rows();
                const size_t icols = tensor.cols();

                const size_t odims = math::clamp(text::from_params<size_t>(configuration(), "dims", 16), 1, 256);
                const size_t krows = math::clamp(text::from_params<size_t>(configuration(), "rows", 8), 1, 32);
                const size_t kcols = math::clamp(text::from_params<size_t>(configuration(), "cols", 8), 1, 32);

                if (irows < krows || icols < kcols)
                {
                        const string_t message =
                                "invalid size (" + text::to_string(idims) + "x" + text::to_string(irows) +
                                 "x" + text::to_string(icols) + ") -> (" + text::to_string(odims) + "x" +
                                 text::to_string(krows) + "x" + text::to_string(kcols) + ")";

                        log_error() << "convolution layer: " << message;
                        throw std::runtime_error("convolution layer: " + message);
                }

                const size_t orows = irows - krows + 1;
                const size_t ocols = icols - kcols + 1;

                // resize buffers
                m_idata.resize(idims, irows, icols);
                m_odata.resize(odims, orows, ocols);
                m_kdata.resize(odims * idims, krows, kcols);

#ifdef NANOCV_HAVE_OPENCL
                // create opencl objects (if available)
                ocl::manager_t& theocl = ocl::manager_t::instance();
                if (theocl.valid())
                {
                        // kernels
                        m_ocl_context = theocl.make_context();
                        m_ocl_queue = theocl.make_command_queue(m_ocl_context);
                        m_ocl_program = theocl.make_program_from_text(m_ocl_context, ocl_conv_source);
                        m_ocl_fkernel = theocl.make_kernel(m_ocl_program, "conv_forward");
                        m_ocl_bikernel = theocl.make_kernel(m_ocl_program, "conv_ibackward");
                        m_ocl_bkkernel = theocl.make_kernel(m_ocl_program, "conv_kbackward");

                        // forward buffers
                        m_ocl_idata = theocl.make_buffer(m_ocl_context, ocl::bytesize(m_idata), CL_MEM_READ_ONLY);
                        m_ocl_kdata = theocl.make_buffer(m_ocl_context, ocl::bytesize(m_kdata), CL_MEM_READ_ONLY);
                        m_ocl_odata = theocl.make_buffer(m_ocl_context, ocl::bytesize(m_odata), CL_MEM_READ_WRITE);

                        // backward buffers
                        m_gkdata.resize(odims * idims, krows, kcols);
                        m_ocl_gidata = theocl.make_buffer(m_ocl_context, ocl::bytesize(m_idata), CL_MEM_WRITE_ONLY);
                        m_ocl_gkdata = theocl.make_buffer(m_ocl_context, ocl::bytesize(m_gkdata), CL_MEM_WRITE_ONLY);

                        // setup forward kernel
                        m_ocl_fkernel.setArg(0, m_ocl_idata);
                        m_ocl_fkernel.setArg(1, int(idims));
                        m_ocl_fkernel.setArg(2, m_ocl_kdata);
                        m_ocl_fkernel.setArg(3, int(krows));
                        m_ocl_fkernel.setArg(4, int(kcols));
                        m_ocl_fkernel.setArg(5, m_ocl_odata);

                        // setup backward kernels
                        m_ocl_bikernel.setArg(0, m_ocl_odata);
                        m_ocl_bikernel.setArg(1, int(odims));
                        m_ocl_bikernel.setArg(2, m_ocl_kdata);
                        m_ocl_bikernel.setArg(3, int(krows));
                        m_ocl_bikernel.setArg(4, int(kcols));
                        m_ocl_bikernel.setArg(5, m_ocl_gidata);

                        m_ocl_bkkernel.setArg(0, m_ocl_odata);
                        m_ocl_bkkernel.setArg(1, int(orows));
                        m_ocl_bkkernel.setArg(2, int(ocols));
                        m_ocl_bkkernel.setArg(3, m_ocl_idata);
                        m_ocl_bkkernel.setArg(4, int(idims));
                        m_ocl_bkkernel.setArg(5, m_ocl_gkdata);
                }
#endif

                return psize();
        }

        void conv_layer_t::zero_params()
        {
                m_kdata.zero();

                params_changed();
        }

        void conv_layer_t::random_params(scalar_t min, scalar_t max)
        {
                m_kdata.random(random_t<scalar_t>(min, max));

                params_changed();
        }

        scalar_t* conv_layer_t::save_params(scalar_t* params) const
        {
                params = tensor::save(m_kdata, params);
                return params;
        }

        const scalar_t* conv_layer_t::load_params(const scalar_t* params)
        {
                params = tensor::load(m_kdata, params);

                params_changed();

                return params;
        }

        void conv_layer_t::params_changed() const
        {
#ifdef NANOCV_HAVE_OPENCL
                // send parameters to OpenCL device (if available)
                ocl::manager_t& theocl = ocl::manager_t::instance();
                if (theocl.valid())
                {
                        m_ocl_queue.enqueueWriteBuffer(m_ocl_kdata, CL_TRUE, 0, ocl::bytesize(m_kdata), m_kdata.data());
                }
#endif
        }

        const tensor_t& conv_layer_t::output(const tensor_t& input)
        {
                assert(idims() == input.dims());
                assert(irows() == input.rows());
                assert(icols() == input.cols());

                m_idata.copy_from(input);

#ifdef NANOCV_HAVE_OPENCL
                // OpenCL version
                ocl::manager_t& theocl = ocl::manager_t::instance();
                if (theocl.valid())
                {
                        m_ocl_queue.enqueueWriteBuffer(m_ocl_idata, CL_TRUE, 0, ocl::bytesize(m_idata), m_idata.data());

                        m_ocl_queue.enqueueNDRangeKernel(m_ocl_fkernel, cl::NullRange,
                                cl::NDRange(odims(), orows(), ocols()),
                                cl::NDRange(1, orows(), ocols()));
                        m_ocl_queue.finish();

                        m_ocl_queue.enqueueReadBuffer(m_ocl_odata, CL_TRUE, 0, ocl::bytesize(m_odata), m_odata.data());
                }
                else
#endif
                // CPU version
                {
                        _output(m_idata.data(), idims(),
                                m_kdata.data(), krows(), kcols(),
                                m_odata.data(), odims(), orows(), ocols());
                }

                return m_odata;
        }        

        const tensor_t& conv_layer_t::igrad(const tensor_t& output)
        {
                assert(odims() == output.dims());
                assert(orows() == output.rows());
                assert(ocols() == output.cols());

                m_odata.copy_from(output);

#ifdef NANOCV_HAVE_OPENCL
                // OpenCL version
                ocl::manager_t& theocl = ocl::manager_t::instance();
                if (theocl.valid())
                {
                        m_ocl_queue.enqueueWriteBuffer(m_ocl_odata, CL_TRUE, 0, ocl::bytesize(m_odata), m_odata.data());

                        m_ocl_queue.enqueueNDRangeKernel(m_ocl_bikernel, cl::NullRange,
                                                         cl::NDRange(idims(), irows(), icols()),
                                                         cl::NDRange(1, irows(), icols()));

                        m_ocl_queue.enqueueReadBuffer(m_ocl_gidata, CL_TRUE, 0, ocl::bytesize(m_idata), m_idata.data());
                }
                else
#endif
                // CPU version
                {
                        _igrad(m_idata.data(), idims(),
                               m_kdata.data(), krows(), kcols(),
                               m_odata.data(), odims(), orows(), ocols());
                }

                return m_idata;
        }

        void conv_layer_t::pgrad(const tensor_t& output, scalar_t* gradient)
        {
                assert(odims() == output.dims());
                assert(orows() == output.rows());
                assert(ocols() == output.cols());

                m_odata.copy_from(output);

#ifdef NANOCV_HAVE_OPENCL
                // OpenCL version
                ocl::manager_t& theocl = ocl::manager_t::instance();
                if (theocl.valid())
                {
                        m_ocl_queue.enqueueWriteBuffer(m_ocl_odata, CL_TRUE, 0, ocl::bytesize(m_odata), m_odata.data());

                        m_ocl_queue.enqueueNDRangeKernel(m_ocl_bkkernel, cl::NullRange,
                                                         cl::NDRange(odims(), krows(), kcols()),
                                                         cl::NDRange(1, krows(), kcols()));

                        m_ocl_queue.enqueueReadBuffer(m_ocl_gkdata, CL_TRUE, 0, ocl::bytesize(m_gkdata), m_gkdata.data());
                        m_gkdata.copy_to(gradient);
                }
                else
#endif
                // CPU version
                {
                        _pgrad(m_idata.data(), idims(),
                               gradient, krows(), kcols(),
                               m_odata.data(), odims(), orows(), ocols());
                }
        }
}


