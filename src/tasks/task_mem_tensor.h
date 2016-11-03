#pragma once

#include "task_mem.h"

namespace nano
{
        struct mem_tensor_sample_t
        {
                explicit mem_tensor_sample_t(
                        const size_t index = 0,
                        const vector_t& target = vector_t(),
                        const string_t& label = string_t()) :
                        m_index(index), m_target(target), m_label(label)
                {
                }

                auto index() const { return m_index; }
                auto input(const tensor3d_t& tensor) const { return tensor; }
                auto hash(const size_t seed) const { return seed; }
                auto target() const { return m_target; }
                auto label() const { return m_label; }

                size_t          m_index;
                vector_t        m_target;
                string_t        m_label;
        };

        ///
        /// \brief in-memory generic task consisting of generic 3D input tensors.
        ///
        using mem_tensor_task_t = mem_task_t<tensor3d_t, mem_tensor_sample_t>;
}
