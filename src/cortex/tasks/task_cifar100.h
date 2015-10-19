#pragma once

#include "cortex/task.h"

namespace cortex
{
        ///
        /// CIFAR100 task:
        ///      - object classification
        ///      - 32x32 color images as inputs
        ///      - 100 outputs (100 labels)
        ///
        /// http://www.cs.toronto.edu/~kriz/cifar.html
        ///
        class cifar100_task_t : public task_t
        {
        public:

                NANOCV_MAKE_CLONABLE(cifar100_task_t, "CIFAR-100 (object classification)")

                // constructor
                explicit cifar100_task_t(const string_t& configuration = string_t());

                // load images from the given directory
                virtual bool load(const string_t& dir) override;

                // access functions
                virtual tensor_size_t irows() const override { return 32; }
                virtual tensor_size_t icols() const override { return 32; }
                virtual tensor_size_t osize() const override { return 100; }
                virtual size_t fsize() const override { return 1; }
                virtual color_mode color() const override { return color_mode::rgba; }

        private:

                // load binary file
                bool load(const string_t& filename, const char* bdata, size_t bdata_size, protocol p, size_t count);
        };
}
