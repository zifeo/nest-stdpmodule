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
	pair-based		Aplus_triplet = Aminus_triplet = 0
	triplet			otherwise
	
	Parameters:
	- tau_plus		double: pair-based potentiation time constant (ms)
	- tau_plus_triplet			double: triplet potentiation time constant (ms)
	- tau_minus		double: pair-based depression time constant (ms) (normally defined in post-synaptic neuron)
	- tau_minus_triplet			double:	triplet depression time constant (ms)
	- Aplus		double: weight change amplitude for pre-post spikes
	- Aminus		double: weight change amplitude for post-pre spikes
	- Aplus_triplet		double: weight change amplitude for pre-post-pre spikes
	- Aminus_triplet		double: weight change amplitude for post-pre-post spikes
	- Kplus			double: pre variable Kplus (e.g. amount of glutamate bound...)
	- Kplus_triplet			double: pre variable Kplus_triplet (e.g. number of NMDA receptors...)
	- Kminus			double: post variable r3 (e.g. influx of calcium concentration...)
	- Kminus_triplet			double: post variable r4 (e.g. number of secondary messengers...)
 
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
		double_t tau_plus_triplet_;
		double_t tau_minus_;
		double_t tau_minus_triplet_;
		double_t Aplus_;
		double_t Aminus_;
		double_t Aplus_triplet_;
		double_t Aminus_triplet_;
		
		double_t Kplus_;
		double_t Kplus_triplet_;
		double_t Kminus_;
		double_t Kminus_triplet_;
	};
	
}

// Default constructor
template < typename targetidentifierT >
stdpmodule::STDPTripletConnection< targetidentifierT >::STDPTripletConnection()
: ConnectionBase()
, weight_( 1.0 )
, tau_plus_( 16.8 ) // visual cortex data set
, tau_plus_triplet_( 101 )
, tau_minus_( 33.7 ) // visual cortex data set
, tau_minus_triplet_( 125 )
, Aplus_( 0.1 )
, Aminus_( 0.1 )
, Aplus_triplet_( 0.1 )
, Aminus_triplet_( 0.1 )
, Kplus_( 0.0 )
, Kplus_triplet_( 0.0 )
, Kminus_( 0.0 )
, Kminus_triplet_( 0.0 )
{
}

