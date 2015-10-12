//
//  stdp_triplet_connection.h
//  NEST
//
//

/*	BeginDocumentation
	Name: stdp_triplet_connection - Synapse type for spike-timing dependent plasticity
	with a pair-based/triplet-based learning rule (not depending on weight).
	
	Description:
	stdp_triplet_connection.h is a connector to create synapses with spike time
	dependent plasticity (as defined in the references).
	
	Examples:
	pair-based STDP	a3_plus = a3_minus = 0
	triplet STDP       otherwise
	
	Parameters:
	- tau_plus		double: time constant of STDP window, potentiation in ms
	- tau_x			double:
	- tau_minus		double: time constant of STDP window, depression in ms
 (normally defined in post-synaptic neuron)
	- tau_y			double:
	- a2_plus		double:
	- a2_minus		double:
	- a3_plus		double:
	- a3_minus		double:
	
	Reference:
	- Triplets of Spikes in a Model of Spike Timing-Dependent Plasticity, Pfister/Gerstner, 2006.
 */

#ifndef stdp_triplet_connection_h
#define stdp_triplet_connection_h

#include <cassert>
#include <cmath>

#include "connection.h"
#include "name.h"

namespace stdpnames
{
	const Name tau_plus( "tau_plus" );
	const Name tau_x( "tau_x" );
	const Name tau_minus( "tau_minus" );
	const Name tau_y( "tau_y" );
	const Name a2_plus( "a2_plus" );
	const Name a2_minus( "a2_minus" );
	const Name a3_plus( "a3_plus" );
	const Name a3_minus( "a3_minus" );
}

namespace stdpmodule
{
	using namespace nest;
	
	// connections are templates of target identifier type (used for pointer / target index addressing)
	// derived from generic connection template
	template < typename targetidentifierT >
	class STDPTripletConnection : public Connection< targetidentifierT >
	{
		
	public:
		typedef CommonSynapseProperties CommonPropertiesType;
		typedef Connection< targetidentifierT > ConnectionBase;
		
		/**
		 * Default Constructor.
		 * Sets default values for all parameters. Needed by GenericConnectorModel.
		 */
		STDPTripletConnection();
		
		/**
		 * Copy constructor.
		 * Needs to be defined properly in order for GenericConnector to work.
		 */
		STDPTripletConnection( const STDPTripletConnection& );
		
		// Explicitly declare all methods inherited from the dependent base ConnectionBase.
		// This avoids explicit name prefixes in all places these functions are used.
		// Since ConnectionBase depends on the template parameter, they are not automatically
		// found in the base class.
		using ConnectionBase::get_delay_steps;
		using ConnectionBase::get_delay;
		using ConnectionBase::get_rport;
		using ConnectionBase::get_target;
		
		/**
		 * Get all properties of this connection and put them into a dictionary.
		 */
		void get_status( DictionaryDatum& d ) const;
		
		/**
		 * Set properties of this connection from the values given in dictionary.
		 */
		void set_status( const DictionaryDatum& d, ConnectorModel& cm );
		
		/**
		 * Send an event to the receiver of this connection.
		 * \param e The event to send
		 * \param t_lastspike Point in time of last spike sent.
		 * \param cp common properties of all synapses (empty).
		 */
		void send( Event& e, thread t, double_t t_lastspike, const CommonSynapseProperties& cp );
		
		
		class ConnTestDummyNode : public ConnTestDummyNodeBase
		{
		public:
			// Ensure proper overriding of overloaded virtual functions.
			// Return values from functions are ignored.
			using ConnTestDummyNodeBase::handles_test_event;
			port
			handles_test_event( SpikeEvent&, rport )
			{
				return invalid_port_;
			}
		};
		
		void
		check_connection( Node& s,
						 Node& t,
				   rport receptor_type,
				   double_t t_lastspike,
				   const CommonPropertiesType& )
		{
			ConnTestDummyNode dummy_target;
			ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
			t.register_stdp_connection( t_lastspike - get_delay() );
		}
		
		void
		set_weight( double_t w )
		{
			weight_ = w;
		}
		
