////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2011 Bryce Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#if !defined(HPX_B0516203_C2D9_48E6_9B11_306C8D5CA96C)
#define HPX_B0516203_C2D9_48E6_9B11_306C8D5CA96C

#include <hpx/hpx_fwd.hpp>
#include <hpx/runtime/components/client_base.hpp>

#include <examples/math/distributed/discovery/stubs/discovery.hpp>

namespace hpx { namespace balancing 
{

typedef std::vector<boost::uint32_t> topology_map;

struct discovery : components::client_base<discovery, stubs::discovery>
{
    typedef components::client_base<discovery, stubs::discovery>
        base_type;

  public:
    discovery(naming::id_type const& gid = naming::invalid_id) 
      : base_type(gid) {}

    ///////////////////////////////////////////////////////////////////////////
    lcos::future_value<std::vector<naming::id_type> > build_network_async()
    { return this->base_type::build_network_async(this->gid_); }

    std::vector<naming::id_type> build_network_sync()
    { return this->base_type::build_network_sync(this->gid_); }

    std::vector<naming::id_type> build_network()
    { return this->base_type::build_network(this->gid_); }

    ///////////////////////////////////////////////////////////////////////////
    lcos::future_value<hpx::uintptr_t> topology_lva_async()
    { return this->base_type::topology_lva_async(this->gid_); }

    hpx::uintptr_t topology_lva_sync()
    { return this->base_type::topology_lva_sync(this->gid_); }

    hpx::uintptr_t topology_lva()
    { return this->base_type::topology_lva(this->gid_); }

    ///////////////////////////////////////////////////////////////////////////
    lcos::future_value<bool> empty_async()
    { return this->base_type::empty_async(this->gid_); }

    bool empty_sync()
    { return this->base_type::empty_sync(this->gid_); }

    bool empty()
    { return this->base_type::empty(this->gid_); }
};

}}

#endif // HPX_B0516203_C2D9_48E6_9B11_306C8D5CA96C

