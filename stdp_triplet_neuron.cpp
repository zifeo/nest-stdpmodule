////
////  stdp_triplet_neuron.cpp
////  NEST
////
////
//
//#include "stdp_triplet_neuron.h"
//
//#include "stdpnames.h"
//
//// TODO : check real usage of libs
//#include "exceptions.h"
//#include "network.h"
//#include "dict.h"
//#include "integerdatum.h"
//#include "doubledatum.h"
//#include "dictutils.h"
//#include "numerics.h"
//
//#include <limits>
//
//using namespace nest;
//
//stdpmodule::STDPTripletNeuron::STDPTripletNeuron()
//: Archiving_Node()
//, P_()
//{
//}
//
//stdpmodule::STDPTripletNeuron::STDPTripletNeuron( const STDPTripletNeuron& n )
//: Archiving_Node( n )
//, P_( n.P_ )
//{
//}
//
//void
//stdpmodule::STDPTripletNeuron::init_buffers_()
//{
//	B_.n_spikes_.clear(); // includes resize
//	B_.n_pre_spikes_.clear();
//	B_.n_post_spikes_.clear();
//	Archiving_Node::clear_history();
//}
//
//void
//stdpmodule::STDPTripletNeuron::update( Time const& origin, const long_t from, const long_t to )
//{
//	assert( to >= 0 && ( delay ) from < Scheduler::get_min_delay() );
//	assert( from < to );
//	
//	// TODO : r2_before last prespike
//	double_t r2_before_last_prespike = P_.r2_;
//	
//	// TODO : not efficient at all... ?
//	for ( long_t lag = from; lag < to; ++lag )
//	{
//		//const ulong_t current_spikes_n = static_cast< ulong_t >( B_.n_spikes_.get_value( lag ) );
//		
//		const double_t current_pre_spikes_n = B_.n_pre_spikes_.get_value( lag );
//		const double_t current_post_spikes_n = B_.n_pre_spikes_.get_value( lag );
//		
//		std::cout << "Current lag: " << lag << std::endl;
//		std::cout << "Current pre spike: " << current_pre_spikes_n << std::endl;
//		std::cout << "Current post spike: " << current_post_spikes_n << std::endl;
//		
//		// model variables remaining delta update
//		P_.r1_ = P_.r1_ * std::exp( - 1.0 / P_.tau_plus_);
//		P_.r2_ = P_.r2_ * std::exp( - 1.0 / P_.tau_x_);
//		P_.o1_ = P_.o1_ * std::exp( - 1.0 / P_.tau_minus_);
//		P_.o2_ = P_.o2_ * std::exp( - 1.0 / P_.tau_y_);
//		
//		if ( current_pre_spikes_n > 0 )
//		{
//			r2_before_last_prespike = P_.r2_;
//			
//			// depress
//			// t = t^pre
//			P_.weight_ = P_.weight_ - P_.o1_ * ( P_.a2_minus_ + P_.a3_minus_ * r2_before_last_prespike);
//			P_.r1_ = P_.r1_ + 1;
//			P_.r2_ = P_.r2_ + 1;
//			
//			SpikeEvent se;
//			se.set_multiplicity( current_pre_spikes_n );
//			se.set_weight( P_.weight_ );
//			network()->send( *this, se, lag );
//			// TODO : others things to set for spike event ?
//			set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
//
//		}
//		
//		if ( current_post_spikes_n > 0 )
//		{
//			
//			// potentiate
//			// t = t^post
//			P_.weight_ = P_.weight_ + P_.r1_ * ( P_.a2_plus_ + P_.a3_plus_ * P_.o2_ );
//			P_.o1_ = P_.o1_ + 1;
//			P_.o2_ = P_.o2_ + 1;
//			
//		}
//		
//	}
//}
//
//void
//stdpmodule::STDPTripletNeuron::get_status( DictionaryDatum& d ) const
//{
//	Archiving_Node::get_status( d );
//	P_.get( d );
//	//( *d )[ names::recordables ] = recordablesMap_.get_list();
//}
//
//void
//stdpmodule::STDPTripletNeuron::set_status( const DictionaryDatum& d )
//{
//	P_.set( d );
//	Archiving_Node::set_status( d );
//}
//
//void
//stdpmodule::STDPTripletNeuron::handle( SpikeEvent& e )
//{
//
//	assert( e.get_delay() > 0 );
//	
//	// TODO : get rid of the cast ?
//	B_.n_spikes_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ),
//						   static_cast< double_t >( e.get_multiplicity() ) );
//	
//	switch (e.get_rport()) {
//		case 0: // PRE
//			B_.n_pre_spikes_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ),
//									   e.get_weight() * e.get_multiplicity() );
//			break;
//			
//		case 1: // POST
//			B_.n_post_spikes_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ),
//							   e.get_weight() * e.get_multiplicity() );
//			// TODO : actually don't care about the weight here ?
//			break;
//			
//		default:
//			break;
//	}
//	
//}
//
//stdpmodule::STDPTripletNeuron::Parameters_::Parameters_()
//: weight_( 5.0 )
//, tau_plus_( 16.8 ) // visual cortex data set
//, tau_x_( 101 )
//, tau_minus_( 33.7 ) // visual cortex data set
//, tau_y_( 125 )
//, a2_plus_( 1.0 )
//, a2_minus_( 1.0 )
//, a3_plus_( 1.0 )
//, a3_minus_( 1.0 )
//, r1_( 0.0 )
//, r2_( 0.0 )
//, o1_( 0.0 )
//, o2_( 0.0 )
//{
//}
//
//void
//stdpmodule::STDPTripletNeuron::Parameters_::get( DictionaryDatum& d ) const
//{
//	def< double_t >( d, names::weight, weight_ );
//	def< double_t >( d, stdpnames::tau_plus, tau_plus_ );
//	def< double_t >( d, stdpnames::tau_x, tau_x_ );
//	def< double_t >( d, stdpnames::tau_minus, tau_minus_ );
//	def< double_t >( d, stdpnames::tau_y, tau_y_ );
//	def< double_t >( d, stdpnames::a2_plus, a2_plus_ );
//	def< double_t >( d, stdpnames::a2_minus, a2_minus_ );
//	def< double_t >( d, stdpnames::a3_plus, a3_plus_ );
//	def< double_t >( d, stdpnames::a3_minus, a3_minus_ );
//	def< double_t >( d, stdpnames::r1, r1_ );
//	def< double_t >( d, stdpnames::r2, r2_ );
//	def< double_t >( d, stdpnames::o1, o1_ );
//	def< double_t >( d, stdpnames::o2, o2_ );
//	def< long_t >( d, names::size_of, sizeof( *this ) );
//}
//
//void
//stdpmodule::STDPTripletNeuron::Parameters_::set( const DictionaryDatum& d )
//{
//	updateValue< double_t >( d, names::weight, weight_ );
//	updateValue< double_t >( d, stdpnames::tau_plus, tau_plus_ );
//	updateValue< double_t >( d, stdpnames::tau_x, tau_x_ );
//	updateValue< double_t >( d, stdpnames::tau_minus, tau_minus_ );
//	updateValue< double_t >( d, stdpnames::tau_y, tau_y_ );
//	updateValue< double_t >( d, stdpnames::a2_plus, a2_plus_ );
//	updateValue< double_t >( d, stdpnames::a2_minus, a2_minus_ );
//	updateValue< double_t >( d, stdpnames::a3_plus, a3_plus_ );
//	updateValue< double_t >( d, stdpnames::a3_minus, a3_minus_ );
//	updateValue< double_t >( d, stdpnames::r1, r1_ );
//	updateValue< double_t >( d, stdpnames::r2, r2_ );
//	updateValue< double_t >( d, stdpnames::o1, o1_ );
//	updateValue< double_t >( d, stdpnames::o2, o2_ );
//	
//	if ( ! ( tau_x_ > tau_plus_ ) ) {
//		throw BadProperty( "Potentiation time-constant for triplet (tau_x) must be bigger than pair-based one (tau_plus)." );
//	}
//	
//	if ( ! ( tau_y_ > tau_minus_ ) ) {
//		throw BadProperty( "Depression time-constant for triplet (tau_y) must be bigger than pair-based one (tau_minus)." );
//	}
//	
//	if ( ! ( r1_ >= 0 ) ) {
//		throw BadProperty( "Variable r1 must be positive." );
//	}
//	
//	if ( ! ( r2_ >= 0 ) ) {
//		throw BadProperty( "Variable r2 must be positive." );
//	}
//	
//	if ( ! ( o1_ >= 0 ) ) {
//		throw BadProperty( "TVariable o1 must be positive." );
//	}
//	
//	if ( ! ( o2_ >= 0 ) ) {
//		throw BadProperty( "Variable o2 must be positive." );
//	}
//}