	private:
		double_t weight_;
		double_t tau_plus_;
		double_t tau_x_;
		double_t tau_minus_;
		double_t tau_y_;
		double_t a2_plus_;
		double_t a2_minus_;
		double_t a3_plus_;
		double_t a3_minus_;
		
		double_t r1_;
		double_t r2_;
		double_t o1_;
		double_t o2_;
	};
	
}




// Default constructor
template < typename targetidentifierT >
stdpmodule::STDPTripletConnection< targetidentifierT >::STDPTripletConnection()
: ConnectionBase()
, weight_( 5.0 )
, tau_plus_( 16.8 )
, tau_x_( 20.0 )
, tau_minus_( 33.7 )
, tau_y_( 40.0 )
, a2_plus_( 1.0 )
, a2_minus_( 1.0 )
, a3_plus_( 1.0 )
, a3_minus_( 1.0 )
, r1_( 1.0 )
, r2_( 1.0 )
, o1_( 1.0 )
, o2_( 1.0 )
{
	// TODO : better error handling (no assert)
	assert(tau_x_ > tau_plus_); // 9674 - J. Neurosci, September 20, 2006
	assert(tau_y_ > tau_minus_); // 9674 - J. Neurosci, September 20, 2006
	assert(r1_ > 0);
	assert(r2_ > 0);
	assert(o1_ > 0);
	assert(o2_ > 0);
}

// Copy constructor.
template < typename targetidentifierT >
stdpmodule::STDPTripletConnection< targetidentifierT >::STDPTripletConnection( const STDPTripletConnection< targetidentifierT >& rhs )
: ConnectionBase( rhs )
, weight_( rhs.weight_ )
, tau_plus_( rhs.tau_plus_ )
, tau_x_( rhs.tau_x_ )
, tau_minus_( rhs.tau_minus_ )
, tau_y_( rhs.tau_y_ )
, a2_plus_( rhs.a2_plus_ )
, a2_minus_( rhs.a2_minus_ )
, a3_plus_( rhs.a3_plus_ )
, a3_minus_( rhs.a3_minus_ )
, r1_( rhs.r1_ )
, r2_( rhs.r2_ )
, o1_( rhs.o1_ )
, o2_( rhs.o2_ )
{
	// TODO : better error handling (no assert)
	assert(tau_x_ > tau_plus_); // 9674 - J. Neurosci, September 20, 2006
	assert(tau_y_ > tau_minus_); // 9674 - J. Neurosci, September 20, 2006
	assert(r1_ > 0);
	assert(r2_ > 0);
	assert(o1_ > 0);
	assert(o2_ > 0);
}

