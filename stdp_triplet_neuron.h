//
//  stdp_triplet_neuron.h
//  NEST
//
//

/* BeginDocumentation
 Name: stdp_triplet_neuron - Neuron that acts like a stdp_triplet_connection.

 Description:
 stdp_triplet_synapse is a modeled connection with spike time dependent
 plasticity accounting for spike triplet effects (as defined in [1]).

 STDP examples:
 pair-based         Aplus_triplet = Aminus_triplet = 0.0
 triplet            Aplus_triplet = Aminus_triplet = 1.0
 nearest-spike      nearest_spile = True

 Parameters:
 Wmax               double: maximum allowed weight
 neareat_spike		bool: states saturate at 1 only taking into account
 neighboring spikes
 
 tau_plus           double: time constant of short presynaptic trace (tau_plus
 of [1])
 tau_plus_triplet   double: time constant of long presynaptic trace (tau_x of
 [1])
 tau_minus          double: time constant of short postsynaptic trace (tau_minus
 of [1])
 tau_minus_triplet  double: time constant of long postsynaptic trace (tau_y of
 [1])
 
 Aplus              double: weight of pair potentiation rule (A_plus_2 of [1])
 Aplus_triplet      double: weight of triplet potentiation rule (A_plus_3 of
 [1])
 Aminus             double: weight of pair depression rule (A_minus_2 of [1])
 Aminus_triplet     double: weight of triplet depression rule (A_minus_3 of [1])

 Notes about delay:
 This model does not have any delay parameter as both axonal and dendritic
 delays are repectively taken into account by pre-synpatic and post-synaptic
 connections. Any parameter tuning should be done at creation time on pynest.

 States:
 weight				double: synaptic weight
 Kplus              double: pre-synaptic trace (e.g. amount of glutamate
 bound...) (r_1 of [1])
 Kplus_triplet      double: triplet pre-synaptic trace (e.g. number of NMDA
 receptors...) (r_2 of [1])
 Kminus				double: post-synaptic trace (e.g. influx of
 calcium
 concentration...) (o_1 of [1])
 Kminus_triplet		double: triplet post-synaptic trace (e.g. number of
 secondary messengers...) (o_2 of [1])

 Receives: SpikeEvent, DataLoggingRequest

 Sends: SpikeEvent

 References:
 [1] J.-P. Pfister & W. Gerstner (2006) Triplets of Spikes in a Model
 of Spike Timing-Dependent Plasticity.  The Journal of Neuroscience
 26(38):9673-9682; doi:10.1523/JNEUROSCI.1425-06.2006
 [2] stdp_triplet_connection.h

 FirstVersion: Octo 2015
 Author: Alexander Seeholzer, Teo Stocco
 SeeAlso: synapsedict, stdp_synapse, static_synapse
 */

#ifndef STDP_TRIPLET_NEURON_H
#define STDP_TRIPLET_NEURON_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "namedatum.h"
#include "universal_data_logger.h"

namespace stdpmodule {
using namespace nest;

class Network;

class STDPTripletNeuron : public Archiving_Node {

public:
  STDPTripletNeuron();
  STDPTripletNeuron(const STDPTripletNeuron &);

  using Node::handle;
  using Node::handles_test_event;

  port send_test_event(Node &, rport, synindex, bool);
  port handles_test_event(SpikeEvent &, rport);
  port handles_test_event(DataLoggingRequest &, rport);

  void get_status(DictionaryDatum &) const;
  void set_status(const DictionaryDatum &);

  void handle(SpikeEvent &);
  void handle(DataLoggingRequest &);

private:
  void init_state_(const Node &proto) {}
  void init_buffers_();
  void calibrate();

  void update(Time const &, const long_t, const long_t);

  friend class RecordablesMap<STDPTripletNeuron>;
  friend class UniversalDataLogger<STDPTripletNeuron>;

  struct Parameters_ {
	  double_t Wmax_;
	  bool nearest_spike_;
	  
    double_t tau_plus_;
    double_t tau_plus_triplet_;
    double_t tau_minus_;
    double_t tau_minus_triplet_;
	  
    double_t Aplus_;
    double_t Aminus_;
    double_t Aplus_triplet_;
    double_t Aminus_triplet_;

    Parameters_();
    void get(DictionaryDatum &) const;
    void set(const DictionaryDatum &);
  };

  struct State_ {
    double_t weight_;

    double_t Kplus_;
    double_t Kplus_triplet_;
    double_t Kminus_;
    double_t Kminus_triplet_;

    State_();
    void get(DictionaryDatum &) const;
    void set(const DictionaryDatum &);
  };

  struct Buffers_ {
    RingBuffer n_pre_spikes_;
    RingBuffer n_post_spikes_;
    UniversalDataLogger<STDPTripletNeuron> logger_;

    Buffers_(STDPTripletNeuron &);
    Buffers_(const Buffers_ &, STDPTripletNeuron &);
  };

  struct Variables_ {
    double_t Kplus_decay_;
    double_t Kplus_triplet_decay_;
    double_t Kminus_decay_;
    double_t Kminus_triplet_decay_;
  };

  // Access functions for UniversalDataLogger
  double_t get_weight_() const { return S_.weight_; }
  double_t get_Kplus_() const { return S_.Kplus_; }
  double_t get_Kplus_triplet_() const { return S_.Kplus_triplet_; }
  double_t get_Kminus_() const { return S_.Kminus_; }
  double_t get_Kminus_triplet_() const { return S_.Kminus_triplet_; }

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  static RecordablesMap<STDPTripletNeuron> recordablesMap_;
};

inline port STDPTripletNeuron::send_test_event(Node &target,
                                               rport receptor_type, synindex,
                                               bool) {
  SpikeEvent e;
  e.set_sender(*this);
  return target.handles_test_event(e, receptor_type);
}

inline port STDPTripletNeuron::handles_test_event(SpikeEvent &,
                                                  rport receptor_type) {
  // Allow connections to port 0 (pre-synaptic) and port 1 (post-synaptic)
  if (receptor_type != 0 and receptor_type != 1) {
    throw UnknownReceptorType(receptor_type, get_name());
  }
  return receptor_type;
}

inline port STDPTripletNeuron::handles_test_event(DataLoggingRequest &dlr,
                                                  rport receptor_type) {
  if (receptor_type != 0) {
    throw UnknownReceptorType(receptor_type, get_name());
  }
  return B_.logger_.connect_logging_device(dlr, recordablesMap_);
}

inline void STDPTripletNeuron::get_status(DictionaryDatum &d) const {
  P_.get(d);
  S_.get(d);
  Archiving_Node::get_status(d);
  (*d)[names::recordables] = recordablesMap_.get_list();
}

inline void STDPTripletNeuron::set_status(const DictionaryDatum &d) {
  P_.set(d);
  S_.set(d);
  Archiving_Node::set_status(d);
}
}

#endif // STDP_TRIPLET_NEURON_H
