#pragma once

#include "tensor_storage.h"

namespace tensor
{
        ///
        /// \brief 3+D tensor mapping an array of 2D planes
        ///
        template
        <
                typename tmap,
                int tdimensions
        >
        class tensor_map_t : public tensor_storage_t<tmap, tdimensions>
        {
        public:

                using tbase = tensor_storage_t<tmap, tdimensions>;
                using tdims = typename tbase::tdims;
                using tsize = typename tbase::tsize;
                using tscalar = typename tbase::tscalar;

                // Eigen compatible
                using Index = typename tbase::Index;
                using Scalar = typename tbase::Scalar;

                ///
                /// \brief constructor
                ///
                template <typename... tsizes>
                tensor_map_t(const tmap& map, const tsizes... dims) :
                        tbase(map, dims...)
                {
                }
        };

        ///
        /// \brief 3+D tensor stored as 2D planes
        ///
        template
        <
                typename tscalar,
                int tdimensions,
                typename tvector = vector_t<tscalar>
        >
        class tensor_t : public tensor_storage_t<tvector, tdimensions>
        {
        public:

                using tbase = tensor_storage_t<tvector, tdimensions>;
                using tdims = typename tbase::tdims;
                using tsize = typename tbase::tsize;

                // Eigen compatible
                using Index = typename tbase::Index;
                using Scalar = typename tbase::Scalar;

                ///
                /// \brief constructor
                ///
                tensor_t() = default;

                ///
                /// \brief constructor
                ///
                template <typename... tsizes>
                explicit tensor_t(const tsizes... dims) :
                        tbase(dims...)
                {
                        this->m_data.resize(this->m_dims.size());
                }

                ///
                /// \brief constructor
                ///
                template
                <
                        typename tmap
                >
                tensor_t(const tensor_map_t<tmap, tdimensions>& other) :
                        tensor_t(other.dims())
                {
                        this->m_data.resize(this->m_dims.size());
                        this->vector() = other.vector().template cast<tscalar>();
                }

                ///
                /// \brief resize to new dimensions
                ///
                template <typename... tsizes>
                tsize resize(const tsizes... dims)
                {
                        this->m_dims = tdims(dims...);
                        this->m_data.resize(this->m_dims.size());
                        return this->size();
                }

                ///
                /// \brief resize to new dimensions
                ///
                tsize resize(const tdims& dims)
                {
                        this->m_dims = dims;
                        this->m_data.resize(this->m_dims.size());
                        return this->size();
                }

        };

        ///
        /// \brief map non-constant data to tensors
        ///
        template
        <
                typename tvalue_,
                typename... tsizes
        >
        auto map_tensor(tvalue_* data, const tsizes... dims)
        {
                using tvalue = typename std::remove_const<tvalue_>::type;
                using tstorage = Eigen::Map<vector_t<tvalue>>;
                return  tensor_map_t<tstorage, sizeof...(dims)>(
                        tensor::map_vector(data, detail::product_variadic<typename tstorage::Index>(dims...)),
                        dims...);
        }

        ///
        /// \brief map constant data to tensors
        ///
        template
        <
                typename tvalue_,
                typename... tsizes
        >
        auto map_tensor(const tvalue_* data, const tsizes... dims)
        {
                using tvalue = typename std::remove_const<tvalue_>::type;
                using tstorage = Eigen::Map<const vector_t<tvalue>>;
                return  tensor_map_t<tstorage, sizeof...(dims)>(
                        tensor::map_vector(data, detail::product_variadic<typename tstorage::Index>(dims...)),
                        dims...);
        }
}