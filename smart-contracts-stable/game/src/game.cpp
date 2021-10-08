// SPDX-License-Identifier: HashCode-EULA-1.1-or-later
//
// Description / Summary:   Gambling Smart Contract (the "Software")
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

#include <game/game.hpp>
#include <affiliate/affiliate.hpp>

namespace x10bit {
  ACTION game::init(
    const asset& locked,
    const public_key& witness
  ) {
    require_auth(get_self());
    
    check(!state_.exists(), "already initialized");
    check(locked.is_valid(), "invalid locked asset");
    check(locked.amount >= 0, "locked amount must be non-negative");
    check(locked.symbol == EOS_SYMBOL, "foreign currency is not accepted");

    state_.set({
      0x00,
      locked,
      witness
    }, get_self());
  }

  ACTION game::startgame(const st_game& game) {
    require_auth(HOUSE_ACCOUNT);

    check(!checksum256_is_empty(game.house_seed_hash), "seed must not be empty");

    auto games_by_houseseedhash = games_table.get_index<"houseseedhash"_n>();
    auto game_itr = games_by_houseseedhash.find(game.house_seed_hash);
    
    check(game_itr == games_by_houseseedhash.end(), "this game was already initiated");

    time_point timestamp = current_time_point() + JET_LAG_US;
    games_table.emplace(get_self(), [&](r_game& _game) {
      _game.id = games_table.available_primary_key();
      _game.timestamp = timestamp;
      _game.house_seed_hash = game.house_seed_hash;
      _game.rules = game.rules;
      _game.bank = asset(0, EOS_SYMBOL);
    });

    action(
      permission_level{get_self(), "active"_n},
      get_self(),
      "logstartgame"_n,
      make_tuple(game, to_milliseconds(timestamp.time_since_epoch()))
    ).send();
  }

  ACTION game::logstartgame(const st_game& game, const int64_t& timestamp) {
    require_auth(get_self());
  }

  ACTION game::bet(const name& player, const st_bet& bet, const vector<st_affiliate>& affiliates, const signature& proof) {
    require_auth(player);

    auto games_by_houseseedhash = games_table.get_index<"houseseedhash"_n>();
    auto game_itr = games_by_houseseedhash.find(bet.game);

    check(game_itr != games_by_houseseedhash.end(), "this game does not exists");
    check(!checksum256_is_empty(bet.seed), "seed must not be empty");
    check(bet.quantity >= game_itr->rules.step, "bet amount too low");
    check(bet.quantity.symbol == EOS_SYMBOL, "foreign currency is not accepted");
    check(!bet.numbers.empty(), "bet numbers can not be empty");

    auto bets_by_seed = bets_table.get_index<"seed"_n>();
    auto bet_itr = bets_by_seed.find(bet.seed);
    check(bet_itr == bets_by_seed.end(), "seed collision occurred"); // apply context of the concrete game

    string payload = player.to_string() + bet.to_string();
    if (!affiliates.empty()) {
      for (const auto& _affiliate: affiliates) {
        payload += _affiliate.to_string();
      }
    }

    vector<char> bytes(payload.begin(), payload.end());
    vector<char> annex(32, 0);
    bytes.insert(
      bytes.end(),
      make_move_iterator(annex.begin()),
      make_move_iterator(annex.end())
    );

    const checksum256 digest = sha256(bytes.data(), bytes.size());
    assert_recover_key(digest, proof, state_.get().witness);
    
    uint8_t coeff = CELL_COEFF / bet.numbers.size();
    for (const uint8_t& _number: bet.numbers) {
      bets_table.emplace(get_self(), [&](r_bet& _bet) {
        _bet.id = bets_table.available_primary_key();
        _bet.game_id = game_itr->id;
        _bet.timestamp = current_time_point();
        _bet.player = player;
        _bet.quantity = bet.quantity;
        _bet.number = _number;
        _bet.coefficient = coeff;
        _bet.seed = bet.seed;
        _bet.paid = false;
        _bet.affiliates = affiliates;
      });
    }
  }

