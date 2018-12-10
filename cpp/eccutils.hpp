#include <bitcoin/bitcoin.hpp> // https://github.com/libbitcoin/libbitcoin
#include <ethash/keccak.hpp> // https://github.com/chfast/ethash
#include <nlohmann/json.hpp> // https://github.com/nlohmann/json/tree/master

namespace xdaex {

    const int DIGEST_SIZE = 32;
    const int SK_SIZE = 32;

    nlohmann::json ParseSignature(const libbitcoin::recoverable_signature sig)
    {
        nlohmann::json sig_json;
        sig_json["r"] = libbitcoin::encode_base64(libbitcoin::slice<0,32>(sig.signature));
        sig_json["s"] = libbitcoin::encode_base64(libbitcoin::slice<32,64>(sig.signature));
        sig_json["v"] = 27 + unsigned(sig.recovery_id);
        return sig_json;
    }

    std::string ECCSignature(const std::string msg, const std::string secret)
    {
        auto digest = ethash::keccak256((const unsigned char *)msg.c_str(), msg.size());
        std::array<unsigned char, DIGEST_SIZE> digest_arr;
        std::copy_n(digest.bytes, DIGEST_SIZE, digest_arr.begin());
        
        std::vector<uint8_t> sk_vec;
        libbitcoin::decode_base64(sk_vec, secret);
        std::array<uint8_t, SK_SIZE> sk_arr;
        std::copy_n(sk_vec.begin(), SK_SIZE, sk_arr.begin());

        libbitcoin::recoverable_signature sig;
        libbitcoin::sign_recoverable(sig, sk_arr, digest_arr);

        return ParseSignature(sig).dump();
    }
}