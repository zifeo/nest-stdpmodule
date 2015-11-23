//
//  stdp_triplet_all_connection.h
//  NEST
//
//

/*	BeginDocumentation
 Name: stdp_triplet_all_in_one_synapse - Synapse type with spike-timing
 dependent
 plasticity accounting for spike triplets as described in [1].

 Description:
 stdp_triplet_synapse is a connection with spike time dependent
 plasticity accounting for spike triplet effects (as defined in [1]).

 STDP examples:
 pair-based   Aplus_triplet = Aminus_triplet = 0.0
 triplet      Aplus_triplet = Aminus_triplet = 1.0

 Parameters:
 tau_plus           double: time constant of short presynaptic trace (tau_plus
 of [1])
 tau_plus_triplet   double: time constant of long presynaptic trace (tau_x of
 [1])
 Aplus              double: weight of pair potentiation rule (A_plus_2 of [1])
 Aplus_triplet      double: weight of triplet potentiation rule (A_plus_3 of
 [1])
 Aminus             double: weight of pair depression rule (A_minus_2 of [1])
 Aminus_triplet     double: weight of triplet depression rule (A_minus_3 of [1])

 States:
 Kplus              double: pre-synaptic trace (e.g. amount of glutamate
 bound...) (r_1 of [1])
 Kplus_triplet      double: triplet pre-synaptic trace (e.g. number of NMDA
 receptors...) (r_2 of [1])
 Kminus				double: post-synaptic trace (e.g. influx of calcium
 concentration...) (o_1 of [1])
 Kminus_triplet		double: triplet post-synaptic trace (e.g. number of
 secondary messengers...) (o_2 of [1])

 Transmits: SpikeEvent

 References:
 [1] J.-P. Pfister & W. Gerstner (2006) Triplets of Spikes in a Model
 of Spike Timing-Dependent Plasticity.  The Journal of Neuroscience
 26(38):9673-9682; doi:10.1523/JNEUROSCI.1425-06.2006

 FirstVersion: Octo 2015
 Author: Alexander Seeholzer, Teo Stocco
 SeeAlso: synapsedict, stdp_synapse, static_synapse
 */

#ifndef stdp_triplet_connection_h
#define stdp_triplet_connection_h

#include <cassert>
#include <cmath>

#include "connection.h"
#include "stdpnames.h"

namespace stdpmodule {
using namespace nest;

// connections are templates of target identifier type (used for pointer /
// target index addressing)
// derived from generic connection template
template <typename targetidentifierT>
class STDPTripletConnection : public Connection<targetidentifierT> {

public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection<targetidentifierT> ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPTripletConnection();

  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPTripletConnection(const STDPTripletConnection &);

  /**
   * Default Destructor.
   */
  ~STDPTripletConnection() {}

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase.
  // This avoids explicit name prefixes in all places these functions are used.
  // Since ConnectionBase depends on the template parameter, they are not
  // automatically
  // found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_delay;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status(DictionaryDatum &d) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status(const DictionaryDatum &d, ConnectorModel &cm);

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param t_lastspike Point in time of last spike sent.
   * \param cp common properties of all synapses (empty).
   */
  void send(Event &e, thread t, double_t t_lastspike,
            const CommonSynapseProperties &cp);

  class ConnTestDummyNode : public ConnTestDummyNodeBase {
  public:
    // Ensure proper overriding of overloaded virtual functions.
    // Return values from functions are ignored.
    using ConnTestDummyNodeBase::handles_test_event;
    port handles_test_event(SpikeEvent &, rport) { return invalid_port_; }
  };

  void check_connection(Node &s, Node &t, rport receptor_type,
                        double_t t_lastspike, const CommonPropertiesType &) {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_(dummy_target, s, t, receptor_type);
    t.register_stdp_connection(t_lastspike - get_delay());
  }

