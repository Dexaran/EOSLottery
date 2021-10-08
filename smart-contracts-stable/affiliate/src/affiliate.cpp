// SPDX-License-Identifier: HashCode-EULA-1.1-or-later
//
// Description / Summary:   Affiliate Program Smart Contract (the "Software")
//                          Part of the 16Bit Platform ecosystem
//
// Authors & Contributors:  Designed and assembled by GeekHack
//                          In collaboration with 16Bit team
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Copyright (c) 2020 GeekHack ÐΞV
// Copyright (c) 2021 HashCode Ltd.

#include <affiliate/affiliate.hpp>
#include <eosio.token/eosio.token.hpp>

namespace x10bit {
  ACTION affiliate::init(
    const asset&            locked,
    const permission_level& witness,
    const global_ttl&       ttl
  ) {
    require_auth(get_self());
    
    check(!state.exists(), "already initialized");
    utils::check_asset(locked);
    check(is_account(witness.actor), "witness account does not exist");
    check(ttl.safeguard.to_seconds() > 0, "safeguard ttl must be positive");
    check(ttl.application.to_seconds() > 0, "application ttl must be positive");

    state.set({0x00, locked, witness, time_point(microseconds(0x00)), ttl}, get_self());
  }

  ACTION affiliate::config(const global_ttl& ttl) {
    utils::require_init(state);
    require_auth(get_self());

    check(ttl.safeguard.to_seconds() > 0, "safeguard ttl must be positive");
    check(ttl.application.to_seconds() > 0, "application ttl must be positive");

    auto _state = state.get();
    check(
      _state.ttl.safeguard != ttl.safeguard || _state.ttl.application != ttl.application,
      "redundant action"
    );

    _state.ttl = ttl;
    state.set(_state, get_self());
  }

  ACTION affiliate::rotate(const permission_level& witness) {
    utils::require_init(state);
    require_auth(get_self());

    auto _state = state.get();
    check(!(_state.witness == witness), "redundant action");

    _state.witness = witness;
    state.set(_state, get_self());
  }

  ACTION affiliate::evacuate(const string& reason) {
    utils::require_init(state);
    require_auth(get_self());

    check(!reason.empty(), "reason must be stated");
    const auto balance = token::get_balance(EOSIO_TOKEN, get_self(), EOS_SYMBOL.code());

    if (balance.amount > 0 && !utils::is_contract(CUSTODIAN_ACCOUNT)) {
      token::transfer_action{
        EOSIO_TOKEN, {get_self(), get_self_permission()}
      }.send(get_self(), CUSTODIAN_ACCOUNT, balance, "Сontingency protocol");
    }

    utils::clear_table(state);
  }

  ACTION affiliate::announce(
    const string&        license,
    const asset&         fee,
    const license_rate&  rate,
    const license_terms& terms
  ) {
    utils::require_init(state);
    require_auth(get_self());

    check(!license.empty(), "license name must not be empty");
    utils::check_asset(fee);

    check(rate.payer == utils::to_underlying(license_rate::payer::partner) ||
          rate.payer == utils::to_underlying(license_rate::payer::platform),
        "invalid rate payer");

    visit(utils::make_visitor(
      [&](const license_rate::fixed& fixed_rate) {
        check(
          fixed_rate > 0 && fixed_rate < 1,
          "rate must be greater than zero and less than one"
        );
        check(!terms.additionable, "rate limit terms not met (license cannot be additionable)");
      },
      [&](const license_rate::range& range_rate) {
        check(
          (range_rate.from > 0 && range_rate.from < 1) &&
          (range_rate.to   > 0 && range_rate.to   < 1),
          "rate range must be greater than zero and less than one"
        );
        check(range_rate.from < range_rate.to, "invalid rate range");
        check(!terms.additionable, "rate limit terms not met (license cannot be additionable)");
      },
      [&](const license_rate::p2p& p2p_rate) {
        check(
          (p2p_rate.from > 0 && p2p_rate.from < 1) &&
          (p2p_rate.to   > 0 && p2p_rate.to   < 1),
          "p2p rate range must be greater than zero and less than one"
        );
        check(p2p_rate.from < p2p_rate.to, "invalid p2p rate range");
      },
      [&](auto&&) { check(false, "invalid rate type"); }
    ), rate.limit);

    check(
      terms.platform_commission < 100,
      "commission percentage must be below 100"
    );
    for (const auto& _license : terms.allowed_scope) {
      const auto scope_itr = licenses.require_find(_license, "invalid license scope");
      if (scope_itr->terms.privileged_access)
        check(terms.privileged_access, "prevention of privilege escalation");
      check(
        scope_itr->terms.global_program == terms.global_program,
        "intersection of different types of programs"
      );
    }
    check(terms.referral_ttl.to_seconds() >= 0, "referral ttl must be non-negative");

    if (terms.franchise) {
      check(!terms.additionable, "franchise terms not met (license cannot be additionable)");
      check(!terms.upgradable, "franchise terms not met (license cannot be upgradable)");
      check(!terms.revocable, "franchise terms not met (license cannot be revocable)");
    }

    licenses.emplace(get_self(), [&](auto& _license) {
      _license.id       = licenses.available_primary_key();
      _license.name     = license;
      _license.fee      = fee;
      _license.rate     = rate;
      _license.terms    = terms;
      _license.active   = false;
      _license.archived = false;
    });
  }

