#ifndef NANOCV_TASK_MNIST_H
#define NANOCV_TASK_MNIST_H

#include "ncv_task.h"

namespace ncv
{
        ////////////////////////////////////////////////////////////////////////////////
        // MNIST task:
        //      - digit classification
        //      - 28x28 grayscale images as inputs
        //      - 10 outputs (10 labels)
        ////////////////////////////////////////////////////////////////////////////////
	
        class mnist_task : public task
        {
        public:

                // create an object clone
                virtual rtask clone(const string_t& /*params*/) const
                {
                        return rtask(new mnist_task(*this));
                }

                // describe the object
                virtual const char* name() const { return "mnist"; }
                virtual const char* desc() const { return "mnist (digit classification)"; }

                // load images from the given directory
                virtual bool load(const string_t& dir);

                // sample training & testing samples
                virtual size_t n_folds() const { return 1; }
                virtual size_t fold_size(index_t f, protocol p) const;
                virtual bool fold_sample(index_t f, protocol p, index_t s, sample& ss) const;

                // access functions
                virtual size_t n_rows() const { return 28; }
                virtual size_t n_cols() const { return 28; }
                virtual size_t n_inputs() const { return n_rows() * n_cols() * 1; }
                virtual size_t n_outputs() const { return 10; }

                virtual size_t n_images() const { return m_images.size(); }
                virtual const annotated_image& image(index_t i) const { return m_images[i]; }

        private:

                // load binary file
                size_t load(const string_t& ifile, const string_t& gfile, protocol p);

        private:

                // attributes
                annotated_images_t      m_images;
        };
}

#endif // NANOCV_TASK_MNIST_H