  NOTIFY game::deposit(const name& from, const name& to, const asset& quantity, const string& memo) {
    if (to == get_self()) {
      check(
        get_sender() == EOSIO_TOKEN,
        "deposit of " + get_sender().to_string() + " is not allowed"
      );
    }
    if (from == get_self() || from == BANK_ACCOUNT || to != BANK_ACCOUNT || memo.empty()) { return; }

    check(quantity.amount > 0, "amount cannot be zero");
    check(quantity.symbol == EOS_SYMBOL, "foreign currency is not accepted");

    checksum256 seed = hexstring_to_checksum256(memo);

    auto bets_by_seed = bets_table.get_index<"seed"_n>();
    auto bet_itr = bets_by_seed.require_find(seed, "bet does not exist");
    auto bet_end_itr = bets_by_seed.upper_bound(seed);
    auto game_itr = games_table.find(bet_itr->game_id);
    check(bet_itr->player == from, "it is not your bet");
    check(!bet_itr->paid, "bet was already deposited");
    check(bet_itr->quantity == quantity, "bet amount must be eq. " + bet_itr->quantity.to_string());

    if (!bet_itr->affiliates.empty()) {
      for (const auto& _affiliate : bet_itr->affiliates) {
        check(is_account(_affiliate.account), "affiliate account does not exist");
        check(_affiliate.account != bet_itr->player, "affiliate recursion is prohibited");

        const auto license = affiliate::get_license(
          AGENT_ACCOUNT, _affiliate.license,
          string("invalid affiliate license for " + _affiliate.account.to_string()).c_str()
        );

        if (license.terms.global_program) {
          const auto partner = affiliate::get_partner(
            AGENT_ACCOUNT, _affiliate.account,
            string(_affiliate.account.to_string() + " is not a valid partner").c_str()
          );
          check(
            utils::contains(partner.contracts, _affiliate.license),
            "license spoofing for " + _affiliate.account.to_string()
          );
        } else if (
          license.terms.instant_payout &&
          license.rate.payer == utils::to_underlying(affiliate::license_rate::payer::platform)
        ) {
          const auto fixed_rate = get_if<affiliate::license_rate::fixed>(&license.rate.limit);
          check(fixed_rate != nullptr, "invalid rate type for " + _affiliate.account.to_string());
          action(
            permission_level{BANK_ACCOUNT, CODE_PERMISSION},
            "eosio.token"_n,
            "transfer"_n,
            make_tuple(
              BANK_ACCOUNT,
              _affiliate.account,
              asset{static_cast<int64_t>(quantity.amount * *fixed_rate), EOS_SYMBOL},
              license.name + " reward! Check out affiliate programs at 16bit.partners (" + bet_itr->player.to_string() + ", " + checksum256_to_string(invert_checksum256(game_itr->house_seed_hash)) + ")",
              get_self()
            )
          ).send();
        }
      }
    }

    st_bet bet{
      game_itr->house_seed_hash,
      bet_itr->seed,
      bet_itr->quantity,
      {}
    };

    const auto affiliates = bet_itr->affiliates;

    for (bet_itr = bet_itr; bet_itr != bet_end_itr; bet_itr++) {
      bets_table.modify(*bet_itr, get_self(), [&](r_bet& _bet) {
        _bet.paid = true;
      });
      bet.numbers.emplace_back(bet_itr->number);
    }

    if (!checksum256_is_empty(game_itr->players_seed)) {
      seed = combine_checksum256(game_itr->players_seed, seed);
    }
    
    games_table.modify(game_itr, same_payer, [&](r_game& _game) {
      _game.players_seed = seed;
      _game.bank += quantity;
    });

    const uint8_t current_win = (seed.get_array()[0] + seed.get_array()[1]) % GAME_CELLS;

    action(
      permission_level{get_self(), "active"_n},
      get_self(),
      "logbet"_n,
      make_tuple(from, bet, seed, current_win, affiliates)
    ).send();
  }

  ACTION game::logbet(const name& player, const st_bet& bet, const checksum256& players_seed, const uint8_t& current_win, const vector<st_affiliate>& affiliates) {
    require_auth(get_self());
  };

  ACTION game::endgame(const string& house_seed) {
    require_auth(HOUSE_ACCOUNT);

    checksum256 house_seed_hash = sha256(house_seed.data(), house_seed.size());

    auto games_by_houseseedhash = games_table.get_index<"houseseedhash"_n>();
    auto game_itr = games_by_houseseedhash.find(house_seed_hash);
    
    check(game_itr != games_by_houseseedhash.end(), "no such game");

    string compound_hash_str = checksum256_to_string(invert_checksum256(game_itr->players_seed)) + house_seed;
    checksum256 compound_hash = invert_checksum256(sha256(compound_hash_str.data(), compound_hash_str.size()));
    const uint8_t win = (compound_hash.get_array()[0] + compound_hash.get_array()[1]) % GAME_CELLS;

    auto bets_by_gamenumber = bets_table.get_index<"gamenumber"_n>();
    auto bet_itr = bets_by_gamenumber.find(r_bet::build_compound_key(game_itr->id, win));

    string game_hash = checksum256_to_string(invert_checksum256(game_itr->house_seed_hash));

    while (bet_itr != bets_by_gamenumber.end()) {
      if ((bet_itr->player != get_self()) && bet_itr->paid && (bet_itr->number == win) && (bet_itr->coefficient > 0)) {
        action(
          permission_level(BANK_ACCOUNT, CODE_PERMISSION),
          "eosio.token"_n,
          "transfer"_n,
          make_tuple(
            BANK_ACCOUNT,
            bet_itr->player,
            bet_itr->quantity * bet_itr->coefficient,
            "Winner! Play " + get_self().to_string() + " at 16bit.game (" + game_hash + ")",
            get_self()
          )
        ).send();
      }
      bet_itr++;
    }

    action(
      permission_level{get_self(), "active"_n},
      get_self(),
      "logendgame"_n,
      make_tuple(house_seed_hash, compound_hash, win)
    ).send();

    action(
      permission_level{get_self(), "active"_n},
      get_self(),
      "clear"_n,
      make_tuple(game_itr->id)
    ).send();
  }

  ACTION game::logendgame(const checksum256& house_seed_hash, const checksum256& compound_hash, const uint8_t& win) {
    require_auth(get_self());
  }

  ACTION game::clear(const uint64_t& game_id) {
    require_auth(get_self());

    auto game_itr = games_table.find(game_id);
    check(game_itr != games_table.end(), "No such game");

    auto bets_by_game = bets_table.get_index<"game"_n>();
    auto bet_itr = bets_by_game.lower_bound(game_id);
    auto bet_end_itr = bets_by_game.upper_bound(game_id);

    while (bet_itr != bet_end_itr) {
      bet_itr = bets_by_game.erase(bet_itr);
    }

    games_table.erase(game_itr);
  }

  ACTION game::migrate(const uint64_t& version) {
    require_auth(get_self());
    auto _state = state_.get();
    check(_state.version != version, "redundant action");
    check(_state.version < version, "downgrade is not supported (rollback through subsequent updates)");

    switch(version) {
      case 0x00:
          _state.version = 0x00;
        break;
      default:
        check(false, "unknown version");
    }

    state_.set(_state, get_self());
  }

  /*ACTION game::reset(const uint16_t& count) {
    require_auth(get_self());

    utils::clear_table(games_table, count);
    utils::clear_table(bets_table, count);
  }*/
}