  ACTION affiliate::activate(const uuid& license) {
    utils::require_init(state);
    require_auth(get_self());

    const auto license_itr = licenses.require_find(license, "invalid license");
    check(!license_itr->active, "redundant action");
    check(!license_itr->archived, "usage of archived license");

    licenses.modify(license_itr, same_payer, [&](auto& _license) {
      _license.active = true;
    });
  };

  ACTION affiliate::allocate(
    const name&            partner,
    const name&            affiliate,
    const uuid&            license,
    const optional<float>& rate,
    const asset&           discount,
          optional<actor>& broker,
    const bool&            approved
  ) {
    utils::require_init(state);
    require_auth(get_self());
    utils::require_account(partner);
    require_recipient(partner);

    check_partner(partner);
    if (affiliate)
      check_affiliate(affiliate);

    const auto license_itr = licenses.require_find(license, "invalid license");
    check(license_itr->active, "usage of inactive license");
    check(!license_itr->archived, "usage of archived license");
    check(license_itr->terms.global_program, "internal license is not allowed");
    check(license_itr->terms.franchise == !!affiliate, "franchise terms not met");

    visit(utils::make_visitor(
      [&](const license_rate::fixed& fixed_rate) {
        check(
          rate.has_value() && rate.value() == fixed_rate,
          "rate value must be equal to " + utils::to_string(fixed_rate)
        );
      },
      [&](const license_rate::range& range_rate) {
        check(
          rate.has_value() && rate.value() >= range_rate.from && rate.value() <= range_rate.to,
          "rate value must be in range from " + utils::to_string(range_rate.from) + " to " + utils::to_string(range_rate.to) + " (incl.)"
        );
      },
      [&](const license_rate::p2p&) {
        check(!rate.has_value(), "rate value must be unspecified");
      },
      [&](auto&&) { check(false, "invalid rate type"); }
    ), license_itr->rate.limit);

    utils::check_asset(discount, 0, license_itr->fee.amount);

    if (broker.has_value()) {
      const auto partners_account_idx = partners.get_index<"account"_n>();

      const auto broker_ptr = get_if<name>(&broker->self);
      check(broker_ptr != nullptr, "invalid broker reference");
      const auto broker_itr = partners_account_idx.require_find(
        broker_ptr->value, "broker does not exist"
      );
      broker->self = broker_itr->id;

      if (broker->root.has_value()) {
        const auto root_ptr = get_if<name>(&broker->root.value());
        check(root_ptr != nullptr, "invalid broker root reference");
        const auto root_itr = partners_account_idx.require_find(
          root_ptr->value, "broker root does not exist"
        );
        check(
          !root_itr->broker.has_value() || !root_itr->broker->root.has_value(),
          "broker root is malformed"
        );
        broker->root = make_optional(root_itr->id);
      }

      const auto& broker_license = licenses.get(
        broker->license, "invalid broker license"
      );
      const auto& broker_scope = broker_license.terms.allowed_scope;

      check(!broker_itr->suspended, "usage of suspended broker");
      check(utils::contains(broker_itr->contracts, broker->license), "broker license spoofing");
      check(utils::contains(broker_scope, license_itr->id), "broker is out of scope");

      visit(utils::make_visitor(
        [&](const license_rate::p2p& p2p_rate) {
          check(
            broker->rate >= p2p_rate.from && broker->rate <= p2p_rate.to,
            "broker rate value must be in range from " + utils::to_string(p2p_rate.from) + " to " + utils::to_string(p2p_rate.to) + " (incl.)"
          );
        },
        [&](auto&&) {
          const auto broker_rate = broker_itr->contracts.at(broker->license).value();
          check(
            broker->rate == broker_rate,
            "broker rate value must be equal to " + utils::to_string(broker_rate)
          );
        }
      ), broker_license.rate.limit);
    }

    if (approved) {
      partners.emplace(get_self(), [&](auto& _partner) {
        _partner.id        = partners.available_primary_key();
        _partner.primary   = partner;
        _partner.standby   = ""_n;
        _partner.affiliate = affiliate;
        _partner.contracts = {{license_itr->id, rate}};
        _partner.broker    = broker;
        _partner.balance   = discount - license_itr->fee;
        _partner.eol       = current_time_point() + state.get().ttl.safeguard;
        _partner.suspended = false;
      });
    } else {
      applications.emplace(get_self(), [&](auto& _application) {
        _application.id        = applications.available_primary_key();
        _application.owner     = partner;
        _application.affiliate = affiliate;
        _application.license   = license_itr->id;
        _application.rate      = rate;
        _application.fee       = license_itr->fee;
        _application.broker    = broker;
        _application.discount  = discount;
        _application.deposit   = asset{0ll, EOS_SYMBOL};
        _application.eol       = current_time_point() + state.get().ttl.application;
      });
    }
  }

