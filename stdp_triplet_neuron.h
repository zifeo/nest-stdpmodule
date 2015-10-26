//
//  stdp_triplet_neuron.h
//  NEST
//
//

/* BeginDocumentation
 Name: stdp_triplet_neuron - Neuron that acts like a stdp_triplet_connection.
 
 Description:
 See stdp_triplet_connection.h
 
 Receives: SpikeEvent
 
 Sends: SpikeEvent
 
 Parameters:
 See stdp_triplet_connection.h
 
 References:
 See stdp_triplet_connection.h
 */


#ifndef STDP_TRIPLET_NEURON_H
#define STDP_TRIPLET_NEURON_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"

namespace stdpmodule
{
	using namespace nest;
	
	class Network;
	
	class STDPTripletNeuron : public Archiving_Node
	{
		
	public:
		STDPTripletNeuron();
		
		/**
		 * Import sets of overloaded virtual functions.
		 * @see Technical Issues / Virtual Functions: Overriding,
		 * Overloading, and Hiding
		 */
		using Node::handle;
		using Node::handles_test_event;
		
		port send_test_event( Node&, rport, synindex, bool );
		
		void handle( SpikeEvent& );
		port handles_test_event( SpikeEvent&, rport );
		
		void get_status( DictionaryDatum& ) const;
		void set_status( const DictionaryDatum& );
		
	private:
		
		void
		init_state_( const Node& )
		{
		} // no state
		
		void init_buffers_();
		
		// TODO : calibrate ?
		void
		calibrate()
		{
		} // no variables
		
		void update( Time const&, const long_t, const long_t );
		
		/**
		 Buffers and accumulates the number of incoming spikes per time step;
		 RingBuffer stores doubles; for now the numbers are casted.
		 */
		struct Buffers_
		{
			RingBuffer n_spikes_;
		};
		
		Buffers_ B_;
	};
	
	inline port
	STDPTripletNeuron::send_test_event( Node& target, rport receptor_type, synindex, bool )
	{
		SpikeEvent e;
		e.set_sender( *this );
		
		return target.handles_test_event( e, receptor_type );
	}
	
	inline port
	STDPTripletNeuron::handles_test_event( SpikeEvent&, rport receptor_type )
	{
		// Allow connections to port 0 (spikes to be repeated)
		// and port 1 (spikes to be ignored).
		if ( receptor_type == 0 or receptor_type == 1 )
		{
			return receptor_type;
		}
		else
		{
			throw UnknownReceptorType( receptor_type, get_name() );
		}
	}
	
}

#endif // STDP_TRIPLET_NEURON_H
