package hashkey

import (
	"encoding/base64"
	"encoding/json"
	"fmt"

	"github.com/ethereum/go-ethereum/crypto"
)

type SigJSON struct {
	R string `json:"r"`
	S string `json:"s"`
	V int    `json:"v"`
}

//ECC 签名函数
func ECCSignature(message []byte, privateKey string) (signature string, err error) {
	//对message进行hash
	digest := crypto.Keccak256(message)
	//获取私钥
	skRaw, err := base64.StdEncoding.DecodeString(privateKey)
	if err != nil {
		return "", fmt.Errorf("ECCSignature error: %v", err)
	}
	sk, err := crypto.ToECDSA(skRaw)
	if err != nil {
		return "", fmt.Errorf("ECCSignature error: %v", err)
	}
	//对hash进行签名
	sig, err := crypto.Sign(digest, sk)
	if err != nil {
		return "", fmt.Errorf("ECCSignature error: %v", err)
	}
	//获取R,S,V
	r := base64.StdEncoding.EncodeToString(sig[:32])
	s := base64.StdEncoding.EncodeToString(sig[32:64])
	v := int(sig[64] + 27)
	//Marshal成json字符串
	sigJSON, err := json.Marshal(SigJSON{r, s, v})
	if err != nil {
		return "", fmt.Errorf("ECCSignature error: %v", err)
	}
	return string(sigJSON), nil
}