  ACTION affiliate::empower(const entity& object, const uuid& capability, const string& reason) {
    utils::require_init(state);
    require_auth(get_self());

    visit(utils::make_visitor(
      [&](const entity::license_uuid& license) {
        const auto license_itr = licenses.require_find(license, "invalid object");
        const auto capability_itr = licenses.require_find(capability, "invalid capability");

        check(!license_itr->active, "modification of active license");
        check(!license_itr->archived, "modification of archived license");
        check(
          !utils::contains(license_itr->terms.allowed_scope, capability), "redundant action"
        );
        check(!capability_itr->archived, "assignment of archived capability");
        if (capability_itr->terms.privileged_access)
          check(license_itr->terms.privileged_access, "prevention of privilege escalation");
        check(
          capability_itr->terms.global_program == license_itr->terms.global_program,
          "intersection of different types of programs"
        );

        licenses.modify(license_itr, same_payer, [&](auto& _license) {
          _license.terms.allowed_scope.push_back(capability);
        });
      },
      [&](const entity::partner_name& partner) {
        utils::safe_require_recipient(partner);
        const auto partners_account_idx = partners.get_index<"account"_n>();
        const auto partner_itr = partners_account_idx.require_find(
          partner.value, "invalid object"
        );
        const auto capability_itr = licenses.require_find(capability, "invalid capability");

        check(!partner_itr->suspended, "modification of suspended partner");
        check(!utils::contains(partner_itr->contracts, capability), "redundant action");
        check(capability_itr->terms.additionable, "misappropriation of capability");
        check(capability_itr->active, "assignment of inactive capability");
        check(!capability_itr->archived, "assignment of archived capability");

        partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
          _partner.contracts.insert(make_pair(capability, nullopt));
        });
      },
      [&](auto&&) { check(false, "invalid entity type"); }
    ), object.id);

    check(!reason.empty(), "reason must be stated");
  }

  ACTION affiliate::revoke(const entity& object, const uuid& capability, const string& reason) {
    utils::require_init(state);
    require_auth(get_self());

    visit(utils::make_visitor(
      [&](const entity::license_uuid& license) {
        const auto license_itr = licenses.require_find(license, "invalid object");
        check(!license_itr->active, "modification of active license");
        check(!license_itr->archived, "modification of archived license");
        check(
          licenses.get(capability, "invalid capability").terms.revocable,
          "removal of irrevocable capability"
        );

        const auto& current_scope = license_itr->terms.allowed_scope;
        const auto capability_itr =
          find(current_scope.begin(), current_scope.end(), capability);
        check(capability_itr != current_scope.end(), "redundant action");

        licenses.modify(license_itr, same_payer, [&](auto& _license) {
          _license.terms.allowed_scope.erase(capability_itr);
        });
      },
      [&](const entity::partner_name& partner) {
        utils::safe_require_recipient(partner);
        const auto partners_account_idx = partners.get_index<"account"_n>();
        const auto partner_itr = partners_account_idx.require_find(
          partner.value, "invalid object"
        );
        check(!partner_itr->suspended, "modification of suspended partner");
        check(
          licenses.get(capability, "invalid capability").terms.revocable,
          "removal of irrevocable capability"
        );

        const auto& current_scope = partner_itr->contracts;
        const auto capability_itr = current_scope.find(capability);
        check(capability_itr != current_scope.end(), "redundant action");

        partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
          _partner.contracts.erase(capability_itr);
        });
      },
      [&](auto&&) { check(false, "invalid entity type"); }
    ), object.id);

    check(!reason.empty(), "reason must be stated");
  }

  ACTION affiliate::archive(const uuid& license, const string& reason) {
    utils::require_init(state);
    require_auth(get_self());

    const auto license_itr = licenses.require_find(license, "invalid license");
    check(license_itr->active, "usage of inactive license");
    check(!license_itr->archived, "redundant action");
    check(!reason.empty(), "reason must be stated");

    licenses.modify(license_itr, same_payer, [&](auto& _license) {
      _license.name    += " (archived)";
      _license.active   = false;
      _license.archived = true;
    });
  }

  ACTION affiliate::suspend(
    const name&          partner,
    const optional<uuid> deal,
    const string&        reason
  ) {
    utils::require_init(state);
    require_auth(get_self());
    utils::safe_require_recipient(partner);

    check(!reason.empty(), "reason must be stated");
    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );

    if (deal.has_value()) {
      const auto deal_itr = partners.require_find(
        deal.value(), "deal does not exist"
      );
      check(deal_itr->broker.has_value(), "invalid deal");
      const auto broker_ptr = get_if<uuid>(&deal_itr->broker->self);
      check(broker_ptr != nullptr, "invalid broker reference");
      check(*broker_ptr == partner_itr->id, "inconsistent data");
      check(!deal_itr->broker->suspended, "redundant action");

      partners.modify(deal_itr, same_payer, [&](auto& _partner) {
        _partner.broker->suspended = true;
      });
    } else {
      check(!partner_itr->suspended, "redundant action");

      partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
        _partner.suspended = true;
      });
    }
  }

  ACTION affiliate::resume(
    const name&          partner,
    const optional<uuid> deal,
    const string&        reason
  ) {
    utils::require_init(state);
    require_auth(get_self());
    utils::safe_require_recipient(partner);

    check(!reason.empty(), "reason must be stated");
    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );

    if (deal.has_value()) {
      const auto deal_itr = partners.require_find(
        deal.value(), "deal does not exist"
      );
      check(deal_itr->broker.has_value(), "invalid deal");
      const auto broker_ptr = get_if<uuid>(&deal_itr->broker->self);
      check(broker_ptr != nullptr, "invalid broker reference");
      check(*broker_ptr == partner_itr->id, "inconsistent data");
      check(deal_itr->broker->suspended, "redundant action");

      partners.modify(deal_itr, same_payer, [&](auto& _partner) {
        _partner.broker->suspended = false;
      });
    } else {
      check(partner_itr->suspended, "redundant action");

      partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
        _partner.suspended = false;
      });
    }
  }

  ACTION affiliate::sync(const vector<account>& accounts) {
    utils::require_init(state);
    require_auth(get_self());

    auto _state = state.get();
    const auto partners_account_idx = partners.get_index<"account"_n>();

    for (const auto& _account : accounts) {
      utils::safe_require_recipient(_account.owner);
      const auto partner_itr = partners_account_idx.require_find(
        _account.owner.value,
        string("partner (" + utils::to_string(_account.owner) + ") does not exist").c_str()
      );
      const auto license_itr = licenses.require_find(
        _account.license,
        string("invalid license of " + utils::to_string(_account.owner)).c_str()
      );
      check(!partner_itr->suspended, "usage of suspended partner (" + utils::to_string(_account.owner) + ")");
      check(utils::contains(partner_itr->contracts, _account.license), "license of " + utils::to_string(_account.owner) + " spoofing");
      check(_account.saldo.is_valid(), "invalid saldo for " + utils::to_string(_account.owner));
      check(_account.saldo.amount != 0, "redundant action for " + utils::to_string(_account.owner));
      check(_account.saldo.symbol == EOS_SYMBOL, "invalid saldo symbol for " + utils::to_string(_account.owner));

      partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
        _partner.balance += _account.saldo;
      });

      if (_account.saldo.amount > 0) {
        _state.locked += partner_itr->balance.amount > 0
                      ? _account.saldo
                      : max(partner_itr->balance + _account.saldo, asset{0ll, EOS_SYMBOL});
      } else if (partner_itr->balance.amount > 0) {
        _state.locked += max(_account.saldo, -partner_itr->balance);
      }
    }

    _state.sync = current_time_point();
    state.set(_state, get_self());
  }

  ACTION affiliate::apply(
    const name&  account,
    const uuid&  license,
    const float& rate,
    const asset& discount,
          actor& broker
  ) {
    utils::require_init(state);
    require_auth(account);
    require_auth(state.get().witness);
    utils::require_account(account);

    check_partner(account);

    const auto license_itr = licenses.require_find(license, "invalid license");
    check(license_itr->active, "usage of inactive license");
    check(!license_itr->archived, "usage of archived license");
    check(
      !license_itr->terms.privileged_access && license_itr->terms.global_program,
      "missing required authority for such license"
    );

    visit(utils::make_visitor(
      [&](const license_rate::fixed& fixed_rate) {
        check(
          rate == fixed_rate,
          "rate value must be equal to " + utils::to_string(fixed_rate)
        );
      },
      [&](const license_rate::range& range_rate) {
        check(
          rate >= range_rate.from && rate <= range_rate.to,
          "rate value must be in range from " + utils::to_string(range_rate.from) + " to " + utils::to_string(range_rate.to) + " (incl.)"
        );
      },
      [&](auto&&) { check(false, "invalid rate type"); }
    ), license_itr->rate.limit);

    {
      const auto partners_account_idx = partners.get_index<"account"_n>();

      const auto broker_ptr = get_if<name>(&broker.self);
      check(broker_ptr != nullptr, "invalid broker reference");
      const auto broker_itr = partners_account_idx.require_find(
        broker_ptr->value, "broker does not exist"
      );
      broker.self = broker_itr->id;

      if (broker.root.has_value()) {
        const auto root_ptr = get_if<name>(&broker.root.value());
        check(root_ptr != nullptr, "invalid broker root reference");
        const auto root_itr = partners_account_idx.require_find(
          root_ptr->value, "broker root does not exist"
        );
        check(
          !root_itr->broker.has_value() || !root_itr->broker->root.has_value(),
          "broker root is malformed"
        );
        broker.root = make_optional(root_itr->id);
      }

      const auto broker_license = licenses.get(
        broker.license, "invalid broker license"
      );
      const auto& broker_scope = broker_license.terms.allowed_scope;

      check(!broker_itr->suspended, "usage of suspended broker");
      check(utils::contains(broker_itr->contracts, broker.license), "broker license spoofing");
      check(utils::contains(broker_scope, license_itr->id), "broker is out of scope");

      visit(utils::make_visitor(
        [&](const license_rate::p2p& p2p_rate) {
          check(
            broker.rate >= p2p_rate.from && broker.rate <= p2p_rate.to,
            "broker rate value must be in range from " + utils::to_string(p2p_rate.from) + " to " + utils::to_string(p2p_rate.to) + " (incl.)"
          );
        },
        [&](auto&&) {
          const auto broker_rate = broker_itr->contracts.at(broker.license).value();
          check(
            broker.rate == broker_rate,
            "broker rate value must be equal to " + utils::to_string(broker_rate)
          );
        }
      ), broker_license.rate.limit);
    }

    utils::check_asset(discount, 0, license_itr->fee.amount);

    applications.emplace(account, [&](auto& _application) {
      _application.id        = applications.available_primary_key();
      _application.owner     = account;
      _application.affiliate = ""_n;
      _application.license   = license_itr->id;
      _application.rate      = rate;
      _application.fee       = license_itr->fee;
      _application.broker    = make_optional(broker);
      _application.discount  = discount;
      _application.deposit   = asset{0ll, EOS_SYMBOL};
      _application.eol       = current_time_point() + state.get().ttl.application;
    });
  }

  ACTION affiliate::refuse(const uuid& application) {
    utils::require_init(state);

    const auto application_itr = applications.require_find(
      application, "application does not exist"
    );
    const auto has_auth_self = has_auth(get_self());
    const auto has_auth_owner = has_auth(application_itr->owner);
    check(
      has_auth_self || has_auth_owner,
      "missing required authority of " + utils::to_string(get_self()) + " or " + utils::to_string(application_itr->owner)
    );
    if (has_auth_self && !has_auth_owner) {
      utils::safe_require_recipient(application_itr->owner);
      check(current_time_point() >= application_itr->eol, "application has not expired");
    }

    applications.erase(application_itr);

    if (application_itr->deposit.amount > 0) {
      action(
        permission_level{BANK_ACCOUNT, CODE_PERMISSION},
        EOSIO_TOKEN,
        "transfer"_n,
        make_tuple(
          BANK_ACCOUNT,
          application_itr->owner,
          application_itr->deposit,
          string("Affiliate license deposit refund (16bit.partners)"),
          get_self()
        )
      ).send();
    }
  }

  NOTIFY affiliate::transfer(
    const name&   from,
    const name&   to,
    const asset&  quantity,
    const string& memo
  ) {
    if (to == get_self()) {
      utils::check_asset(quantity, 1);
      check(
        get_sender() == EOSIO_TOKEN,
        "fake token transfer"
      );
      
      if (!utils::contains(whitelisted_senders, from))
        check(memo.empty(), "memo must be empty (spam protection)");
    } else if (to == BANK_ACCOUNT) {
      utils::require_init(state);
      utils::check_asset(quantity, 1);
      check(
        get_sender() == BANK_ACCOUNT,
        "fake token transfer"
      );

            auto _state = state.get();
      const auto partners_account_idx     = partners.get_index<"account"_n>();
      const auto applications_account_idx = applications.get_index<"account"_n>();
      const auto application_itr = applications_account_idx.find(from.value);
      const auto partner_itr     = partners_account_idx.find(from.value);

      if (application_itr != applications_account_idx.end()) {
        applications.modify(*application_itr, same_payer, [&](auto& _application) {
          _application.deposit += quantity;
        });
      } else if (partner_itr != partners_account_idx.end()) {
        partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
          _partner.balance += quantity;
        });
      } else {
        check(false, "no viable target");
      }

      _state.locked += quantity;
      state.set(_state, get_self());
    } else if (from == get_self()) {
      utils::require_account(to);
    } else if (from == BANK_ACCOUNT) {
      utils::require_init(state);
      utils::check_asset(quantity, 1);

      auto _state = state.get();
      const auto partners_account_idx = partners.get_index<"account"_n>();

      if (partners_account_idx.find(to.value) != partners_account_idx.end()) {
        _state.locked -= quantity;
        state.set(_state, get_self());
      } else {
        const auto balance = token::get_balance(EOSIO_TOKEN, BANK_ACCOUNT, EOS_SYMBOL.code());
        check(_state.locked <= balance, "overdraft is not allowed");
      }
    } else { return; }
  }

  ACTION affiliate::claim(const name& partner) {
    utils::require_init(state);
    require_auth(partner);
    utils::require_account(partner);

    auto _state = state.get();
    const auto applications_account_idx = applications.get_index<"account"_n>();
    const auto application_itr = applications_account_idx.require_find(
      partner.value, "application does not exist"
    );

    auto payable_fee = application_itr->fee - application_itr->discount;
    check(payable_fee <= application_itr->deposit, "deposit is not enough to cover fee");
    
    applications.erase(*application_itr);

    partners.emplace(get_self(), [&](auto& _partner) {
      _partner.id        = partners.available_primary_key();
      _partner.primary   = application_itr->owner;
      _partner.standby   = ""_n;
      _partner.affiliate = application_itr->affiliate;
      _partner.contracts = {{application_itr->license, application_itr->rate}};
      _partner.broker    = application_itr->broker;
      _partner.balance   = application_itr->deposit - payable_fee;
      _partner.suspended = false;
      _partner.eol       = current_time_point() + state.get().ttl.safeguard;
    });

    if (application_itr->broker.has_value() && !application_itr->broker->suspended) {
      const auto broker_ptr = get_if<uuid>(&application_itr->broker->self);
      check(broker_ptr != nullptr, "invalid broker reference");
      const auto broker_itr = partners.require_find(
        *broker_ptr, "broker does not exist"
      );
      const auto license_itr = licenses.require_find(
        application_itr->broker->license, "invalid broker license"
      );

      if (
        utils::contains(broker_itr->contracts, application_itr->broker->license) &&
        !broker_itr->suspended && license_itr->terms.instant_payout
      ) {
        const auto broker_reward =
          asset(payable_fee.amount * application_itr->broker->rate, EOS_SYMBOL);
        
        payable_fee -= broker_reward;

        partners.modify(broker_itr, same_payer, [&](auto& _partner) {
          _partner.balance += broker_reward;
        });
      }
    }

    _state.locked = max(_state.locked - payable_fee, asset{0ll, EOS_SYMBOL});
    state.set(_state, get_self());
  }

  ACTION affiliate::prolong(const name& partner) {
    utils::require_init(state);
    require_auth(partner);
    utils::require_account(partner);

    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );

    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      _partner.eol = current_time_point() + state.get().ttl.safeguard;
    });
  }

  ACTION affiliate::withdraw(const name& partner, const name& payee, const asset& quantity) {
    utils::require_init(state);
    require_auth(partner);
    utils::require_account(partner);

    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );

    check(is_account(payee), "payee account does not exist");
    utils::check_asset(quantity, 1, partner_itr->balance.amount);

    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      _partner.balance -= quantity;
    });

    action(
      permission_level{BANK_ACCOUNT, CODE_PERMISSION},
      EOSIO_TOKEN,
      "transfer"_n,
      make_tuple(
        BANK_ACCOUNT,
        payee,
        quantity,
        string("Affiliate program profit from 16bit.partners"),
        get_self()
      )
    ).send();
  }

  ACTION affiliate::update(
    const name&            partner,
    const uuid&            license,
    const optional<float>& rate,
    const optional<asset>& balance
  ) {
    utils::require_init(state);
    require_auth(partner);
    require_auth(state.get().witness);
    utils::require_account(partner);

    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );
    const auto license_itr = licenses.require_find(license, "invalid license");

    check(!partner_itr->suspended, "usage of suspended partner");
    check(utils::contains(partner_itr->contracts, license), "license spoofing");
    check(rate.has_value() || balance.has_value(), "redundant action");

    if (rate.has_value()) {
      check(partner_itr->contracts.at(license) != rate.value(), "redundant action");
      visit(utils::make_visitor(
        [&](const license_rate::range& range_rate) {
          check(
            rate >= range_rate.from && rate <= range_rate.to,
            "rate value must be in range from " + utils::to_string(range_rate.from) + " to " + utils::to_string(range_rate.to) + " (incl.)"
          );
        },
        [&](auto&&) {
          check(false, "license terms not met (rate is not updatable due to limit type)");
        }
      ), license_itr->rate.limit);
    }

    if (balance.has_value())
      check(balance.value().amount != 0, "redundant action");

    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      if (rate.has_value())
        _partner.contracts.at(license) = rate;
      if (balance.has_value())
        _partner.balance += balance.value();
    });

    if (balance.has_value()) {
      auto _state = state.get();
      if (balance.value().amount > 0) {
        _state.locked += partner_itr->balance.amount > 0
                      ? balance.value()
                      : max(partner_itr->balance + balance.value(), asset{0ll, EOS_SYMBOL});
      } else if (partner_itr->balance.amount > 0) {
        _state.locked += max(balance.value(), -partner_itr->balance);
      }
      state.set(_state, get_self());
    }
  }

  ACTION affiliate::upgrade(
    const name&             partner,
    const pair<uuid, uuid>& license,
    const optional<float>&  rate
  ) {
    utils::require_init(state);
    require_auth(partner);
    require_auth(state.get().witness);
    utils::require_account(partner);

    const auto [current_license_id, target_license_id] = license;
    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );
    const auto current_license_itr = licenses.require_find(
      current_license_id, "invalid current license"
    );
    const auto target_license_itr = licenses.require_find(
      target_license_id, "invalid target license"
    );
    const auto upgrade_fee = target_license_itr->fee - current_license_itr->fee;

    check(!partner_itr->suspended, "usage of suspended partner");
    check(
      utils::contains(partner_itr->contracts, current_license_id),
      "current license spoofing"
    );
    check(!utils::contains(partner_itr->contracts, target_license_id), "redundant action");
    check(current_license_itr->terms.upgradable, "current license is non-upgradable");
    check(target_license_itr->active, "target license is inactive");
    check(!target_license_itr->archived, "target license is archived");
    check(
      !target_license_itr->terms.privileged_access && target_license_itr->terms.global_program,
      "missing required authority for such target license"
    );
    check(
      current_license_itr->terms.franchise == target_license_itr->terms.franchise,
      "franchise terms not met"
    );

    visit(utils::make_visitor(
      [&](const license_rate::fixed&, const license_rate::fixed& fixed_rate) {
        check(
          rate == fixed_rate,
          "rate value must be equal to " + utils::to_string(fixed_rate)
        );
      },
      [&](const license_rate::range&, const license_rate::range& range_rate) {
        check(
          rate >= range_rate.from && rate <= range_rate.to,
          "rate value must be in range from " + utils::to_string(range_rate.from) + " to " + utils::to_string(range_rate.to) + " (incl.)"
        );
      },
      [&](const license_rate::p2p&, const license_rate::p2p&) {
        check(!rate.has_value(), "rate value must be unspecified");
      },
      [&](auto&&, auto&&) {
        check(false, "upgrade terms not met (rate limit types are not consistent)");
      }
    ), current_license_itr->rate.limit, target_license_itr->rate.limit);

    check(upgrade_fee.amount >= 0, "license downgrade is not allowed");
    check(upgrade_fee <= partner_itr->balance, "balance is not enough to cover upgrade fee");

    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      _partner.contracts.erase(current_license_itr->id);
      _partner.contracts.insert(make_pair(target_license_itr->id, rate));
      _partner.balance -= upgrade_fee;
    });

    auto _state = state.get();
    _state.locked = max(_state.locked - upgrade_fee, asset{0ll, EOS_SYMBOL});
    state.set(_state, get_self());
  }

  ACTION affiliate::insure(const name& partner, const name& standby) {
    utils::require_init(state);
    require_auth(partner);
    utils::require_account(partner);
    utils::require_account(standby);
    require_recipient(standby);

    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );
    check(!partner_itr->suspended, "usage of suspended partner");

    check_partner(standby);

    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      _partner.standby = standby;
    });
  }

  ACTION affiliate::failover(const name& partner) {
    utils::require_init(state);
    utils::safe_require_recipient(partner);

    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );
    check(!partner_itr->suspended, "usage of suspended partner");
    check(!!partner_itr->standby, "standby account not set");

    require_auth(partner_itr->standby);
    utils::require_account(partner_itr->standby);

    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      _partner.primary = partner_itr->standby;
      _partner.standby = partner_itr->primary;
    });
  }

  ACTION affiliate::delegate(const name& partner, const name& successor) {
    utils::require_init(state);
    utils::require_account(partner);
    utils::require_account(successor);
    require_recipient(partner, successor);

    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );
    check(!partner_itr->suspended, "usage of suspended partner");

    const auto has_auth_self = has_auth(get_self());
    const auto has_auth_partner = has_auth(partner_itr->primary);
    check(
      has_auth_self || has_auth_partner,
      "missing required authority of " + utils::to_string(get_self()) + " or " + utils::to_string(partner_itr->primary)
    );
    if (has_auth_self && !has_auth_partner)
      check(current_time_point() >= partner_itr->eol, "partner account has not expired");

    check_partner(successor);
    
    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      _partner.primary = successor;
      _partner.standby = ""_n;
    });
  }

  ACTION affiliate::quit(const name& partner) {
    utils::require_init(state);
    require_auth(partner);
    utils::require_account(partner);

    const auto partners_account_idx = partners.get_index<"account"_n>();
    const auto partner_itr = partners_account_idx.require_find(
      partner.value, "partner does not exist"
    );
    check(!partner_itr->suspended, "usage of suspended partner");

    partners.modify(*partner_itr, same_payer, [&](auto& _partner) {
      _partner.contracts.clear();
      _partner.balance   = asset{0ll, EOS_SYMBOL};
      _partner.suspended = true;
    });

    if (partner_itr->balance.amount > 0) {
      action(
        permission_level{BANK_ACCOUNT, CODE_PERMISSION},
        EOSIO_TOKEN,
        "transfer"_n,
        make_tuple(
          BANK_ACCOUNT,
          partner_itr->primary,
          partner_itr->balance,
          string("Affiliate program participation termination (16bit.partners)"),
          get_self()
        )
      ).send();
    }
  };

  #ifndef DEBUG
    ACTION affiliate::migrate(const uuid& version) {
      require_auth(get_self());

      auto _state = state.get();
      check(_state.version != version, "redundant action");
      check(_state.version < version, "version downgrade is not supported (use \"rollbacks through subsequent updates\" paradigm)");

      switch(version) {
        case 0x00:
          break;
        default: check(false, "invalid version");
      }

      _state.version = version;
      state.set(_state, get_self());
    }
  #else
    ACTION affiliate::reset(const uint8_t& type, const uint64_t& limit) {
      require_auth(get_self());

      switch(type) {
        case 0: utils::clear_table(state);
        case 1: utils::clear_table(licenses, limit);
        case 2: utils::clear_table(partners, limit);
        case 3: utils::clear_table(applications, limit);
          break;
        default: check(false, "unknown type");
      };
    }
  #endif
}
