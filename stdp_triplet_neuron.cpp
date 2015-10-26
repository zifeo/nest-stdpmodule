//
//  stdp_triplet_neuron.cpp
//  NEST
//
//

#include "stdp_triplet_neuron.h"

// TODO : check real usage of libs
#include "exceptions.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"

#include <limits>

namespace stdpmodule
{
	using namespace nest;
	
	STDPTripletNeuron::STDPTripletNeuron()
	: Archiving_Node()
	{
	}
	
	void
	STDPTripletNeuron::init_buffers_()
	{
		B_.n_spikes_.clear(); // includes resize
		Archiving_Node::clear_history();
	}
	
	void
	STDPTripletNeuron::update( Time const& origin, const long_t from, const long_t to )
	{
		assert( to >= 0 && ( delay ) from < Scheduler::get_min_delay() );
		assert( from < to );
		
		SpikeEvent se;
		
		for ( long_t lag = from; lag < to; ++lag )
		{
			const ulong_t current_spikes_n = static_cast< ulong_t >( B_.n_spikes_.get_value( lag ) );
	  
			if ( current_spikes_n > 0 )
			{
				for ( ulong_t i_spike = 0; i_spike < current_spikes_n; i_spike++ )
				{
					network()->send( *this, se, lag );
				}
				set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
			}
		}
	}
	
	void
	STDPTripletNeuron::get_status( DictionaryDatum& d ) const
	{
		def< double >( d, names::t_spike, get_spiketime_ms() );
		Archiving_Node::get_status( d );
	}
	
	void
	STDPTripletNeuron::set_status( const DictionaryDatum& d )
	{
		Archiving_Node::set_status( d );
	}
	
	void
	STDPTripletNeuron::handle( SpikeEvent& e )
	{
		// Repeat only spikes incoming on port 0, port 1 will be ignored
		if ( 0 == e.get_rport() )
		{
			B_.n_spikes_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ),
								   static_cast< double_t >( e.get_multiplicity() ) );
		}
	}
	
}