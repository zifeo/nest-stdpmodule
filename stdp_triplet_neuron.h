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

  void handle(SpikeEvent &);
  void handle(DataLoggingRequest &);

  port handles_test_event(SpikeEvent &, rport);
  port handles_test_event(DataLoggingRequest &, rport);

  void get_status(DictionaryDatum &) const;
  void set_status(const DictionaryDatum &);

private:
  void init_state_(const Node &proto);
  void init_buffers_();
  void calibrate();

  void update(Time const &, const long_t, const long_t);

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap<STDPTripletNeuron>;
  friend class UniversalDataLogger<STDPTripletNeuron>;

  /**
   * Independent parameters of the model.
   */
  struct Parameters_ {

    double_t weight_;
    double_t tau_plus_;
    double_t tau_x_;
    double_t tau_minus_;
    double_t tau_y_;
    double_t Aplus_;
    double_t Aminus_;
    double_t Aplus_triplet_;
    double_t Aminus_triplet_;

    Parameters_(); //!< Sets default parameter values

    void get(DictionaryDatum &) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    void set(const DictionaryDatum &);
  };

  /**
   * State variables of the model.
   */
  struct State_ {

    double_t Kplus_;
    double_t Kplus_triplet_;
    double_t Kminus_;
    double_t Kminus_triplet_;

    State_(); //!< Default initialization

    void get(DictionaryDatum &, const Parameters_ &) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set(const DictionaryDatum &);
  };

  /**
   * Buffers of the model.
   * Buffers and accumulates the number of incoming spikes per time step;
   * RingBuffer stores doubles; for now the numbers are casted.
   */
  struct Buffers_ {
    Buffers_(STDPTripletNeuron &);
    Buffers_(const Buffers_ &, STDPTripletNeuron &);

    RingBuffer n_spikes_;
    RingBuffer n_pre_spikes_;
    RingBuffer n_post_spikes_;

    UniversalDataLogger<STDPTripletNeuron> logger_;
  };

  /**
   * Internal variables of the model.
   */
  struct Variables_ {};

  // Access functions for UniversalDataLogger
  double_t get_Kplus_() const { return S_.Kplus_; }
  double_t get_Kplus_triplet_() const { return S_.Kplus_triplet_; }
  double_t get_Kminus_() const { return S_.Kminus_; }
  double_t get_Kminus_triplet_() const { return S_.Kminus_triplet_; }

  /**
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is crucial: Moving Variables_
   *       to the very end increases simulation time for brunel-2.sli
   *       from 72s to 81s on a Mac, Intel Core 2 Duo 2.2GHz, g++ 4.0.1 -O3
   */

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
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
  // Allow connections to port 0 (spikes to be repeated)
  // and port 1 (spikes to be ignored).
  if (receptor_type == 0 or receptor_type == 1) {
    return receptor_type;
  } else {
    throw UnknownReceptorType(receptor_type, get_name());
  }
}

inline port STDPTripletNeuron::handles_test_event(DataLoggingRequest &dlr,
                                                  rport receptor_type) {
  if (receptor_type != 0)
    throw UnknownReceptorType(receptor_type, get_name());
  return B_.logger_.connect_logging_device(dlr, recordablesMap_);
}

inline void STDPTripletNeuron::get_status(DictionaryDatum &d) const {
  P_.get(d);
  S_.get(d, P_);
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
