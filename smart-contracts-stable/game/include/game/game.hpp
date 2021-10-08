/**
 * SPDX-License-Identifier: HashCode-EULA-1.1-or-later
 *
 * Description / Summary:   Gambling Smart Contract (the "Software")
 *                          Part of the 16Bit Platform ecosystem
 *
 * Authors & Contributors:  Designed and assembled by GeekHack
 *                          In collaboration with 16Bit team
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Copyright (c) 2020 GeekHack ÐΞV
 * Copyright (c) 2021 HashCode Ltd.
 */

#pragma once

#include <vector>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>
#include "utils/utils.hpp"

#define NOTIFY          [[eosio::on_notify("*::transfer")]] void
#define EOS_SYMBOL      symbol("EOS", 4)
#define EOSIO_TOKEN     name("eosio.token")
#define HOUSE_ACCOUNT   name("fairbethouse")
#define AGENT_ACCOUNT   name("fairbetagent")
#define VAULT_ACCOUNT   name("fairbetvault")
#define BANK_ACCOUNT    name("fairbet.bank")
#define CODE_PERMISSION name("code")
#define JET_LAG_US      milliseconds(1000)

using namespace std;
using namespace eosio;

namespace x10bit {
  CONTRACT game : public contract {
    public:
      using contract::contract;

      const uint8_t GAME_CELLS = 16;
      const uint8_t CELL_COEFF = 15;
      const uint8_t PLOWBACK   = 90;

      game(name receiver, name code, datastream<const char *> ds):
        contract(receiver, code, ds), games_table(receiver, receiver.value),
        state_(receiver, receiver.value),
        bets_table(receiver, receiver.value)
      {}

      struct game_rules {
        uint8_t                 ttl;
        asset                   cap;
        asset                   step;
      };

      struct st_game {
        checksum256             house_seed_hash;
        game_rules              rules;

        EOSLIB_SERIALIZE(st_game, (house_seed_hash)(rules));
      };

      struct st_bet {
        checksum256             game;
        checksum256             seed;
        asset                   quantity;
        vector<uint8_t>         numbers;

        string to_string() const {
          const string numbers_concat(numbers.begin(), numbers.end());
          return 
            checksum256_to_string(game) +
            checksum256_to_string(seed) +
            quantity.to_string()        +
            numbers_concat;
        }
        EOSLIB_SERIALIZE(st_bet, (game)(seed)(quantity)(numbers));
      };

      struct st_affiliate {
        name                    account;
        uint64_t                license;

        string to_string() const { return account.to_string() + ::to_string(license); }
        EOSLIB_SERIALIZE(st_affiliate, (account)(license));
      };

      ACTION init(const asset& locked, const public_key& witness);
      ACTION startgame(const st_game& game);
      ACTION logstartgame(const st_game& game, const int64_t& timestamp);
      ACTION bet(const name& player, const st_bet& bet, const vector<st_affiliate>& affiliates, const signature& proof);
      ACTION logbet(const name& player, const st_bet& bet, const checksum256& players_seed, const uint8_t& current_win, const vector<st_affiliate>& affiliates);
      ACTION endgame(const string& house_seed);
      ACTION logendgame(const checksum256& house_seed_hash, const checksum256& compound_hash, const uint8_t& win);
      ACTION clear(const uint64_t& game_id);
      ACTION migrate(const uint64_t& version);
      //ACTION reset(const uint16_t& count);

      NOTIFY deposit(const name& from, const name& to, const asset& quantity, const string& memo);

    private:

      TABLE state {
        uint64_t      version;
        asset         locked;
        public_key    witness;

        EOSLIB_SERIALIZE(state, (version)(locked)(witness));
      };

      TABLE r_game {
        uint64_t                id;
        time_point              timestamp;
        checksum256             house_seed_hash;
        checksum256             players_seed;
        game_rules              rules;
        asset                   bank;

        uint64_t primary_key() const { return id; }
        checksum256 secondary_key() const { return house_seed_hash; }
      };
      
      TABLE r_bet {
        uint64_t                id;
        uint64_t                game_id;
        time_point              timestamp;
        name                    player;
        asset                   quantity;
        uint8_t                 number;
        uint8_t                 coefficient;
        checksum256             seed;
        bool                    paid;
        vector<st_affiliate>    affiliates;

        uint64_t primary_key() const { return id; }
        uint64_t partition_key() const { return game_id; }
        checksum256 secondary_key() const { return seed; }
        uint128_t compound_key() const { return build_compound_key(game_id, number); }
        static uint128_t build_compound_key(const uint64_t &_game_id, const uint8_t &_number) {
          return (uint128_t(_game_id) << 64) | uint128_t(_number);
        }

        EOSLIB_SERIALIZE(r_bet, (id)(game_id)(timestamp)
                                (player)(quantity)(number)(coefficient)
                                (seed)(paid)(affiliates)
        );
      };

      using state_idx = singleton<"state"_n, state>;
      typedef multi_index< "games"_n, r_game,
        indexed_by< "houseseedhash"_n, const_mem_fun<r_game, checksum256, &r_game::secondary_key> >
        > games_index;
      typedef multi_index< "bets"_n, r_bet,
        indexed_by< "game"_n, const_mem_fun<r_bet, uint64_t, &r_bet::partition_key> >,
        indexed_by< "seed"_n, const_mem_fun<r_bet, checksum256, &r_bet::secondary_key> >,
        indexed_by< "gamenumber"_n, const_mem_fun<r_bet, uint128_t, &r_bet::compound_key> >
        > bets_index;

      state_idx   state_;
      games_index games_table;
      bets_index  bets_table;

      inline bool checksum256_is_empty(const checksum256 cs) {
        uint8_t *first_word = (uint8_t *) &cs.get_array()[0];
        uint8_t *second_word = (uint8_t *) &cs.get_array()[1];
        for(int i = 0; i < 16; i++)
          if(first_word[i] != 0 && second_word[i] != 0)
            return false;
        return true;
      }

      inline checksum256 invert_checksum256(const checksum256 cs) {
        uint8_t *first_word = (uint8_t *) &cs.get_array()[0];
        uint8_t *second_word = (uint8_t *) &cs.get_array()[1];
        checksum256 cs_inverted;
        uint8_t *first_word_inverted = (uint8_t *) &cs_inverted.get_array()[0];
        uint8_t *second_word_inverted = (uint8_t *) &cs_inverted.get_array()[1];
        for (int i = 0; i < 16; i++) {
          first_word_inverted[15 - i] = first_word[i];
          second_word_inverted[15 - i] = second_word[i];
        }
        return cs_inverted;
      }

      inline checksum256 combine_checksum256(const checksum256 cs, const checksum256 cs2) {
        uint8_t *first_word = (uint8_t *) &cs.get_array()[0];
        uint8_t *second_word = (uint8_t *) &cs.get_array()[1];
        uint8_t *first_word2 = (uint8_t *) &cs2.get_array()[0];
        uint8_t *second_word2 = (uint8_t *) &cs2.get_array()[1];
        
        checksum256 cs_combined;
        uint8_t *first_word_combined = (uint8_t *) &cs_combined.get_array()[0];
        uint8_t *second_word_combined = (uint8_t *) &cs_combined.get_array()[1];
        for (int i = 0; i < 16; i++) {
          first_word_combined[i] = first_word[i] + first_word2[i];
          second_word_combined[i] = second_word[i] + second_word2[i];
        }
        return cs_combined;
      }

      inline vector<unsigned char> hexstring_to_vector32(const string_view s) {
        vector<unsigned char> v32;
        for(unsigned int i = 0; i < 32; i++)
        {
            unsigned int ui;
            sscanf(s.data() + (i * 2), "%02x", &ui);
            unsigned char uc = (unsigned char) ui;
            v32.push_back(uc);
        }
        return v32;
      }

      inline checksum256 hexstring_to_checksum256(const string_view hs) {
        vector hs_bytes = hexstring_to_vector32(hs);
        eosio::checksum256 cs;
        uint8_t *first_word = (uint8_t *) &cs.get_array()[0];
        uint8_t *second_word = (uint8_t *) &cs.get_array()[1];
        for (int i = 0; i < 16; i++)
        {
          first_word[i] = hs_bytes[i];
          second_word[i] = hs_bytes[i + 16];
        }
        return cs;
      }

      static string checksum256_to_string(const checksum256 cs) {
        uint8_t *first_word = (uint8_t *) &cs.get_array()[0];
        uint8_t *second_word = (uint8_t *) &cs.get_array()[1];
        char hexstr[64];
        for (int i = 0; i < 16; i++)
            sprintf(hexstr + i * 2, "%02x", first_word[i]);
        for (int i = 16; i < 32; i++)
            sprintf(hexstr + i * 2, "%02x", second_word[i - 16]);
        string c_str = string(hexstr);
        return c_str;
      }

      inline int64_t to_milliseconds(const microseconds& m) {
        return m._count / 1000;
      }
  };
}
