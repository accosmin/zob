#pragma once

#include "learner.h"

namespace nano
{
        ///
        /// \brief Gradient Boosting with stumps as weak learners.
        ///     todo: add citations
        ///
        class gboost_stump_t final : public learner_t
        {
        public:

                enum class stump
                {
                        real,                   ///< stump \in R (no restriction)
                        discrete,               ///< stump \in {-1, +1}
                };

                enum class regularization
                {
                        none,                   ///<
                        adaptive,               ///< shrinkage per round using the validation dataset
                        shrinkage,              ///< global shrinkage (needs tuning)
                        vadaboost,              ///< VadaBoost (needs tuning)
                };

                gboost_stump_t() = default;

                void to_json(json_t&) const override;
                void from_json(const json_t&) override;

                trainer_result_t train(const task_t&, const size_t fold, const loss_t&) const override;

                tensor4d_t output(const tensor4d_t& input) const override;

                bool save(obstream_t&) const override;
                bool load(ibstream_t&) override;

                tensor3d_dim_t idims() const override { return m_idims; }
                tensor3d_dim_t odims() const override { return m_odims; }

                probes_t probes() const override;

        private:

                // attributes
                tensor3d_dim_t  m_idims{0, 0, 0};                       ///< input dimensions
                tensor3d_dim_t  m_odims{0, 0, 0};                       ///< output dimensions
                int             m_rounds{0};                            ///< training: number of boosting rounds
                stump           m_stype{stump::discrete};               ///< training:
                regularization  m_rtype{regularization::adaptive};      ///< training:
        };

        template <>
        inline enum_map_t<gboost_stump_t::stump> enum_string<gboost_stump_t::stump>()
        {
                return
                {
                        { gboost_stump_t::stump::real,                  "real" },
                        { gboost_stump_t::stump::discrete,              "discrete" }
                };
        }

        template <>
        inline enum_map_t<gboost_stump_t::regularization> enum_string<gboost_stump_t::regularization>()
        {
                return
                {
                        { gboost_stump_t::regularization::none,         "none" },
                        { gboost_stump_t::regularization::adaptive,     "adaptive" },
                        { gboost_stump_t::regularization::shrinkage,    "shrinkage" },
                        { gboost_stump_t::regularization::vadaboost,    "vadaboost" }
                };
        }
}
