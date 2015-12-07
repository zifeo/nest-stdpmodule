//
//  stdp_long_neuron.h
//  NEST
//
//

#ifndef STDP_LONG_NEURON_H
#define STDP_LONG_NEURON_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace stdpmodule {
using namespace nest;

class Network;

class STDPLongNeuron : public Archiving_Node {

public:
  STDPLongNeuron();
  STDPLongNeuron(const STDPLongNeuron &);

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

  friend class RecordablesMap<STDPLongNeuron>;
  friend class UniversalDataLogger<STDPLongNeuron>;

  struct Parameters_ {
    double_t tau_plus_;
    double_t tau_slow_;
    double_t tau_minus_;
    double_t tau_ht_;
    double_t tau_hom_;
    double_t tau_const_;

    double_t A_;
    double_t P_;
    double_t WP_;
    double_t beta_;
    double_t delta_;

    bool nearest_spike_;

    Parameters_();
    void get(DictionaryDatum &) const;
    void set(const DictionaryDatum &);
  };

  struct State_ {
    double_t weight_;
    double_t weight_ref_;

    double_t B_;
    double_t C_;
    double_t Zplus_;
    double_t Zslow_;
    double_t Zminus_;
    double_t Zht_;

    State_();
    void get(DictionaryDatum &) const;
    void set(const DictionaryDatum &);
  };

  struct Buffers_ {
    RingBuffer n_pre_spikes_;
    RingBuffer n_post_spikes_;
    UniversalDataLogger<STDPLongNeuron> logger_;

    Buffers_(STDPLongNeuron &);
    Buffers_(const Buffers_ &, STDPLongNeuron &);
  };

  struct Variables_ {
    double_t Zplus_decay_;
    double_t Zslow_decay_;
    double_t Zminus_decay_;
    double_t Zht_decay_;
  };

  // Access functions for UniversalDataLogger
  double_t get_weight_() const { return S_.weight_; }

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  static RecordablesMap<STDPLongNeuron> recordablesMap_;
};

inline port STDPLongNeuron::send_test_event(Node &target, rport receptor_type,
                                            synindex, bool) {
  SpikeEvent e;
  e.set_sender(*this);
  return target.handles_test_event(e, receptor_type);
}

inline port STDPLongNeuron::handles_test_event(SpikeEvent &,
                                               rport receptor_type) {
  // Allow connections to port 0 (pre-synaptic) and port 1 (post-synaptic)
  if (receptor_type != 0 and receptor_type != 1) {
    throw UnknownReceptorType(receptor_type, get_name());
  }
  return receptor_type;
}

inline port STDPLongNeuron::handles_test_event(DataLoggingRequest &dlr,
                                               rport receptor_type) {
  if (receptor_type != 0) {
    throw UnknownReceptorType(receptor_type, get_name());
  }
  return B_.logger_.connect_logging_device(dlr, recordablesMap_);
}

inline void STDPLongNeuron::get_status(DictionaryDatum &d) const {
  P_.get(d);
  S_.get(d);
  Archiving_Node::get_status(d);
  (*d)[names::recordables] = recordablesMap_.get_list();
}

inline void STDPLongNeuron::set_status(const DictionaryDatum &d) {
  P_.set(d);
  S_.set(d);
  Archiving_Node::set_status(d);
}
}

#endif /* STDP_LONG_NEURON_H */
