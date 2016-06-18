//  Copyright (c) 2016 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file cuda/target_distribution_policy.hpp

#if !defined(HPX_COMPUTE_CUDA_TARGET_DISTRIBUTION_POLICY)
#define HPX_COMPUTE_CUDA_TARGET_DISTRIBUTION_POLICY

#include <hpx/config.hpp>

#if defined(HPX_HAVE_CUDA) && !defined(__CUDACC__)

#include <hpx/dataflow.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/runtime/components/stubs/stub_base.hpp>
#include <hpx/runtime/serialization/base_object.hpp>
#include <hpx/traits/is_distribution_policy.hpp>
#include <hpx/util/assert.hpp>

#include <hpx/compute/cuda/target.hpp>
#include <hpx/compute/detail/target_distribution_policy.hpp>

#include <algorithm>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

namespace hpx { namespace compute { namespace cuda
{
    /// A target_distribution_policy used for CPU bound localities.
    struct target_distribution_policy
      : compute::detail::target_distribution_policy<cuda::target>
    {
        typedef compute::detail::target_distribution_policy<cuda::target>
            base_type;

        /// Default-construct a new instance of a \a target_distribution_policy.
        /// This policy will represent all devices on the current locality.
        ///
        target_distribution_policy() {}

        /// Create a new \a target_distribution_policy representing the given
        /// set of targets
        ///
        /// \param targets [in] The targets the new instances should represent
        ///
        target_distribution_policy operator()(
            std::vector<target_type> const& targets,
            std::size_t num_partitions = std::size_t(-1)) const
        {
            if (num_partitions == std::size_t(-1))
                num_partitions = targets.size();
            return target_distribution_policy(targets, num_partitions);
        }

        /// Create a new \a target_distribution_policy representing the given
        /// set of targets
        ///
        /// \param targets [in] The targets the new instances should represent
        ///
        target_distribution_policy operator()(
            std::vector<target_type> && targets,
            std::size_t num_partitions = std::size_t(-1)) const
        {
            if (num_partitions == std::size_t(-1))
                num_partitions = targets.size();
            return target_distribution_policy(std::move(targets), num_partitions);
        }

        /// Create a new \a target_distribution_policy representing the given
        /// target
        ///
        /// \param target [in] The target the new instances should represent
        ///
        target_distribution_policy operator()(
            target_type const& target, std::size_t num_partitions = 1) const
        {
            std::vector<target_type> targets;
            targets.push_back(target);
            return target_distribution_policy(std::move(targets), num_partitions);
        }

        /// Create a new \a target_distribution_policy representing the given
        /// target
        ///
        /// \param target [in] The target the new instances should represent
        ///
        target_distribution_policy operator()(
            target_type && target, std::size_t num_partitions = 1) const
        {
            std::vector<target_type> targets;
            targets.push_back(std::move(target));
            return target_distribution_policy(std::move(targets), num_partitions);
        }

        /// Create one object on one of the localities associated by
        /// this policy instance
        ///
        /// \param ts  [in] The arguments which will be forwarded to the
        ///            constructor of the new object.
        ///
        /// \note This function is part of the placement policy implemented by
        ///       this class
        ///
        /// \returns A future holding the global address which represents
        ///          the newly created object
        ///
        template <typename Component, typename ... Ts>
        hpx::future<hpx::id_type> create(Ts &&... ts) const
        {
            target_type t = this->get_next_target();
            hpx::id_type target_locality = t.get_locality();
            return components::stub_base<Component>::create_async(
                target_locality, std::forward<Ts>(ts)..., std::move(t));
        }

        /// \cond NOINTERNAL
        typedef std::pair<hpx::id_type, std::vector<hpx::id_type> >
            bulk_locality_result;
        /// \endcond

        /// Create multiple objects on the localities associated by
        /// this policy instance
        ///
        /// \param count [in] The number of objects to create
        /// \param vs   [in] The arguments which will be forwarded to the
        ///             constructors of the new objects.
        ///
        /// \note This function is part of the placement policy implemented by
        ///       this class
        ///
        /// \returns A future holding the list of global addresses which
        ///          represent the newly created objects
        ///
        template <typename Component, typename ...Ts>
        hpx::future<std::vector<bulk_locality_result> >
        bulk_create(std::size_t count, Ts &&... ts) const
        {
            std::vector<hpx::id_type> localities;
            localities.reserve(this->targets_.size());

            std::vector<hpx::future<hpx::id_type> > objs;
            objs.reserve(this->targets_.size());

            for(target_type& t : this->targets_)
            {
                hpx::id_type target_locality = t.get_locality();

                std::size_t num_partitions = this->get_num_items(count, t);
                objs.push_back(components::stub_base<Component>::create_async(
                    target_locality, num_partitions, ts..., std::move(t)));

                localities.push_back(std::move(target_locality));
            }

            return hpx::dataflow(
                [=](std::vector<hpx::future<hpx::id_type> > && v)
                    mutable -> std::vector<bulk_locality_result>
                {
                    HPX_ASSERT(localities.size() == v.size());

                    std::map<hpx::id_type, std::vector<hpx::id_type> > m;
                    for (std::size_t i = 0; i != v.size(); ++i)
                    {
                        m[localities[i]].push_back(v[i].get());
                    }

                    std::vector<bulk_locality_result> result;
                    result.reserve(m.size());

                    auto end = m.end()
                    for (auto it = m.begin(); it != end; ++it)
                    {
                        result.insert(std::move(*it));
                    }

                    return result;
                },
                std::move(objs));
        }

    protected:
        /// \cond NOINTERNAL
        target_distribution_policy(std::vector<target_type> const& targets,
                std::size_t num_partitions)
          : base_type(targets, num_partitions)
        {}

        target_distribution_policy(std::vector<target_type> && targets,
                std::size_t num_partitions)
          : base_type(std::move(targets), num_partitions)
        {}

        friend class hpx::serialization::access;

        template <typename Archive>
        void serialize(Archive& ar, unsigned int const)
        {
            ar & serialization::base_object<base_type>(*this);
        }
        /// \endcond
    };

    /// A predefined instance of the \a target_distribution_policy for CUDA.
    /// It will represent all local CUDA devices and will place all items to
    /// create here.
    static target_distribution_policy const target_layout;
}}}

/// \cond NOINTERNAL
namespace hpx { namespace traits
{
    template <typename Target>
    struct is_distribution_policy<compute::cuda::target_distribution_policy>
      : std::true_type
    {};

    template <typename Target>
    struct num_container_partitions<compute::cuda::target_distribution_policy>
    {
        static std::size_t
        call(compute::cuda::target_distribution_policy const& policy)
        {
            return policy.get_num_partitions();
        }
    };
}}
/// \endcond

#endif
#endif
