from bitcoin import ecdsa_raw_sign, N, G, inv, hash_to_int, fast_add, fast_multiply, decode_pubkey
from bitcoin import privkey_to_pubkey
import sha3
import base64
import json

from datetime import datetime
from pprint import pprint
import requests

HALF_CURVE_ORDER = N >> 1


class Signer:
    @staticmethod
    def sign_message(json_msg, secret):
        int_secret = Signer.__int_key(secret)
        v, r, s = Signer.__sign(json_msg, int_secret)
        r_b = r.to_bytes(32, byteorder='big')
        s_b = s.to_bytes(32, byteorder='big')
        r_b64 = base64.b64encode(r_b).decode()
        s_b64 = base64.b64encode(s_b).decode()

        sign_data = {
            'r': r_b64,
            's': s_b64,
            'v': v
        }
        return json.dumps(sign_data)

    @staticmethod
    def __sign(json_msg, int_secret):
        msg_hash = Signer.__hash_msg(json_msg)
        v, r, s_ = ecdsa_raw_sign(msg_hash, int_secret)
        s = Signer.__trans_sig_s(s_)
        public_key = privkey_to_pubkey(int_secret)
        new_v = Signer.__correct_v(msg_hash, (r, s_), public_key)
        return new_v, r, s

    @staticmethod
    def __hash_msg(json_msg):
        k = sha3.keccak_256()
        encode_result = json_msg.replace(" ", "").encode()
        k.update(encode_result)
        return k.digest()

    @staticmethod
    def __trans_sig_s(sig_s):
        if not Signer.__is_canonical(sig_s):
            s = N - sig_s
        else:
            s = sig_s
        return s

    @staticmethod
    def __is_canonical(s):
        if s <= HALF_CURVE_ORDER:
            return True
        return False

    @staticmethod
    def __correct_v(msg_hash, rs, public_key):
        """
        27 = lower X even Y.
        28 = lower X odd Y.
        29 = higher X even Y.
        30 = higher X odd Y.
        Note that 29 and 30 are exceedingly rarely,
        and will in practice only ever be seen in specifically generated examples. ^29和30在非刻意构造的情况下统计上出现概率为0。
        """
        r, s = rs

        w = inv(s, N)
        z = hash_to_int(msg_hash)

        u1, u2 = z * w % N, r * w % N
        x, y = fast_add(fast_multiply(G, u1), fast_multiply(decode_pubkey(public_key), u2))
        return 27 + (y % 2)

    @staticmethod
    def __int_key(secret):
        secret_byte = base64.b64decode(secret)
        secret_int = int.from_bytes(secret_byte, byteorder='big')
        return secret_int


class Auth:
    @staticmethod
    def HashKey(key: str, secret: str, method: str, path: str, body: str = "") -> dict:
        timestamp = int((datetime.now().timestamp()) * 1000)
        sign_msg = f"{timestamp}{method}{path}{body}".replace(" ", "")

        return {"API-SIGNATURE": Signer.sign_message(json_msg=sign_msg, secret=secret),
                "API-KEY": key,
                "API-TIMESTAMP": str(timestamp),
                "Content-Type": "application/json",
                "AUTH-TYPE": "PUB-PRIV"}


if __name__ == '__main__':
    api_key = "MTU0MjEwNDAwMTA1NjAwMDAwMDAwNTQ="
    api_secret = "uvX6WIUzE5jJLMszT7elkTMKgRZEoYkx7X7mTpPWyXo="
    path = "/v1/info/time"

    headers = Auth.HashKey(key=api_key, secret=api_secret, method="GET", path=path)
    response = requests.get("https://api-preview.pro.hashkey.com/APITrade/v1/info/time", headers=headers)

    if response.status_code == 200:
        pprint(response.json())