// Send an event to the receiver of this connection.
template < typename targetidentifierT >
inline void
stdpmodule::STDPTripletConnection< targetidentifierT >::send( Event& e,
															 thread t,
															 double_t t_lastspike, // last spike emitted
															 const CommonSynapseProperties& ) // params
{
	
	// timing
	double_t t_spike = e.get_stamp().get_ms();
	double_t delta_with_lastspike = t_spike - t_lastspike;
	assert(delta_with_lastspike >= 0);
	
	// save before spike value
	double_t r1_before_ = r1_;
	double_t r2_before_ = r2_;
	// TODO check delta step == gobal step
	
	Node* target = get_target( t );
	double_t dendritic_delay = get_delay();
	
	// get spike history in relevant range (t1, t2] from post-synaptic neuron
	std::deque< histentry >::iterator start;
	std::deque< histentry >::iterator finish;
	
	// For a new synapse, t_lastspike contains the point in time of the last spike.
	// So we initially read the history(t_last_spike - dendritic_delay, ...,  T_spike-dendritic_delay]
	// which increases the access counter for these entries.
	// At registration, all entries' access counters of history[0, ..., t_last_spike -
	// dendritic_delay] have been
	// incremented by Archiving_Node::register_stdp_connection(). See bug #218 for details.
	target->get_history( t_lastspike - dendritic_delay, t_spike - dendritic_delay, &start, &finish );
	
	std::cout << "# Spike event at " << t_spike << std::endl;
	std::cout << "- dendritic delay " << dendritic_delay << std::endl;
	std::cout << "- r1 " << r1_ << std::endl;
	std::cout << "- r2 " << r2_ << std::endl;
	std::cout << "- o1 " << o1_ << std::endl;
	std::cout << "- o2 " << o2_ << std::endl;
	
	// facilitation due to post-synaptic spikes since last pre-synaptic spike
	// go through also post-synaptic-received spikes since the last one from this connection
	double_t t_last_postspike = t_lastspike;
	while ( start != finish )
	{

		double_t delta = ( start->t_ + dendritic_delay ) - t_last_postspike;
		t_last_postspike = start->t_ + dendritic_delay;
		assert(delta >= 0);
		
		std::cout << " " << start->t_ << std::endl;
		++start;
		if ( delta == 0 )
		{
			continue;
		}
		
		// model variables
		r1_ = r1_ * std::exp( - delta / tau_plus_);
		r2_ = r2_ * std::exp( - delta / tau_x_);
		o1_ = o1_ * std::exp( - delta / tau_minus_);
		o2_ = o2_ * std::exp( - delta / tau_y_);

		// t = t^post
		weight_ = weight_ + r1_ * ( a2_plus_ + a3_plus_ * o2_);
		o1_ = o1_ + 1;
		o2_ = o2_ + 1;
	}
	
	// handeling the remaing delta between the last postspike and current spike time
	double_t remaing_delta_ = t_spike - t_last_postspike;
	assert(remaing_delta_ >= 0);
	r1_ = r1_ * std::exp( - remaing_delta_ / tau_plus_);
	r2_ = r2_ * std::exp( - remaing_delta_ / tau_x_);
	o1_ = o1_ * std::exp( - remaing_delta_ / tau_minus_);
	o2_ = o2_ * std::exp( - remaing_delta_ / tau_y_);
	
	// t = t^pre
	weight_ = weight_ - o1_ * ( a2_minus_ + a3_minus_ * r2_before_);
	r1_ = r1_ + 1;
	r2_ = r2_ + 1;
	
	e.set_receiver( *target );
	e.set_weight( weight_ );
	e.set_delay( get_delay_steps() );
	e.set_rport( get_rport() );
	e();

}

// Get parameters
template < typename targetidentifierT >
void
stdpmodule::STDPTripletConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
	ConnectionBase::get_status( d );
	def< double_t >( d, names::weight, weight_ );
	def< double_t >( d, stdpnames::tau_plus, tau_plus_ );
	def< double_t >( d, stdpnames::tau_x, tau_x_ );
	def< double_t >( d, stdpnames::tau_minus, tau_minus_ );
	def< double_t >( d, stdpnames::tau_y, tau_y_ );
	def< double_t >( d, stdpnames::a2_plus, a2_plus_ );
	def< double_t >( d, stdpnames::a2_minus, a2_minus_ );
	def< double_t >( d, stdpnames::a3_plus, a3_plus_ );
	def< double_t >( d, stdpnames::a3_minus, a3_minus_ );
	def< long_t >( d, names::size_of, sizeof( *this ) );
}

// Set parameters
template < typename targetidentifierT >
void
stdpmodule::STDPTripletConnection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
	ConnectionBase::set_status( d, cm );
	updateValue< double_t >( d, names::weight, weight_ );
	updateValue< double_t >( d, stdpnames::tau_plus, tau_plus_ );
	updateValue< double_t >( d, stdpnames::tau_x, tau_x_ );
	updateValue< double_t >( d, stdpnames::tau_minus, tau_minus_ );
	updateValue< double_t >( d, stdpnames::tau_y, tau_y_ );
	updateValue< double_t >( d, stdpnames::a2_plus, a2_plus_ );
	updateValue< double_t >( d, stdpnames::a2_minus, a2_minus_ );
	updateValue< double_t >( d, stdpnames::a3_plus, a3_plus_ );
	updateValue< double_t >( d, stdpnames::a3_minus, a3_minus_ );
}

#endif /* stdp_triplet_connection_h */