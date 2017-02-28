#pragma once

#include "task_mem_tensor.h"

namespace nano
{
        ///
        /// IRIS task:
        ///      - irises (the plant) classification
        ///      - 4 attributes
        ///      - 3 classes
        ///
        /// http://archive.ics.uci.edu/ml/datasets/Iris
        ///
        class iris_task_t final : public mem_tensor_task_t
        {
        public:

                explicit iris_task_t(const string_t& configuration = string_t());

                virtual bool populate() override;
        };
}
