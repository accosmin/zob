#pragma once

#include "conv2d_dot.hpp"
#include "conv2d_mad.hpp"

namespace tensor
{
        ///
        /// \brief 2D convolution: odata += idata @ kdata (decode at runtime the best operator to use)
        ///
        struct conv2d_dyn_t
        {
                template
                <
                        typename tmatrixi,
                        typename tmatrixk,
                        typename tmatrixo
                >
                void operator()(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo&& odata) const
                {
                        conv2d_assert(idata, kdata, odata);

                        if (kdata.rows() == 1 && kdata.cols() == 1)
                        {
                                odata.noalias() += idata * kdata(0, 0);
                        }
                        else if (odata.rows() == 1 && odata.cols() == 1)
                        {
                                odata(0, 0) += (idata.array() * kdata.array()).sum();
                        }
                        else if (kdata.rows() == 3 && kdata.cols() == 3)
                        {
                                conv3x3(idata, kdata, odata);
                        }
                        else if (kdata.rows() == 5 && kdata.cols() == 5)
                        {
                                conv5x5(idata, kdata, odata);
                        }
                        else if (kdata.rows() == 7 && kdata.cols() == 7)
                        {
                                conv7x7(idata, kdata, odata);
                        }
                        else
                        {
                                conv2d_dot_dyn_t()(idata, kdata, odata);
                        }
                }

        private:

                template <typename tmatrixi, typename tmatrixk, typename tmatrixo>
                void conv3x3(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo&& odata) const
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();

                        const auto krow0 = kdata.row(0);
                        const auto krow1 = kdata.row(1);
                        const auto krow2 = kdata.row(2);

                        for (auto r = 0; r < orows; r ++)
                        {
                                const auto irow0 = idata.row(r + 0);
                                const auto irow1 = idata.row(r + 1);
                                const auto irow2 = idata.row(r + 2);

                                for (auto c = 0; c < ocols; c ++)
                                {
                                        odata(r, c) +=
                                        irow0.template segment<3>(c).dot(krow0) +
                                        irow1.template segment<3>(c).dot(krow1) +
                                        irow2.template segment<3>(c).dot(krow2);
                                }
                        }
                }

                template <typename tmatrixi, typename tmatrixk, typename tmatrixo>
                void conv5x5(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo&& odata) const
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();

                        const auto krow0 = kdata.row(0);
                        const auto krow1 = kdata.row(1);
                        const auto krow2 = kdata.row(2);
                        const auto krow3 = kdata.row(3);
                        const auto krow4 = kdata.row(4);

                        for (auto r = 0; r < orows; r ++)
                        {
                                const auto irow0 = idata.row(r + 0);
                                const auto irow1 = idata.row(r + 1);
                                const auto irow2 = idata.row(r + 2);
                                const auto irow3 = idata.row(r + 3);
                                const auto irow4 = idata.row(r + 4);

                                for (auto c = 0; c < ocols; c ++)
                                {
                                        odata(r, c) +=
                                        irow0.template segment<5>(c).dot(krow0) +
                                        irow1.template segment<5>(c).dot(krow1) +
                                        irow2.template segment<5>(c).dot(krow2) +
                                        irow3.template segment<5>(c).dot(krow3) +
                                        irow4.template segment<5>(c).dot(krow4);
                                }
                        }
                }

                template <typename tmatrixi, typename tmatrixk, typename tmatrixo>
                void conv7x7(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo&& odata) const
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();

                        const auto krow0 = kdata.row(0);
                        const auto krow1 = kdata.row(1);
                        const auto krow2 = kdata.row(2);
                        const auto krow3 = kdata.row(3);
                        const auto krow4 = kdata.row(4);
                        const auto krow5 = kdata.row(5);
                        const auto krow6 = kdata.row(6);

                        for (auto r = 0; r < orows; r ++)
                        {
                                const auto irow0 = idata.row(r + 0);
                                const auto irow1 = idata.row(r + 1);
                                const auto irow2 = idata.row(r + 2);
                                const auto irow3 = idata.row(r + 3);
                                const auto irow4 = idata.row(r + 4);
                                const auto irow5 = idata.row(r + 5);
                                const auto irow6 = idata.row(r + 6);

                                for (auto c = 0; c < ocols; c ++)
                                {
                                        odata(r, c) +=
                                        irow0.template segment<7>(c).dot(krow0) +
                                        irow1.template segment<7>(c).dot(krow1) +
                                        irow2.template segment<7>(c).dot(krow2) +
                                        irow3.template segment<7>(c).dot(krow3) +
                                        irow4.template segment<7>(c).dot(krow4) +
                                        irow5.template segment<7>(c).dot(krow5) +
                                        irow6.template segment<7>(c).dot(krow6);
                                }
                        }
                }

        };
}

