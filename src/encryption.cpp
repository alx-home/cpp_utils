/*
MIT License

Copyright (c) 2025 Alexandre GARCIN

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "cpp_utils/encryption.h"

#include <windows.h>
#include <wincrypt.h>
#include <string>

namespace cpp_utils {

/**
 * @brief Encrypts a string using Windows CryptoAPI with optional password entropy.
 *
 * Uses CryptProtectData to securely encrypt the input. The password is used as additional entropy.
 * Only the same Windows user (and password, if provided) can decrypt the data.
 *
 * @param input    The plaintext data to encrypt.
 * @param password Optional password/entropy to strengthen encryption (can be empty).
 * @return         Pair of (success, encrypted data as string).
 */
std::pair<bool, std::string>
Encrypt(std::string_view input, std::string_view password) {
   DATA_BLOB in_blob;
   in_blob.pbData          = (BYTE*)input.data();
   in_blob.cbData          = static_cast<DWORD>(input.size());
   DATA_BLOB  out_blob     = {};
   DATA_BLOB  entropy_blob = {};
   DATA_BLOB* entropy_ptr  = nullptr;
   if (!password.empty()) {
      entropy_blob.pbData = (BYTE*)password.data();
      entropy_blob.cbData = static_cast<DWORD>(password.size());
      entropy_ptr         = &entropy_blob;
   }
   if (!CryptProtectData(&in_blob, L"frt_cookie", entropy_ptr, nullptr, nullptr, 0, &out_blob)) {
      return {false, {}};
   }
   std::string output(reinterpret_cast<char*>(out_blob.pbData), out_blob.cbData);
   LocalFree(out_blob.pbData);
   return {true, output};
}

/**
 * @brief Decrypts a string using Windows CryptoAPI with optional password entropy.
 *
 * Uses CryptUnprotectData to securely decrypt the input. The password must match the one used for
 * encryption. Only the same Windows user (and password, if provided) can decrypt the data.
 *
 * @param input    The encrypted data to decrypt.
 * @param password Optional password/entropy used during encryption (must match).
 * @return         Pair of (success, decrypted plaintext as string).
 */
std::pair<bool, std::string>
Decrypt(std::string_view input, std::string_view password) {
   DATA_BLOB in_blob;
   in_blob.pbData          = (BYTE*)input.data();
   in_blob.cbData          = static_cast<DWORD>(input.size());
   DATA_BLOB  out_blob     = {};
   DATA_BLOB  entropy_blob = {};
   DATA_BLOB* entropy_ptr  = nullptr;
   if (!password.empty()) {
      entropy_blob.pbData = (BYTE*)password.data();
      entropy_blob.cbData = static_cast<DWORD>(password.size());
      entropy_ptr         = &entropy_blob;
   }
   if (!CryptUnprotectData(&in_blob, nullptr, entropy_ptr, nullptr, nullptr, 0, &out_blob)) {
      return {false, {}};
   }
   std::string output(reinterpret_cast<char*>(out_blob.pbData), out_blob.cbData);
   LocalFree(out_blob.pbData);
   return {true, output};
}

}  // namespace cpp_utils
