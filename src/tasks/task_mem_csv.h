#pragma once

#include "task_mem_tensor.h"

namespace nano
{
        ///
        /// \brief in-memory task with the data loaded from csv:
        ///     - assumes the target is one dimensional (either a class or a scalar to predict)
        ///     - assumes no missing data
        ///
        class mem_csv_task_t : public mem_tensor_task_t
        {
        public:

                ///
                /// \brief constructor
                ///
                mem_csv_task_t(
                        const tensor3d_dim_t& idims,
                        const tensor3d_dim_t& odims,
                        const size_t fsize) :
                        mem_tensor_task_t(idims, odims, fsize)
                {
                }

                ///
                /// \brief load CSV for classification
                ///
                bool load_classification(const string_t& path, const string_t& task_name,
                        const size_t expected_samples,
                        const strings_t& labels, const size_t label_column);

                ///
                /// \brief load CSV for regression
                ///
                bool load_regression(const string_t& path, const string_t& task_name,
                        const size_t expected_samples,
                        const size_t target_column);
        };
}
