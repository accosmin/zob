#ifndef NANOCV_CONVOLUTION_H
#define NANOCV_CONVOLUTION_H

#include <functional>
#include <cassert>
#include "dot.hpp"
#include "mad.hpp"

namespace ncv
{
        ///
        /// 2D convolution: odata += idata @ kdata (using Eigen 2D blocks)
        ///
        template
        <
                typename tmatrixi,
                typename tmatrixk = tmatrixi,
                typename tmatrixo = tmatrixi,
                typename tscalar = typename tmatrixi::Scalar
        >
        void conv_eig(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata)
        {
                assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                for (auto r = 0; r < odata.rows(); r ++)
                {
                        for (auto c = 0; c < odata.cols(); c ++)
                        {
                                odata(r, c) += kdata.cwiseProduct(idata.block(r, c, kdata.rows(), kdata.cols())).sum();
                        }
                }
        }

        ///
        /// inverse 2D convolution: odata += idata @ kdata (using Eigen 2D blocks)
        ///
        template
        <
                typename tmatrixo,
                typename tmatrixk = tmatrixo,
                typename tmatrixi = tmatrixo,
                typename tscalar = typename tmatrixi::Scalar
        >
        void iconv_eig(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
        {
                assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                for (auto r = 0; r < odata.rows(); r ++)
                {
                        for (auto c = 0; c < odata.cols(); c ++)
                        {
                                idata.block(r, c, kdata.rows(), kdata.cols()) += kdata * odata(r, c);
                        }
                }
        }

        namespace detail
        {
                template
                <
                        typename tmatrixi,
                        typename tmatrixk = tmatrixi,
                        typename tmatrixo = tmatrixi,
                        typename tdotop,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void conv_dot(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata, const tdotop& dotop)
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();
                        const auto krows = kdata.rows();
                        const auto kcols = kdata.cols();
                        const auto icols = idata.cols();

                        const tscalar* pidata = idata.data();
                        const tscalar* pkdata = kdata.data();
                        tscalar* podata = odata.data();

                        for (auto r = 0; r < orows; r ++)
                        {
                                tscalar* ppodata = podata + r * ocols;

                                for (auto kr = 0; kr < krows; kr ++)
                                {
                                        const tscalar* ppidata = pidata + (r + kr) * icols;
                                        const tscalar* ppkdata = pkdata + kr * kcols;

                                        for (auto c = 0; c < ocols; c ++)
                                        {
                                                ppodata[c] += dotop(ppidata + c, ppkdata);
                                        }
                                }
                        }
                }

                template
                <
                        typename tmatrixi,
                        typename tmatrixk = tmatrixi,
                        typename tmatrixo = tmatrixi,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void conv_dot(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata)
                {
                        const auto kcols = kdata.cols();

                        using std::placeholders::_1;
                        using std::placeholders::_2;

                        // decode at run-time the kernel size
                        if (kcols == 1) { conv_dot(idata, kdata, odata, dot<tscalar, 1>); }
                        else if (kcols == 2) { conv_dot(idata, kdata, odata, dot<tscalar, 2>); }
                        else if (kcols == 3) { conv_dot(idata, kdata, odata, dot<tscalar, 3>); }
                        else if (kcols == 4) { conv_dot(idata, kdata, odata, dot<tscalar, 4>); }
                        else if (kcols == 5) { conv_dot(idata, kdata, odata, dot<tscalar, 5>); }
                        else if (kcols == 6) { conv_dot(idata, kdata, odata, dot<tscalar, 6>); }
                        else if (kcols == 7) { conv_dot(idata, kdata, odata, dot<tscalar, 7>); }
                        else if (kcols == 8) { conv_dot(idata, kdata, odata, dot<tscalar, 8>); }
                        else if (kcols == 9) { conv_dot(idata, kdata, odata, dot<tscalar, 9>); }
                        else if (kcols == 10) { conv_dot(idata, kdata, odata, dot<tscalar, 10>); }
                        else if (kcols == 11) { conv_dot(idata, kdata, odata, dot<tscalar, 11>); }
                        else if (kcols == 12) { conv_dot(idata, kdata, odata, dot<tscalar, 12>); }
                        else if (kcols == 13) { conv_dot(idata, kdata, odata, dot<tscalar, 13>); }
                        else if (kcols == 14) { conv_dot(idata, kdata, odata, dot<tscalar, 14>); }
                        else if (kcols == 15) { conv_dot(idata, kdata, odata, dot<tscalar, 15>); }
                        else
                        {
                                conv_dot(idata, kdata, odata, std::bind(dot<tscalar, decltype(kcols)>, _1, _2, kcols));
                        }
                }

                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tmadop,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void iconv_mad(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata, const tmadop& madop)
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();
                        const auto krows = kdata.rows();
                        const auto kcols = kdata.cols();
                        const auto icols = idata.cols();

                        const tscalar* pkdata = kdata.data();
                        const tscalar* podata = odata.data();
                        tscalar* pidata = idata.data();

                        for (auto r = 0; r < orows; r ++)
                        {
                                const tscalar* ppodata = podata + r * ocols;

                                for (auto kr = 0; kr < krows; kr ++)
                                {
                                        tscalar* ppidata = pidata + (r + kr) * icols;
                                        const tscalar* ppkdata = pkdata + kr * kcols;

                                        for (auto c = 0; c < ocols; c ++)
                                        {
                                                madop(ppkdata, ppodata[c], ppidata + c);
                                        }
                                }
                        }
                }

                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void iconv_mad(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
                {
                        const auto kcols = kdata.cols();

                        using std::placeholders::_1;
                        using std::placeholders::_2;
                        using std::placeholders::_3;

                        // decode at run-time the kernel size
                        if (kcols == 1) { iconv_mad(odata, kdata, idata, mad<tscalar, 1>); }
                        else if (kcols == 2) { iconv_mad(odata, kdata, idata, mad<tscalar, 2>); }
                        else if (kcols == 3) { iconv_mad(odata, kdata, idata, mad<tscalar, 3>); }
                        else if (kcols == 4) { iconv_mad(odata, kdata, idata, mad<tscalar, 4>); }
                        else if (kcols == 5) { iconv_mad(odata, kdata, idata, mad<tscalar, 5>); }
                        else if (kcols == 6) { iconv_mad(odata, kdata, idata, mad<tscalar, 6>); }
                        else if (kcols == 7) { iconv_mad(odata, kdata, idata, mad<tscalar, 7>); }
                        else if (kcols == 8) { iconv_mad(odata, kdata, idata, mad<tscalar, 8>); }
                        else if (kcols == 9) { iconv_mad(odata, kdata, idata, mad<tscalar, 9>); }
                        else if (kcols == 10) { iconv_mad(odata, kdata, idata, mad<tscalar, 10>); }
                        else if (kcols == 11) { iconv_mad(odata, kdata, idata, mad<tscalar, 11>); }
                        else if (kcols == 12) { iconv_mad(odata, kdata, idata, mad<tscalar, 12>); }
                        else if (kcols == 13) { iconv_mad(odata, kdata, idata, mad<tscalar, 13>); }
                        else if (kcols == 14) { iconv_mad(odata, kdata, idata, mad<tscalar, 14>); }
                        else if (kcols == 15) { iconv_mad(odata, kdata, idata, mad<tscalar, 15>); }
                        else
                        {
                                iconv_mad(odata, kdata, idata, std::bind(mad<tscalar, decltype(kcols)>, _1, _2, kcols, _3));
                        }
                }
        }

