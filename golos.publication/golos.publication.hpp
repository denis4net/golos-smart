#pragma once
#include "objects.hpp"

#include "eosio.token/eosio.token.hpp"

namespace eosio {

class publication : public eosio::contract {
  public:
    publication(account_name self);
    void apply(uint64_t code, uint64_t action);

  private:
    void create_post(account_name account, std::string permlink,
                     std::string parentid, std::string parentprmlnk,
                     uint64_t curatorprcnt, std::string payouttype,
                     std::vector<structures::beneficiary> beneficiaries, std::string paytype,
                     std::string headerpost, std::string bodypost,
                     std::string languagepost, std::vector<structures::tag> tags,
                     std::string jsonmetadata);

    void update_post(account_name account,
                     std::string permlink, std::string parentid, std::string parentprmlnk,
                     uint64_t curatorprcnt, std::string payouttype,
                     std::vector<structures::beneficiary> beneficiaries, std::string paytype, std::string headerpost,
                     std::string bodypost, std::string languagepost,
                     std::vector<structures::tag> tags, std::string jsonmetadata);
    void delete_post(account_name account, std::string permlink);

  private:
    tables::post_table _post_table;
    tables::content_table _content_table;
};

}

