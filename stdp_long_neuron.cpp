//
//  stdp_long_neuron.cpp
//  NEST
//
//

#include "stdp_long_neuron.h"

#include "stdpnames.h"
#include "network.h"
#include "universal_data_logger_impl.h"

using namespace nest;

/* ----------------------------------------------------------- devices */

nest::RecordablesMap<stdpmodule::STDPLongNeuron>
    stdpmodule::STDPLongNeuron::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
namespace nest {
template <> void RecordablesMap<stdpmodule::STDPLongNeuron>::create() {
  insert_(names::weight, &stdpmodule::STDPLongNeuron::get_weight_);
  insert_(stdpnames::Kplus, &stdpmodule::STDPLongNeuron::get_Kplus_);
  insert_(stdpnames::Kplus_triplet,
          &stdpmodule::STDPLongNeuron::get_Kplus_triplet_);
  insert_(stdpnames::Kminus, &stdpmodule::STDPLongNeuron::get_Kminus_);
  insert_(stdpnames::Kminus_triplet,
          &stdpmodule::STDPLongNeuron::get_Kminus_triplet_);
}
}

/* ----------------------------------------------------------- parameters */

stdpmodule::STDPLongNeuron::Parameters_::Parameters_()
    : tau_plus_(16.8), tau_plus_triplet_(101.0), tau_minus_(33.7),
      tau_minus_triplet_(125), Aplus_(0.1), Aminus_(0.1), Aplus_triplet_(0.1),
      Aminus_triplet_(0.1), nearest_spike_(false) {}

void stdpmodule::STDPLongNeuron::Parameters_::get(DictionaryDatum &d) const {
  def<double_t>(d, stdpnames::tau_plus, tau_plus_);
  def<double_t>(d, stdpnames::tau_plus_triplet, tau_plus_triplet_);
  def<double_t>(d, stdpnames::tau_minus, tau_minus_);
  def<double_t>(d, stdpnames::tau_minus_triplet, tau_minus_triplet_);
  def<double_t>(d, stdpnames::Aplus, Aplus_);
  def<double_t>(d, stdpnames::Aminus, Aminus_);
  def<double_t>(d, stdpnames::Aplus_triplet, Aplus_triplet_);
  def<double_t>(d, stdpnames::Aminus_triplet, Aminus_triplet_);
  def<bool>(d, stdpnames::nearest_spike, nearest_spike_);
}

void stdpmodule::STDPLongNeuron::Parameters_::set(const DictionaryDatum &d) {

  updateValue<double_t>(d, stdpnames::tau_plus, tau_plus_);
  updateValue<double_t>(d, stdpnames::tau_plus_triplet, tau_plus_triplet_);
  updateValue<double_t>(d, stdpnames::tau_minus, tau_minus_);
  updateValue<double_t>(d, stdpnames::tau_minus_triplet, tau_minus_triplet_);
  updateValue<double_t>(d, stdpnames::Aplus, Aplus_);
  updateValue<double_t>(d, stdpnames::Aminus, Aminus_);
  updateValue<double_t>(d, stdpnames::Aplus_triplet, Aplus_triplet_);
  updateValue<double_t>(d, stdpnames::Aminus_triplet, Aminus_triplet_);
  updateValue<bool>(d, stdpnames::nearest_spike, nearest_spike_);
}

/* ----------------------------------------------------------- states */

stdpmodule::STDPLongNeuron::State_::State_()
    : weight_(5.0), Kplus_(0.0), Kplus_triplet_(0.0), Kminus_(0.0),
      Kminus_triplet_(0.0) {}

void stdpmodule::STDPLongNeuron::State_::get(DictionaryDatum &d) const {
  def<double_t>(d, names::weight, weight_);
  def<double_t>(d, stdpnames::Kplus, Kplus_);
  def<double_t>(d, stdpnames::Kplus_triplet, Kplus_triplet_);
  def<double_t>(d, stdpnames::Kminus, Kminus_);
  def<double_t>(d, stdpnames::Kminus_triplet, Kminus_triplet_);
}

void stdpmodule::STDPLongNeuron::State_::set(const DictionaryDatum &d) {
  updateValue<double_t>(d, names::weight, weight_);
  updateValue<double_t>(d, stdpnames::Kplus, Kplus_);
  updateValue<double_t>(d, stdpnames::Kplus_triplet, Kplus_triplet_);
  updateValue<double_t>(d, stdpnames::Kminus, Kminus_);
  updateValue<double_t>(d, stdpnames::Kminus_triplet, Kminus_triplet_);
}

/* ----------------------------------------------------------- buffers */

stdpmodule::STDPLongNeuron::Buffers_::Buffers_(STDPLongNeuron &n)
    : logger_(n) {}

