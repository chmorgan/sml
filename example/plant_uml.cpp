//
// Copyright (c) 2016-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/sml.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <typeinfo>

namespace sml = boost::sml;

struct e1 {};
struct e2 {};
struct e3 {};
struct e4 {};

struct guard {
  bool operator()() const { return true; }
} guard;

struct action {
  void operator()() {}
} action;

void on_s1_entry() {}
void on_s2_exit() {}

struct plant_uml {
  auto operator()() const noexcept {
    using namespace sml;
    // clang-format off
    return make_transition_table(
       *"idle"_s + event<e1> = "s1"_s
      , "s1"_s + event<e2> [ guard ] / action = "s2"_s
      , "s1"_s + sml::on_entry<_> / [] { on_s1_entry(); }
      , "s2"_s + event<e3> [ guard ] = "s1"_s
      , "s2"_s + sml::on_exit<_> / [] { on_s2_exit(); }
      , "s2"_s + event<e4> / action = X
    );
    // clang-format on
  }
};

template <class T>
void dump_transition() noexcept {
  auto src_state = std::string{sml::aux::string<typename T::src_state>{}.c_str()};
  auto dst_state = std::string{sml::aux::string<typename T::dst_state>{}.c_str()};
  if (dst_state == "X") {
    dst_state = "[*]";
  }

  if (T::initial) {
    std::cout << "[*] --> " << src_state << std::endl;
  }

  const auto has_event = !sml::aux::is_same<typename T::event, sml::anonymous>::value;
  const auto has_guard = !sml::aux::is_same<typename T::guard, sml::front::always>::value;
  const auto has_action = !sml::aux::is_same<typename T::action, sml::front::none>::value;

  const auto entry_text = "on_entry<boost::ext::sml::";
  const auto exit_text = "on_exit<boost::ext::sml::";

  bool is_on_entry_or_exit = false;

  if(has_event) {
    // identify if this event is an entry or exit event
    auto event = std::string(boost::sml::aux::get_type_name<typename T::event>());
    if((event.find(entry_text) != std::string::npos) ||
       (event.find(exit_text) != std::string::npos)) {
      is_on_entry_or_exit = true;
    }
  }

  // entry / exit entry
  if(is_on_entry_or_exit) {
    std::cout << src_state;
  } else { // state to state transition
    std::cout << src_state << " --> " << dst_state;
  }

  if (has_event || has_guard || has_action) {
    std::cout << " :";
  }

  if (has_event) {
    // handle 'on_entry' and 'on_exit' per plant-uml syntax
    auto event = std::string(boost::sml::aux::get_type_name<typename T::event>());
    if(event.find(entry_text) != std::string::npos) {
      event = "entry";
    } else if(event.find(exit_text) != std::string::npos) {
      event = "exit";
    }
    std::cout << " " << event;
  }

  if (has_guard) {
    std::cout << " [" << boost::sml::aux::get_type_name<typename T::guard::type>() << "]";
  }

  if (has_action) {
    std::cout << " / " << boost::sml::aux::get_type_name<typename T::action::type>();
  }

  std::cout << std::endl;
}

template <template <class...> class T, class... Ts>
void dump_transitions(const T<Ts...>&) noexcept {
  int _[]{0, (dump_transition<Ts>(), 0)...};
  (void)_;
}

template <class SM>
void dump(const SM&) noexcept {
  std::cout << "@startuml" << std::endl << std::endl;
  dump_transitions(typename SM::transitions{});
  std::cout << std::endl << "@enduml" << std::endl;
}

int main() {
  sml::sm<plant_uml> sm;
  dump(sm);
}
