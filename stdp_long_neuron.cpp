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
}
}

/* ----------------------------------------------------------- parameters */

stdpmodule::STDPLongNeuron::Parameters_::Parameters_()
    : tau_plus_(200e-3), tau_slow_(100e-3), tau_minus_(200e-3), tau_ht_(3600),
      tau_hom_(1200), tau_const_(1200), A_(0.1), P_(1.0), WP_(0.5),
      beta_(5.0e-2), delta_(2.0e-2), nearest_spike_(false) {}

void stdpmodule::STDPLongNeuron::Parameters_::get(DictionaryDatum &d) const {
  def<double_t>(d, stdpnames::tau_plus, tau_plus_);
  def<double_t>(d, stdpnames::tau_slow, tau_slow_);
  def<double_t>(d, stdpnames::tau_minus, tau_minus_);
  def<double_t>(d, stdpnames::tau_ht, tau_ht_);
  def<double_t>(d, stdpnames::tau_hom, tau_hom_);
  def<double_t>(d, stdpnames::tau_const, tau_const_);
  def<double_t>(d, stdpnames::A, A_);
  def<double_t>(d, stdpnames::P, P_);
  def<double_t>(d, stdpnames::WP, WP_);
  def<double_t>(d, stdpnames::beta, beta_);
  def<double_t>(d, stdpnames::delta, delta_);
  def<bool>(d, stdpnames::nearest_spike, nearest_spike_);
}

void stdpmodule::STDPLongNeuron::Parameters_::set(const DictionaryDatum &d) {

  updateValue<double_t>(d, stdpnames::tau_plus, tau_plus_);
  updateValue<double_t>(d, stdpnames::tau_slow, tau_slow_);
  updateValue<double_t>(d, stdpnames::tau_minus, tau_minus_);
  updateValue<double_t>(d, stdpnames::tau_ht, tau_ht_);
  updateValue<double_t>(d, stdpnames::tau_hom, tau_hom_);
  updateValue<double_t>(d, stdpnames::tau_const, tau_const_);
  updateValue<double_t>(d, stdpnames::A, A_);
  updateValue<double_t>(d, stdpnames::P, P_);
  updateValue<double_t>(d, stdpnames::WP, WP_);
  updateValue<double_t>(d, stdpnames::beta, beta_);
  updateValue<double_t>(d, stdpnames::delta, delta_);
  updateValue<bool>(d, stdpnames::nearest_spike, nearest_spike_);
}

/* ----------------------------------------------------------- states */

stdpmodule::STDPLongNeuron::State_::State_()
    : weight_(1.0), weight_ref_(1.0), B_(0.0), C_(0.0), Zplus_(0.0),
      Zslow_(0.0), Zminus_(0.0), Zht_(0.0) {}

void stdpmodule::STDPLongNeuron::State_::get(DictionaryDatum &d) const {
  def<double_t>(d, names::weight, weight_);
  def<double_t>(d, stdpnames::weight_ref, weight_ref_);
  def<double_t>(d, stdpnames::B, B_);
  def<double_t>(d, stdpnames::C, C_);
  def<double_t>(d, stdpnames::Zplus, Zplus_);
  def<double_t>(d, stdpnames::Zslow, Zslow_);
  def<double_t>(d, stdpnames::Zminus, Zminus_);
  def<double_t>(d, stdpnames::Zht, Zht_);
}

void stdpmodule::STDPLongNeuron::State_::set(const DictionaryDatum &d) {
  updateValue<double_t>(d, names::weight, weight_);
  updateValue<double_t>(d, stdpnames::weight_ref, weight_ref_);
  updateValue<double_t>(d, stdpnames::B, B_);
  updateValue<double_t>(d, stdpnames::C, C_);
  updateValue<double_t>(d, stdpnames::Zplus, Zplus_);
  updateValue<double_t>(d, stdpnames::Zslow, Zslow_);
  updateValue<double_t>(d, stdpnames::Zminus, Zminus_);
  updateValue<double_t>(d, stdpnames::Zht, Zht_);
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

  const double negative_delta = -Time::get_resolution().get_ms();

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
    S_.weight_ref_ +=
        (S_.weight_ - S_.weight_ref_ -
         P_.P_ * S_.weight_ref_ * (P_.WP_ / 2.0 - S_.weight_ref_) *
             (P_.WP_ - S_.weight_ref_)) /
        P_.tau_const_;                                // (16)
    S_.C_ -= S_.C_ / P_.tau_hom_ + S_.Zht_ * S_.Zht_; // (18)
    S_.B_ = P_.A_ * std::min(S_.C_, 1.0);             // (17)

    if (current_pre_spikes_n > 0) {

      // depress: t = t^pre
      S_.weight_ -= S_.B_ * S_.Zminus_; // doublet LTD (12)
      S_.weight_ += P_.delta_;          // transmitter - induced (14)

      S_.Zplus_ += 1.0;

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
      S_.weight_ -= P_.beta_ * (S_.weight_ - S_.weight_ref_) * S_.Zminus_ *
                    S_.Zminus_ * S_.Zminus_; // heterosynpatic (13)

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