stdpmodule::STDPLongNeuron::Buffers_::Buffers_(const Buffers_ &,
                                               STDPLongNeuron &n)
    : logger_(n) {}

/* ----------------------------------------------------------- constructors */

stdpmodule::STDPLongNeuron::STDPLongNeuron()
    : Archiving_Node(), P_(), S_(), B_(*this) {
  recordablesMap_.create();
}

stdpmodule::STDPLongNeuron::STDPLongNeuron(const STDPLongNeuron &n)
    : Archiving_Node(n), P_(n.P_), S_(n.S_), B_(n.B_, *this) {}

/* ----------------------------------------------------------- initialization */

void stdpmodule::STDPLongNeuron::init_buffers_() {
  B_.n_pre_spikes_.clear();
  B_.n_post_spikes_.clear();
  B_.logger_.reset();
  Archiving_Node::clear_history();
}

void stdpmodule::STDPLongNeuron::calibrate() {
  B_.logger_.init();

  const double negative_delta = - Time::get_resolution().get_ms();

  // precompute decays
  V_.Zplus_decay_ = std::exp(negative_delta / P_.tau_plus_);
  V_.Zslow_decay_ = std::exp(negative_delta / P_.tau_slow_);
  V_.Zminus_decay_ = std::exp(negative_delta / P_.tau_minus_);
  V_.Zht_decay_ = std::exp(negative_delta / P_.tau_ht_);
}

/* ----------------------------------------------------------- updates */

void stdpmodule::STDPLongNeuron::update(Time const &origin, const long_t from,
                                        const long_t to) {
  assert(to >= 0 && (delay)from < Scheduler::get_min_delay());
  assert(from < to);

  double delta = Time::get_resolution().get_ms();

  for (long_t lag = from; lag < to; ++lag) {

    const double_t current_pre_spikes_n = B_.n_pre_spikes_.get_value(lag);
    const double_t current_post_spikes_n = B_.n_post_spikes_.get_value(lag);

    // model states decay
    S_.Zplus_ *= V_.Zplus_decay_;
    S_.Zslow_ *= V_.Zslow_decay_;
    S_.Zminus_ *= V_.Zminus_decay_;
    S_.Zht_ *= V_.Zht_decay_;
	  
	  // others states variables
	  S_.weight_ref_ +=  (S_.weight_ - S_.weight_ref_ - P_.P_ * S.weight_ref_ * ( P_.WP_ / 2.0 - S_.weight_ref_ ) * ( P_.WP_ - S.weight_ref_)) / P_.tau_const_  // (16)
	  S_.C_ -= S_.C_ / P_.tau_hom_ + S_.Zht_ * S_.Zht_; // (18)
	  S_.B_ = P_.A_ * std::min(S_.C_, 1.0); // (17)
	  
    if (current_pre_spikes_n > 0) {

      // depress: t = t^pre
		S_.weight_ -= S_.B_ * S_.Zminus_; // doublet LTD (12)
		S_.weight_ += delta_; // transmitter - induced (14)
		
		
		
      if (P_.nearest_spike_) {

	  }

      SpikeEvent se;
      se.set_multiplicity(current_pre_spikes_n);
      se.set_weight(S_.weight_);
      network()->send(*this, se, lag);

      set_spiketime(Time::step(origin.get_steps() + lag + 1));
    }

    if (current_post_spikes_n > 0) {

      // potentiate: t = t^post
		
		S_.weight_ += P_.A_ * S_.Zplus_ * S_.Zslow_; // triplet LTP (11)
		
		S_.weight_ -= P_.beta_ * (S_.weight_ - S_.weight_ref_) * S_.Zminus_ * S_.Zminus_ * S_.Zminus_; // heterosynpatic (13)
		
		S_.Zplus_ += 1.0;
		S_.Zslow_ += 1.0;
		S_.Zminus_ += 1.0;
		S_.Zht_ += 1.0;

      if (P_.nearest_spike_) {

	  }
		
    }

    B_.logger_.record_data(origin.get_steps() + lag);
  }
}

void stdpmodule::STDPLongNeuron::handle(SpikeEvent &e) {

  assert(e.get_delay() > 0);

  switch (e.get_rport()) {
  case 0: // PRE
    B_.n_pre_spikes_.add_value(
        e.get_rel_delivery_steps(network()->get_slice_origin()),
        e.get_multiplicity());
    break;

  case 1: // POST
    B_.n_post_spikes_.add_value(
        e.get_rel_delivery_steps(network()->get_slice_origin()),
        e.get_multiplicity());
    break;

  default:
    break;
  }
}

void stdpmodule::STDPLongNeuron::handle(DataLoggingRequest &e) {
  B_.logger_.handle(e);
}
