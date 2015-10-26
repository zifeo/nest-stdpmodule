//
//  stdp_triplet_neuron.cpp
//  NEST
//
//

#include "stdp_triplet_neuron.h"

#include "stdpnames.h"

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
	, P_()
	{
	}
	
	STDPTripletNeuron::STDPTripletNeuron( const STDPTripletNeuron& n )
	: Archiving_Node( n )
	, P_( n.P_ )
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
		Archiving_Node::get_status( d );
		P_.get( d );
		//( *d )[ names::recordables ] = recordablesMap_.get_list();
	}
	
	void
	STDPTripletNeuron::set_status( const DictionaryDatum& d )
	{
		P_.set( d );
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
	
	STDPTripletNeuron::Parameters_::Parameters_()
	: weight_( 5.0 )
	, tau_plus_( 16.8 ) // visual cortex data set
	, tau_x_( 101 )
	, tau_minus_( 33.7 ) // visual cortex data set
	, tau_y_( 125 )
	, a2_plus_( 1.0 )
	, a2_minus_( 1.0 )
	, a3_plus_( 1.0 )
	, a3_minus_( 1.0 )
	, r1_( 0.0 )
	, r2_( 0.0 )
	, o1_( 0.0 )
	, o2_( 0.0 )
	{
	}
	
	void
	STDPTripletNeuron::Parameters_::get( DictionaryDatum& d ) const
	{
		def< double_t >( d, names::weight, weight_ );
		def< double_t >( d, stdpnames::tau_plus, tau_plus_ );
		def< double_t >( d, stdpnames::tau_x, tau_x_ );
		def< double_t >( d, stdpnames::tau_minus, tau_minus_ );
		def< double_t >( d, stdpnames::tau_y, tau_y_ );
		def< double_t >( d, stdpnames::a2_plus, a2_plus_ );
		def< double_t >( d, stdpnames::a2_minus, a2_minus_ );
		def< double_t >( d, stdpnames::a3_plus, a3_plus_ );
		def< double_t >( d, stdpnames::a3_minus, a3_minus_ );
		def< double_t >( d, stdpnames::r1, r1_ );
		def< double_t >( d, stdpnames::r2, r2_ );
		def< double_t >( d, stdpnames::o1, o1_ );
		def< double_t >( d, stdpnames::o2, o2_ );
		def< long_t >( d, names::size_of, sizeof( *this ) );
	}
	
	void
	STDPTripletNeuron::Parameters_::set( const DictionaryDatum& d )
	{
		updateValue< double_t >( d, names::weight, weight_ );
		updateValue< double_t >( d, stdpnames::tau_plus, tau_plus_ );
		updateValue< double_t >( d, stdpnames::tau_x, tau_x_ );
		updateValue< double_t >( d, stdpnames::tau_minus, tau_minus_ );
		updateValue< double_t >( d, stdpnames::tau_y, tau_y_ );
		updateValue< double_t >( d, stdpnames::a2_plus, a2_plus_ );
		updateValue< double_t >( d, stdpnames::a2_minus, a2_minus_ );
		updateValue< double_t >( d, stdpnames::a3_plus, a3_plus_ );
		updateValue< double_t >( d, stdpnames::a3_minus, a3_minus_ );
		updateValue< double_t >( d, stdpnames::r1, r1_ );
		updateValue< double_t >( d, stdpnames::r2, r2_ );
		updateValue< double_t >( d, stdpnames::o1, o1_ );
		updateValue< double_t >( d, stdpnames::o2, o2_ );
		
		if ( ! ( tau_x_ > tau_plus_ ) ) {
			throw BadProperty( "Potentiation time-constant for triplet (tau_x) must be bigger than pair-based one (tau_plus)." );
		}
		
		if ( ! ( tau_y_ > tau_minus_ ) ) {
			throw BadProperty( "Depression time-constant for triplet (tau_y) must be bigger than pair-based one (tau_minus)." );
		}
		
		if ( ! ( r1_ >= 0 ) ) {
			throw BadProperty( "Variable r1 must be positive." );
		}
		
		if ( ! ( r2_ >= 0 ) ) {
			throw BadProperty( "Variable r2 must be positive." );
		}
		
		if ( ! ( o1_ >= 0 ) ) {
			throw BadProperty( "TVariable o1 must be positive." );
		}
		
		if ( ! ( o2_ >= 0 ) ) {
			throw BadProperty( "Variable o2 must be positive." );
		}
	}
	
	
}