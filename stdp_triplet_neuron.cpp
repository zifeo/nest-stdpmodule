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

using namespace nest;

/* ---------------------------------------------------------------- devices */

nest::RecordablesMap<stdpmodule::STDPTripletNeuron>
    stdpmodule::STDPTripletNeuron::recordablesMap_;

namespace nest {
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
nest::RecordablesMap<stdpmodule::STDPTripletNeuron>::create() {
  // use standard names whereever you can for consistency!
  insert_(stdpnames::Kplus, &stdpmodule::STDPTripletNeuron::get_Kplus_);
  insert_(stdpnames::Kplus_triplet, &stdpmodule::STDPTripletNeuron::get_Kplus_triplet_);
  insert_(stdpnames::Kminus, &stdpmodule::STDPTripletNeuron::get_Kminus_);
  insert_(stdpnames::Kminus_triplet, &stdpmodule::STDPTripletNeuron::get_Kminus_triplet_);
}
}

/* ---------------------------------------------------------------- parameters */

stdpmodule::STDPTripletNeuron::Parameters_::Parameters_()
: weight_(5.0), tau_plus_(16.8) // visual cortex data set
,
tau_x_(101), tau_minus_(33.7) // visual cortex data set
,
tau_y_(125), Aplus_(1.0), Aminus_(1.0), Aplus_triplet_(1.0),
Aminus_triplet_(1.0) {}


void stdpmodule::STDPTripletNeuron::Parameters_::get(DictionaryDatum &d) const {
  def<double_t>(d, names::weight, weight_);
  def<double_t>(d, stdpnames::tau_plus, tau_plus_);
  def<double_t>(d, stdpnames::tau_plus_triplet, tau_x_);
  def<double_t>(d, stdpnames::tau_minus, tau_minus_);
  def<double_t>(d, stdpnames::tau_minus_triplet, tau_y_);
  def<double_t>(d, stdpnames::Aplus, Aplus_);
  def<double_t>(d, stdpnames::Aminus, Aminus_);
  def<double_t>(d, stdpnames::Aplus_triplet, Aplus_triplet_);
  def<double_t>(d, stdpnames::Aminus_triplet, Aminus_triplet_);
}

void stdpmodule::STDPTripletNeuron::Parameters_::set(const DictionaryDatum &d) {

  updateValue<double_t>(d, names::weight, weight_);
  updateValue<double_t>(d, stdpnames::tau_plus, tau_plus_);
  updateValue<double_t>(d, stdpnames::tau_plus_triplet, tau_x_);
  updateValue<double_t>(d, stdpnames::tau_minus, tau_minus_);
  updateValue<double_t>(d, stdpnames::tau_minus_triplet, tau_y_);
  updateValue<double_t>(d, stdpnames::Aplus, Aplus_);
  updateValue<double_t>(d, stdpnames::Aminus, Aminus_);
  updateValue<double_t>(d, stdpnames::Aplus_triplet, Aplus_triplet_);
  updateValue<double_t>(d, stdpnames::Aminus_triplet, Aminus_triplet_);

  if (!(tau_x_ > tau_plus_)) {
    throw BadProperty("Potentiation time-constant for triplet (tau_x) must be "
                      "bigger than pair-based one (tau_plus).");
  }

  if (!(tau_y_ > tau_minus_)) {
    throw BadProperty("Depression time-constant for triplet (tau_y) must be "
                      "bigger than pair-based one (tau_minus).");
  }
}

/* ---------------------------------------------------------------- states */

stdpmodule::STDPTripletNeuron::State_::State_(): Kplus_(0.0), Kplus_triplet_(0.0), Kminus_(0.0),
Kminus_triplet_(0.0) {}

void stdpmodule::STDPTripletNeuron::State_::get(DictionaryDatum &d,
                                                const Parameters_ &p) const {
  def<double_t>(d, stdpnames::Kplus, Kplus_);
  def<double_t>(d, stdpnames::Kplus_triplet, Kplus_triplet_);
  def<double_t>(d, stdpnames::Kminus, Kminus_);
  def<double_t>(d, stdpnames::Kminus_triplet, Kminus_triplet_);
}

