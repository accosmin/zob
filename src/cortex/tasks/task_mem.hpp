#pragma once

#include "cortex/task.h"
#include "math/random.hpp"

namespace nano
{
        ///
        /// \brief in-memory task consisting of samples organized by fold.
        ///
        template
        <
                typename tsample
        >
        class mem_task_t : public task_t
        {
        public:

                ///
                /// \brief constructor
                ///
                mem_task_t(
                        const string_t& name,
                        const tensor_size_t idims, const tensor_size_t irows, const tensor_size_t icols,
                        const tensor_size_t osize) :
                        m_name(name),
                        m_idims(idims), m_irows(irows), m_icols(icols), m_osize(osize) {}

                ///
                /// \brief destructor
                ///
                virtual ~mem_task_t() {}

                ///
                /// \brief short name of this task
                ///
                virtual string_t name() const override final { return m_name; }

                ///
                /// \brief load the task from the given directory (if possible)
                ///
                virtual bool load(const string_t& dir = string_t()) override final;

                ///
                /// \brief input size
                ///
                virtual tensor_size_t idims() const override final { return m_idims; }
                virtual tensor_size_t irows() const override final { return m_irows; }
                virtual tensor_size_t icols() const override final { return m_icols; }

                ///
                /// \brief output size
                ///
                virtual tensor_size_t osize() const override final { return m_osize; }

                ///
                /// \brief number of folds (not considering the protocol!)
                ///
                virtual size_t n_folds() const override final;

                ///
                /// \brief total number of samples
                ///
                virtual size_t n_samples() const override final;

                ///
                /// \brief number of samples for the given fold
                ///
                virtual size_t n_samples(const fold_t&) const override final;

                ///
                /// \brief randomly shuffle the samples associated for the given fold
                ///
                virtual void shuffle(const fold_t&) const override final;

                ///
                /// \brief retrieve the 3D input tensor for a given sample
                ///
                virtual tensor3d_t input(const fold_t&, const size_t index) const override final;

                ///
                /// \brief retrieve the target for a given sample
                ///
                virtual target_t target(const fold_t&, const size_t index) const override final;

        protected:

                template <typename... tsample_params>
                void push_back(const fold_t& fold, tsample_params&&... sample)
                {
                        auto& data = m_data[fold];
                        data.emplace_back(sample...);
                }

                virtual bool populate(const string_t& dir = string_t()) = 0;

        private:

                using tsamples = std::vector<tsample>;
                using tstorage = std::map<fold_t, tsamples>;

        private:

                // attributes
                string_t                m_name;
                tensor_size_t           m_idims;        ///< input size
                tensor_size_t           m_irows;
                tensor_size_t           m_icols;
                tensor_size_t           m_osize;        ///< output size
                mutable tstorage        m_data;         ///< stored samples (training, validation, test)
        };

        template <typename tsample>
        bool mem_task_t<tsample>::load(const string_t& dir)
        {
                m_data.clear();
                if (!populate(dir))
                {
                        m_data.clear();
                        return false;
                }
                else
                {
                        // tidy-up memory
                        for (auto& data : m_data)
                        {
                                data.second.shrink_to_fit();
                        }
                        return true;
                }
        }

        template <typename tsample>
        size_t mem_task_t<tsample>::n_folds() const
        {
                size_t max_fold = 0;
                for (const auto& data : m_data)
                {
                        max_fold = std::max(max_fold, data.first.m_index);
                }

                return max_fold + 1;
        }

        template <typename tsample>
        size_t mem_task_t<tsample>::n_samples() const
        {
                size_t count = 0;
                for (const auto& data : m_data)
                {
                        count += data.second.size();
                }

                return count;
        }

        template <typename tsample>
        size_t mem_task_t<tsample>::n_samples(const fold_t& fold) const
        {
                const auto it = m_data.find(fold);
                assert(it != m_data.end());
                return it->second.size();
        }

        template <typename tsample>
        void mem_task_t<tsample>::shuffle(const fold_t& fold) const
        {
                const auto it = m_data.find(fold);
                assert(it != m_data.end());

                random_t<size_t> rng(0, it->second.size());
                std::shuffle(it->second.begin(), it->second.end(), rng);
        }

        template <typename tsample>
        tensor3d_t mem_task_t<tsample>::input(const fold_t& fold, const size_t index) const
        {
                const auto it = m_data.find(fold);
                assert(it != m_data.end());
                assert(index < it->second.size());

                const auto ret = it->second[index].input();
                assert(ret.template size<0>() == idims());
                assert(ret.template size<1>() == irows());
                assert(ret.template size<2>() == icols());
                return ret;
        }

        template <typename tsample>
        target_t mem_task_t<tsample>::target(const fold_t& fold, const size_t index) const
        {
                const auto it = m_data.find(fold);
                assert(it != m_data.end());
                assert(index < it->second.size());

                const auto ret = it->second[index].target();
                assert(!ret.annotated() || ret.m_target.size() == osize());
                return ret;
        }
}

