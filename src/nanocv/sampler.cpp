#include "sampler.h"
#include "task.h"
#include "common/usampler.hpp"
#include "common/random.hpp"
#include <algorithm>

namespace ncv
{
        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t::sampler_t(const task_t& task)
                :       m_task(task),
                        m_stype(stype::batch),
                        m_ssize(0)
        {
                reset();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        void sampler_t::reset()
        {
                // collect all available samples (no restriction)
                m_samples = m_task.samples();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t& sampler_t::setup(fold_t fold)
        {
                m_samples.erase(std::remove_if(m_samples.begin(), m_samples.end(),
                                [&] (const sample_t& sample) { return sample.m_fold == fold; }),
                                m_samples.end());

                return order();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t& sampler_t::setup(protocol p)
        {
                m_samples.erase(std::remove_if(m_samples.begin(), m_samples.end(),
                                [&] (const sample_t& sample) { return sample.m_fold.second == p; }),
                                m_samples.end());

                return order();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t& sampler_t::setup(stype s, size_t size)
        {
                m_stype = s;
                m_ssize = size;

                return *this;
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t& sampler_t::setup(atype a)
        {
                const bool annotated = a == atype::annotated;

                m_samples.erase(std::remove_if(m_samples.begin(), m_samples.end(),
                                [&] (const sample_t& sample) { return sample.annotated() == annotated; }),
                                m_samples.end());

                return order();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t& sampler_t::setup(const string_t& label)
        {
                m_samples.erase(std::remove_if(m_samples.begin(), m_samples.end(),
                                [&] (const sample_t& sample) { return sample.m_label == label; }),
                                m_samples.end());

                return order();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t& sampler_t::split(size_t percentage, sampler_t& other)
        {
                assert(&other.m_task == &m_task);

                samples_t tsamples, vsamples;
                ncv::uniform_split(m_samples, percentage, random_t<size_t>(0, m_samples.size()), tsamples, vsamples);

                m_samples = tsamples;
                other.m_samples = vsamples;
                other.order();

                return order();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        samples_t sampler_t::get() const
        {
                samples_t samples;

                switch (m_stype)
                {
                case stype::batch:
                        samples = m_samples;
                        break;

                case stype::random:
                        samples = ncv::uniform_sample(m_samples, m_ssize, random_t<size_t>(0, m_samples.size()));
                        std::sort(samples.begin(), samples.end());
                        break;
                }

                return samples;
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        sampler_t& sampler_t::order()
        {
                std::sort(m_samples.begin(), m_samples.end());

                return *this;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
}