  void set_weight(double_t w) { weight_ = w; }

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
template <typename targetidentifierT>
stdpmodule::STDPTripletConnection<targetidentifierT>::STDPTripletConnection()
    : ConnectionBase(), weight_(1.0), tau_plus_(16.8) // visual cortex data set
      ,
      tau_plus_triplet_(101), tau_minus_(33.7) // visual cortex data set
      ,
      tau_minus_triplet_(125), Aplus_(0.1), Aminus_(0.1), Aplus_triplet_(0.1),
      Aminus_triplet_(0.1), Kplus_(0.0), Kplus_triplet_(0.0), Kminus_(0.0),
      Kminus_triplet_(0.0) {}

// Copy constructor.
template <typename targetidentifierT>
stdpmodule::STDPTripletConnection<targetidentifierT>::STDPTripletConnection(
    const STDPTripletConnection<targetidentifierT> &rhs)
    : ConnectionBase(rhs), weight_(rhs.weight_), tau_plus_(rhs.tau_plus_),
      tau_plus_triplet_(rhs.tau_plus_triplet_), tau_minus_(rhs.tau_minus_),
      tau_minus_triplet_(rhs.tau_minus_triplet_), Aplus_(rhs.Aplus_),
      Aminus_(rhs.Aminus_), Aplus_triplet_(rhs.Aplus_triplet_),
      Aminus_triplet_(rhs.Aminus_triplet_), Kplus_(rhs.Kplus_),
      Kplus_triplet_(rhs.Kplus_triplet_), Kminus_(rhs.Kminus_),
      Kminus_triplet_(rhs.Kminus_triplet_) {}

// Send an event to the receiver of this connection.
template <typename targetidentifierT>
inline void stdpmodule::STDPTripletConnection<targetidentifierT>::send(
    Event &e, thread t, double_t t_lastspike, const CommonSynapseProperties &) {

  double_t t_spike = e.get_stamp().get_ms();
  double_t dendritic_delay = get_delay();
  Node *target = get_target(t);

  // get spike history in relevant range (t1, t2] from post-synaptic neuron
  // (without the added dentritic delay
  std::deque<histentry>::iterator start;
  std::deque<histentry>::iterator finish;
  target->get_history(t_lastspike - dendritic_delay, t_spike - dendritic_delay,
                      &start, &finish);

  // go through all post-synaptic spikes since the last pre-synaptic spike from
  // this connection
  double_t t_last_postspike = t_lastspike;
  while (start != finish) {
    // deal with dendritic delay
    double_t t_adjusted = start->t_ + dendritic_delay;
    assert(t_adjusted > t_last_postspike);

    // get elapsed time
    double_t delta = t_adjusted - t_last_postspike;
    assert(delta >= 0);

    // prepare next iteration
    t_last_postspike = t_adjusted;
    ++start;

    if (delta == 0) {
		Kminus_ = Kminus_ + 1;
		Kminus_triplet_ = Kminus_triplet_ + 1;
      continue;
    }

    // model variables each delta update
    Kplus_ = Kplus_ * std::exp(-delta / tau_plus_);
    Kplus_triplet_ = Kplus_triplet_ * std::exp(-delta / tau_plus_triplet_);
    Kminus_ = Kminus_ * std::exp(-delta / tau_minus_);
    Kminus_triplet_ = Kminus_triplet_ * std::exp(-delta / tau_minus_triplet_);

    // potentiate: t = t^post
    weight_ = weight_ + Kplus_ * (Aplus_ + Aplus_triplet_ * Kminus_triplet_);
    Kminus_ = Kminus_ + 1;
    Kminus_triplet_ = Kminus_triplet_ + 1;
  }

  // handeling the remaing delta between the last postspike and current spike
  // time
  double_t remaing_delta_ = t_spike - t_last_postspike;
  assert(remaing_delta_ >= 0);

  // model variables remaining delta update
  Kplus_ = Kplus_ * std::exp(-remaing_delta_ / tau_plus_);
  Kplus_triplet_ =
      Kplus_triplet_ * std::exp(-remaing_delta_ / tau_plus_triplet_);
  Kminus_ = Kminus_ * std::exp(-remaing_delta_ / tau_minus_);
  Kminus_triplet_ =
      Kminus_triplet_ * std::exp(-remaing_delta_ / tau_minus_triplet_);

  // depress: t = t^pre
  weight_ = weight_ - Kminus_ * (Aminus_ + Aminus_triplet_ * Kplus_triplet_);
  Kplus_ = Kplus_ + 1;
  Kplus_triplet_ = Kplus_triplet_ + 1;

  // send event
  e.set_receiver(*target);
  e.set_weight(weight_);
  e.set_delay(get_delay_steps());
  e.set_rport(get_rport());
  e();
}

// Get parameters
template <typename targetidentifierT>
void stdpmodule::STDPTripletConnection<targetidentifierT>::get_status(
    DictionaryDatum &d) const {
  ConnectionBase::get_status(d);
  def<double_t>(d, names::weight, weight_);
  def<double_t>(d, stdpnames::tau_plus, tau_plus_);
  def<double_t>(d, stdpnames::tau_plus_triplet, tau_plus_triplet_);
  def<double_t>(d, stdpnames::tau_minus, tau_minus_);
  def<double_t>(d, stdpnames::tau_minus_triplet, tau_minus_triplet_);
  def<double_t>(d, stdpnames::Aplus, Aplus_);
  def<double_t>(d, stdpnames::Aminus, Aminus_);
  def<double_t>(d, stdpnames::Aplus_triplet, Aplus_triplet_);
  def<double_t>(d, stdpnames::Aminus_triplet, Aminus_triplet_);
  def<double_t>(d, stdpnames::Kplus, Kplus_);
  def<double_t>(d, stdpnames::Kplus_triplet, Kplus_triplet_);
  def<double_t>(d, stdpnames::Kminus, Kminus_);
  def<double_t>(d, stdpnames::Kminus_triplet, Kminus_triplet_);
  def<long_t>(d, names::size_of, sizeof(*this));
}

// Set parameters
template <typename targetidentifierT>
void stdpmodule::STDPTripletConnection<targetidentifierT>::set_status(
    const DictionaryDatum &d, ConnectorModel &cm) {
  ConnectionBase::set_status(d, cm);
  updateValue<double_t>(d, names::weight, weight_);
  updateValue<double_t>(d, stdpnames::tau_plus, tau_plus_);
  updateValue<double_t>(d, stdpnames::tau_plus_triplet, tau_plus_triplet_);
  updateValue<double_t>(d, stdpnames::tau_minus, tau_minus_);
  updateValue<double_t>(d, stdpnames::tau_minus_triplet, tau_minus_triplet_);
  updateValue<double_t>(d, stdpnames::Aplus, Aplus_);
  updateValue<double_t>(d, stdpnames::Aminus, Aminus_);
  updateValue<double_t>(d, stdpnames::Aplus_triplet, Aplus_triplet_);
  updateValue<double_t>(d, stdpnames::Aminus_triplet, Aminus_triplet_);
  updateValue<double_t>(d, stdpnames::Kplus, Kplus_);
  updateValue<double_t>(d, stdpnames::Kplus_triplet, Kplus_triplet_);
  updateValue<double_t>(d, stdpnames::Kminus, Kminus_);
  updateValue<double_t>(d, stdpnames::Kminus_triplet, Kminus_triplet_);

  if (!(tau_plus_triplet_ > tau_plus_)) {
    throw BadProperty("Parameter tau_plus_triplet (time-constant of long "
                      "trace) must be larger than tau_plus "
                      "(time-constant of short trace).");
  }

  if (!(tau_minus_triplet_ > tau_minus_)) {
    throw BadProperty("Parameter tau_minus_triplet (time-constant of long "
                      "trace) must be larger than tau_minus "
                      "(time-constant of short trace).");
  }

  if (!(Kplus_ >= 0)) {
    throw BadProperty("State Kplus must be positive.");
  }

  if (!(Kplus_triplet_ >= 0)) {
    throw BadProperty("State Kplus_triplet must be positive.");
  }

  if (!(Kminus_ >= 0)) {
    throw BadProperty("State Kminus must be positive.");
  }

  if (!(Kminus_triplet_ >= 0)) {
    throw BadProperty("State Kminus_triplet must be positive.");
  }
}

#endif /* stdp_triplet_connection_h */