        ///
        /// 2D convolution: odata += idata @ kdata (using a dot operator)
        ///
        template
        <
                typename tmatrixi,
                typename tmatrixk = tmatrixi,
                typename tmatrixo = tmatrixi,
                typename tscalar = typename tmatrixi::Scalar
        >
        void conv_dot(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata)
        {
                assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                detail::conv_dot(idata, kdata, odata);
        }

        ///
        /// 2D convolution for compile-time kernel size: odata += idata @ kdata (using a dot operator)
        ///
        template
        <
                int tsize,
                typename tmatrixi,
                typename tmatrixk = tmatrixi,
                typename tmatrixo = tmatrixi,
                typename tscalar = typename tmatrixi::Scalar
        >
        void conv_dot(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata)
        {
                assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                assert(idata.cols() + 1 == kdata.cols() + odata.cols());
                assert(tsize == kdata.cols());

                detail::conv_dot(idata, kdata, odata, dot<tscalar, tsize>);
        }
        
        ///
        /// inverse 2D convolution: odata += idata @ kdata (using a mad product)
        ///
        template
        <
                typename tmatrixo,
                typename tmatrixk = tmatrixo,
                typename tmatrixi = tmatrixo,
                typename tscalar = typename tmatrixi::Scalar
        >
        void iconv_mad(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
        {
                assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                detail::iconv_mad(odata, kdata, idata);
        }

        ///
        /// inverse 2D convolution for compile-time kernel size: odata += idata @ kdata (using a mad product)
        ///
        template
        <
                int tsize,
                typename tmatrixo,
                typename tmatrixk = tmatrixo,
                typename tmatrixi = tmatrixo,
                typename tscalar = typename tmatrixi::Scalar
        >
        void iconv_mad(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
        {
                assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                detail::iconv_mad(odata, kdata, idata, mad<tscalar, tsize>);
        }
}

#endif // NANOCV_CONVOLUTION_H

