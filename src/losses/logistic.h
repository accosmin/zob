#pragma once

#include "classification.h"

namespace nano
{
        ///
        /// \brief multi-class logistic loss: sum(log(1 + exp(-targets_k * scores_k)), k).
        ///
        struct logistic_t
        {
                static auto value(const vector_cmap_t& targets, const vector_cmap_t& scores)
                {
                        return  (1 + (-targets.array() * scores.array()).exp()).log().sum();
                }

                static auto vgrad(const vector_cmap_t& targets, const vector_cmap_t& scores)
                {
                        return  -targets.array() * (-targets.array() * scores.array()).exp() /
                                (1 + (-targets.array() * scores.array()).exp());
                }
        };

        using mlogistic_loss_t = mclassification_t<logistic_t>;
        using slogistic_loss_t = sclassification_t<logistic_t>;
}
