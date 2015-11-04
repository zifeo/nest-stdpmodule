//
//  stdp_triplet_all_connection.h
//  NEST
//
//

/*	BeginDocumentation
	Name: stdp_triplet_connection - Synapse type for spike-timing dependent plasticity
	with a pair-based/triplet-based learning rule (not depending on weight).
	
	Description:
	stdp_triplet_connection.h is a connector to create synapses with spike time
	dependent plasticity (as defined in the references).
	
	STDP Examples:
	pair-based		a3_plus = a3_minus = 0
	triplet			otherwise
	
	Parameters:
	- tau_plus		double: pair-based potentiation time constant (ms)
	- tau_x			double: triplet potentiation time constant (ms)
	- tau_minus		double: pair-based depression time constant (ms) (normally defined in post-synaptic neuron)
	- tau_y			double:	triplet depression time constant (ms)
	- a2_plus		double: weight change amplitude for pre-post spikes
	- a2_minus		double: weight change amplitude for post-pre spikes
	- a3_plus		double: weight change amplitude for pre-post-pre spikes
	- a3_minus		double: weight change amplitude for post-pre-post spikes
	- r1			double: pre variable r1 (e.g. amount of glutamate bound...)
	- r2			double: pre variable r2 (e.g. number of NMDA receptors...)
	- o1			double: post variable r3 (e.g. influx of calcium concentration...)
	- o2			double: post variable r4 (e.g. number of secondary messengers...)
 
	Reference:
	- Triplets of Spikes in a Model of Spike Timing-Dependent Plasticity, Pfister/Gerstner, 2006.
 */

#ifndef stdp_triplet_connection_h
#define stdp_triplet_connection_h

#include <cassert>
#include <cmath>

#include "connection.h"
#include "stdpnames.h"

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
		
		/**
		 * Default Destructor.
		 */
		~STDPTripletConnection() {}
		
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
}

// Send an event to the receiver of this connection.
template < typename targetidentifierT >
inline void
stdpmodule::STDPTripletConnection< targetidentifierT >::send( Event& e,
															 thread t,
															 double_t t_lastspike,
															 const CommonSynapseProperties& )
{
	
	// timing
	double_t t_spike = e.get_stamp().get_ms();
	
	// get post-synaptic neuron
	Node* target = get_target( t );
	double_t dendritic_delay = get_delay();
	
	// get spike history in relevant range (t1, t2] from post-synaptic neuron (without the added dentritic delay
	std::deque< histentry >::iterator start;
	std::deque< histentry >::iterator finish;
	target->get_history( t_lastspike - dendritic_delay, t_spike - dendritic_delay, &start, &finish );
	
	// go through all post-synaptic spikes since the last pre-synaptic spike from this connection
	double_t t_last_postspike = t_lastspike;
	while ( start != finish )
	{
		// deal with dendritic delay
		double_t t_adjusted = start->t_ + dendritic_delay;
		assert(t_adjusted > t_last_postspike);
		
		// get elapsed time
		double_t delta = t_adjusted - t_last_postspike;
		assert(delta >= 0);
		
		// prepare next iteration
		t_last_postspike = t_adjusted;
		++start;
		
		if ( delta == 0 )
		{
			continue;
		}
		
		// model variables each delta update
		r1_ = r1_ * std::exp( - delta / tau_plus_);  // kplus
		r2_ = r2_ * std::exp( - delta / tau_x_);	 // kx
		o1_ = o1_ * std::exp( - delta / tau_minus_); // kminus
		o2_ = o2_ * std::exp( - delta / tau_y_);	 // ky
		// TODO rename ?
		// TODO max weight ?
		// TODO at the same time ?
		
		// potentiate
		// t = t^post
		weight_ = weight_ + r1_ * ( a2_plus_ + a3_plus_ * o2_ ); // TODO cannot go negative ?
		o1_ = o1_ + 1;
		o2_ = o2_ + 1;
		
	}
	
	// handeling the remaing delta between the last postspike and current spike time
	double_t remaing_delta_ = t_spike - t_last_postspike;
	assert(remaing_delta_ >= 0);
	
	// model variables remaining delta update
	r1_ = r1_ * std::exp( - remaing_delta_ / tau_plus_);
	r2_ = r2_ * std::exp( - remaing_delta_ / tau_x_);
	o1_ = o1_ * std::exp( - remaing_delta_ / tau_minus_);
	o2_ = o2_ * std::exp( - remaing_delta_ / tau_y_);
		
	// depress
	// t = t^pre
	weight_ = weight_ - o1_ * ( a2_minus_ + a3_minus_ * r2_);
	r1_ = r1_ + 1;
	r2_ = r2_ + 1;
	
	// send event
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
	def< double_t >( d, stdpnames::r1, r1_ );
	def< double_t >( d, stdpnames::r2, r2_ );
	def< double_t >( d, stdpnames::o1, o1_ );
	def< double_t >( d, stdpnames::o2, o2_ );
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

#endif /* stdp_triplet_connection_h */