void stdpmodule::STDPTripletNeuron::State_::set(const DictionaryDatum &d) {
  updateValue<double_t>(d, stdpnames::Kplus, Kplus_);
  updateValue<double_t>(d, stdpnames::Kplus_triplet, Kplus_triplet_);
  updateValue<double_t>(d, stdpnames::Kminus, Kminus_);
  updateValue<double_t>(d, stdpnames::Kminus_triplet, Kminus_triplet_);

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

/* ---------------------------------------------------------------- variables */


/* ---------------------------------------------------------------- buffers */

stdpmodule::STDPTripletNeuron::Buffers_::Buffers_(STDPTripletNeuron &n)
    : logger_(n) {}

stdpmodule::STDPTripletNeuron::Buffers_::Buffers_(const Buffers_ &,
                                                  STDPTripletNeuron &n)
    : logger_(n) {}

/* ---------------------------------------------------------------- constructors */

stdpmodule::STDPTripletNeuron::STDPTripletNeuron()
    : Archiving_Node(), P_(), S_(), B_(*this) {}

stdpmodule::STDPTripletNeuron::STDPTripletNeuron(const STDPTripletNeuron &n)
    : Archiving_Node(n), P_(n.P_), S_(n.S_), B_(n.B_, *this) {}

/* ---------------------------------------------------------------- initialization */


void stdpmodule::STDPTripletNeuron::init_state_(const Node &proto) {
  // TODO for what ?
}

void stdpmodule::STDPTripletNeuron::init_buffers_() {
  B_.n_spikes_.clear(); // includes resize
  B_.n_pre_spikes_.clear();
  B_.n_post_spikes_.clear();
  B_.logger_.reset(); // includes resize
  Archiving_Node::clear_history();
}

void stdpmodule::STDPTripletNeuron::calibrate() { B_.logger_.init(); }

/* ---------------------------------------------------------------- updates */

void stdpmodule::STDPTripletNeuron::update(Time const &origin,
                                           const long_t from, const long_t to) {
  assert(to >= 0 && (delay)from < Scheduler::get_min_delay());
  assert(from < to);

  // TODO : not efficient at all... ?
  for (long_t lag = from; lag < to; ++lag) {
    // const ulong_t current_spikes_n = static_cast< ulong_t >(
    // B_.n_spikes_.get_value( lag ) );

    const double_t current_pre_spikes_n = B_.n_pre_spikes_.get_value(lag);
    const double_t current_post_spikes_n = B_.n_pre_spikes_.get_value(lag);

    std::cout << "Current lag: " << lag << std::endl;
    std::cout << "Current pre spike: " << current_pre_spikes_n << std::endl;
    std::cout << "Current post spike: " << current_post_spikes_n << std::endl;

    // model variables remaining delta update
    S_.Kplus_ = S_.Kplus_ * std::exp(-1.0 / P_.tau_plus_);
    S_.Kplus_triplet_ = S_.Kplus_triplet_ * std::exp(-1.0 / P_.tau_x_);
    S_.Kminus_ = S_.Kminus_ * std::exp(-1.0 / P_.tau_minus_);
    S_.Kminus_triplet_ = S_.Kminus_triplet_ * std::exp(-1.0 / P_.tau_y_);

    if (current_pre_spikes_n > 0) {

      // depress
      // t = t^pre
      P_.weight_ = P_.weight_ - S_.Kminus_ * (P_.Aminus_ + P_.Aminus_triplet_ * S_.Kplus_triplet_);
      S_.Kplus_ = S_.Kplus_ + 1;
      S_.Kplus_triplet_ = S_.Kplus_triplet_ + 1;

      SpikeEvent se;
      se.set_multiplicity(current_pre_spikes_n);
      se.set_weight(P_.weight_);
      network()->send(*this, se, lag);

      // TODO : others things to set for spike event ?
      set_spiketime(Time::step(origin.get_steps() + lag + 1));
    }

    if (current_post_spikes_n > 0) {

      // potentiate
      // t = t^post
      P_.weight_ = P_.weight_ + S_.Kplus_ * (P_.Aplus_ + P_.Aplus_triplet_ * S_.Kminus_triplet_);
      S_.Kminus_ = S_.Kminus_ + 1;
      S_.Kminus_triplet_ = S_.Kminus_triplet_ + 1;
    }
  }
}

void stdpmodule::STDPTripletNeuron::handle(SpikeEvent &e) {

  assert(e.get_delay() > 0);

  B_.n_spikes_.add_value(
      e.get_rel_delivery_steps(network()->get_slice_origin()),
      static_cast<double_t>(e.get_multiplicity()));

  switch (e.get_rport()) {
  case 0: // PRE
    B_.n_pre_spikes_.add_value(
        e.get_rel_delivery_steps(network()->get_slice_origin()),
        e.get_weight() * e.get_multiplicity());
    break;

  case 1: // POST
    B_.n_post_spikes_.add_value(
        e.get_rel_delivery_steps(network()->get_slice_origin()),
        e.get_weight() * e.get_multiplicity());
    // TODO : actually don't care about the weight here ?
    break;

  default:
    break;
  }
}

void stdpmodule::STDPTripletNeuron::handle(DataLoggingRequest &e) {
	B_.logger_.handle(e);
}
