package hashkey

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/base64"
)

func SHA256HMAC(data []byte, key string) string {
	hmac := hmac.New(sha256.New, []byte(key))
	hmac.Write(data)
	return base64.StdEncoding.EncodeToString(hmac.Sum(nil))
}