// Copy constructor.
template < typename targetidentifierT >
stdpmodule::STDPTripletConnection< targetidentifierT >::STDPTripletConnection( const STDPTripletConnection< targetidentifierT >& rhs )
: ConnectionBase( rhs )
, weight_( rhs.weight_ )
, tau_plus_( rhs.tau_plus_ )
, tau_plus_triplet_( rhs.tau_plus_triplet_ )
, tau_minus_( rhs.tau_minus_ )
, tau_minus_triplet_( rhs.tau_minus_triplet_ )
, Aplus_( rhs.Aplus_ )
, Aminus_( rhs.Aminus_ )
, Aplus_triplet_( rhs.Aplus_triplet_ )
, Aminus_triplet_( rhs.Aminus_triplet_ )
, Kplus_( rhs.Kplus_ )
, Kplus_triplet_( rhs.Kplus_triplet_ )
, Kminus_( rhs.Kminus_ )
, Kminus_triplet_( rhs.Kminus_triplet_ )
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
		Kplus_ = Kplus_ * std::exp( - delta / tau_plus_);  // kplus
		Kplus_triplet_ = Kplus_triplet_ * std::exp( - delta / tau_plus_triplet_);	 // kx
		Kminus_ = Kminus_ * std::exp( - delta / tau_minus_); // kminus
		Kminus_triplet_ = Kminus_triplet_ * std::exp( - delta / tau_minus_triplet_);	 // ky
		// TODO rename ?
		// TODO max weight ?
		// TODO at the same time ?
		
		// potentiate
		// t = t^post
		weight_ = weight_ + Kplus_ * ( Aplus_ + Aplus_triplet_ * Kminus_triplet_ ); // TODO cannot go negative ?
		Kminus_ = Kminus_ + 1;
		Kminus_triplet_ = Kminus_triplet_ + 1;
		
	}
	
	// handeling the remaing delta between the last postspike and current spike time
	double_t remaing_delta_ = t_spike - t_last_postspike;
	assert(remaing_delta_ >= 0);
	
	// model variables remaining delta update
	Kplus_ = Kplus_ * std::exp( - remaing_delta_ / tau_plus_);
	Kplus_triplet_ = Kplus_triplet_ * std::exp( - remaing_delta_ / tau_plus_triplet_);
	Kminus_ = Kminus_ * std::exp( - remaing_delta_ / tau_minus_);
	Kminus_triplet_ = Kminus_triplet_ * std::exp( - remaing_delta_ / tau_minus_triplet_);
	
	// depress
	// t = t^pre
	weight_ = weight_ - Kminus_ * ( Aminus_ + Aminus_triplet_ * Kplus_triplet_);
	Kplus_ = Kplus_ + 1;
	Kplus_triplet_ = Kplus_triplet_ + 1;
	
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
	def< double_t >( d, stdpnames::tau_plus_triplet, tau_plus_triplet_ );
	def< double_t >( d, stdpnames::tau_minus, tau_minus_ );
	def< double_t >( d, stdpnames::tau_minus_triplet, tau_minus_triplet_ );
	def< double_t >( d, stdpnames::Aplus, Aplus_ );
	def< double_t >( d, stdpnames::Aminus, Aminus_ );
	def< double_t >( d, stdpnames::Aplus_triplet, Aplus_triplet_ );
	def< double_t >( d, stdpnames::Aminus_triplet, Aminus_triplet_ );
	def< double_t >( d, stdpnames::Kplus, Kplus_ );
	def< double_t >( d, stdpnames::Kplus_triplet, Kplus_triplet_ );
	def< double_t >( d, stdpnames::Kminus, Kminus_ );
	def< double_t >( d, stdpnames::Kminus_triplet, Kminus_triplet_ );
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
	updateValue< double_t >( d, stdpnames::tau_plus_triplet, tau_plus_triplet_ );
	updateValue< double_t >( d, stdpnames::tau_minus, tau_minus_ );
	updateValue< double_t >( d, stdpnames::tau_minus_triplet, tau_minus_triplet_ );
	updateValue< double_t >( d, stdpnames::Aplus, Aplus_ );
	updateValue< double_t >( d, stdpnames::Aminus, Aminus_ );
	updateValue< double_t >( d, stdpnames::Aplus_triplet, Aplus_triplet_ );
	updateValue< double_t >( d, stdpnames::Aminus_triplet, Aminus_triplet_ );
	updateValue< double_t >( d, stdpnames::Kplus, Kplus_ );
	updateValue< double_t >( d, stdpnames::Kplus_triplet, Kplus_triplet_ );
	updateValue< double_t >( d, stdpnames::Kminus, Kminus_ );
	updateValue< double_t >( d, stdpnames::Kminus_triplet, Kminus_triplet_ );
	
	if ( ! ( tau_plus_triplet_ > tau_plus_ ) ) {
		throw BadProperty( "Potentiation time-constant for triplet (tau_plus_triplet) must be bigger than pair-based one (tau_plus)." );
	}
	
	if ( ! ( tau_minus_triplet_ > tau_minus_ ) ) {
		throw BadProperty( "Depression time-constant for triplet (tau_minus_triplet) must be bigger than pair-based one (tau_minus)." );
	}
	
	if ( ! ( Kplus_ >= 0 ) ) {
		throw BadProperty( "Variable Kplus must be positive." );
	}
	
	if ( ! ( Kplus_triplet_ >= 0 ) ) {
		throw BadProperty( "Variable Kplus_triplet must be positive." );
	}
	
	if ( ! ( Kminus_ >= 0 ) ) {
		throw BadProperty( "TVariable Kminus must be positive." );
	}
	
	if ( ! ( Kminus_triplet_ >= 0 ) ) {
		throw BadProperty( "Variable Kminus_triplet must be positive." );
	}
	
}

#endif /* stdp_triplet_connection_h */