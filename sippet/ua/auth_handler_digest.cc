// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_handler_digest.h"

#include <string>

#include "base/i18n/icu_string_conversions.h"
#include "base/logging.h"
#include "base/md5.h"
#include "base/rand_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "sippet/ua/auth.h"
#include "sippet/message/request.h"
#include "url/gurl.h"

namespace sippet {

// Based on net/http/http_auth_handler_digest.cc,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Digest authentication is specified in RFC 2617.
// The expanded derivations are listed in the tables below.

//==========+==========+==========================================+
//    qop   |algorithm |               response                   |
//==========+==========+==========================================+
//    ?     |  ?, md5, | MD5(MD5(A1):nonce:MD5(A2))               |
//          | md5-sess |                                          |
//--------- +----------+------------------------------------------+
//   auth,  |  ?, md5, | MD5(MD5(A1):nonce:nc:cnonce:qop:MD5(A2)) |
// auth-int | md5-sess |                                          |
//==========+==========+==========================================+
//    qop   |algorithm |                  A1                      |
//==========+==========+==========================================+
//          | ?, md5   | user:realm:password                      |
//----------+----------+------------------------------------------+
//          | md5-sess | MD5(user:realm:password):nonce:cnonce    |
//==========+==========+==========================================+
//    qop   |algorithm |                  A2                      |
//==========+==========+==========================================+
//  ?, auth |          | req-method:req-uri                       |
//----------+----------+------------------------------------------+
// auth-int |          | req-method:req-uri:MD5(req-entity-body)  |
//=====================+==========================================+

AuthHandlerDigest::NonceGenerator::NonceGenerator() {
}

AuthHandlerDigest::NonceGenerator::~NonceGenerator() {
}

AuthHandlerDigest::DynamicNonceGenerator::DynamicNonceGenerator() {
}

std::string AuthHandlerDigest::DynamicNonceGenerator::GenerateNonce() const {
  // This is how mozilla generates their cnonce -- a 16 digit hex string.
  static const char domain[] = "0123456789abcdef";
  std::string cnonce;
  cnonce.reserve(16);
  for (int i = 0; i < 16; ++i)
    cnonce.push_back(domain[base::RandInt(0, 15)]);
  return cnonce;
}

AuthHandlerDigest::FixedNonceGenerator::FixedNonceGenerator(
  const std::string& nonce)
  : nonce_(nonce) {
}

std::string AuthHandlerDigest::FixedNonceGenerator::GenerateNonce() const {
  return nonce_;
}

AuthHandlerDigest::Factory::Factory()
  : nonce_generator_(new DynamicNonceGenerator()) {
}

AuthHandlerDigest::Factory::~Factory() {
}

void AuthHandlerDigest::Factory::set_nonce_generator(
    const NonceGenerator* nonce_generator) {
  nonce_generator_.reset(nonce_generator);
}

int AuthHandlerDigest::Factory::CreateAuthHandler(
    const Challenge &challenge,
    Auth::Target target,
    const GURL& origin,
    CreateReason create_reason,
    int digest_nonce_count,
    const net::BoundNetLog& net_log,
    scoped_ptr<AuthHandler>* handler) {
  scoped_ptr<AuthHandler> tmp_handler(
      new AuthHandlerDigest(digest_nonce_count, nonce_generator_.get()));
  if (!tmp_handler->InitFromChallenge(challenge, target, origin, net_log))
    return net::ERR_INVALID_RESPONSE;
  handler->swap(tmp_handler);
  return net::OK;
}

Auth::AuthorizationResult AuthHandlerDigest::HandleAnotherChallenge(
    const Challenge& challenge) {
  // Even though Digest is not connection based, a "second round" is parsed
  // to differentiate between stale and rejected responses.
  // Note that the state of the current handler is not mutated - this way if
  // there is a rejection the realm hasn't changed.
  if (!base::LowerCaseEqualsASCII(challenge.scheme(), "digest"))
    return net::HttpAuth::AUTHORIZATION_RESULT_INVALID;

  // Try to find the "stale" value, and also keep track of the realm
  // for the new challenge.
  std::string original_realm;
  if (challenge.HasStale() && challenge.stale())
    return net::HttpAuth::AUTHORIZATION_RESULT_STALE;
  if (challenge.HasRealm())
    original_realm = challenge.realm();
  return (original_realm_ != original_realm) ?
      net::HttpAuth::AUTHORIZATION_RESULT_DIFFERENT_REALM :
      net::HttpAuth::AUTHORIZATION_RESULT_REJECT;
}

bool AuthHandlerDigest::Init(const Challenge& challenge) {
  return ParseChallenge(challenge);
}

int AuthHandlerDigest::GenerateAuthImpl(
    const net::AuthCredentials* credentials,
    const scoped_refptr<Request> &request,
    const net::CompletionCallback& callback) {
  // Generate a random client nonce.
  std::string cnonce = nonce_generator_->GenerateNonce();

  // Extract the request method and path -- the meaning of 'path' is overloaded
  // in certain cases, to be a hostname.
  std::string method;
  std::string request_uri;
  GetRequestMethodAndRequestUri(request, &method, &request_uri);

  AssembleCredentials(method, request_uri, *credentials,
                      cnonce, nonce_count_, request);
  return net::OK;
}

AuthHandlerDigest::AuthHandlerDigest(
    int nonce_count, const NonceGenerator* nonce_generator)
    : stale_(false),
      algorithm_(ALGORITHM_UNSPECIFIED),
      qop_(QOP_UNSPECIFIED),
      nonce_count_(nonce_count),
      nonce_generator_(nonce_generator) {
  DCHECK(nonce_generator_);
}

AuthHandlerDigest::~AuthHandlerDigest() {
}

// The digest challenge header looks like:
//   WWW-Authenticate: Digest
//     [realm="<realm-value>"]
//     nonce="<nonce-value>"
//     [domain="<list-of-URIs>"]
//     [opaque="<opaque-token-value>"]
//     [stale="<true-or-false>"]
//     [algorithm="<digest-algorithm>"]
//     [qop="<list-of-qop-values>"]
//     [<extension-directive>]
//
// Note that according to RFC 2617 (section 1.2) the realm is required.
// However we allow it to be omitted, in which case it will default to the
// empty string.
//
// This allowance breaks the default SIP RFC recommendations, though.
bool AuthHandlerDigest::ParseChallenge(
    const Challenge& challenge) {
  auth_scheme_ = net::HttpAuth::AUTH_SCHEME_DIGEST;
  score_ = 2;

  // Initialize to defaults.
  stale_ = false;
  algorithm_ = ALGORITHM_UNSPECIFIED;
  qop_ = QOP_UNSPECIFIED;
  realm_ = original_realm_ = nonce_ = domain_ = opaque_ = std::string();

  // FAIL -- Couldn't match auth-scheme.
  if (!base::LowerCaseEqualsASCII(challenge.scheme(), "digest"))
    return false;

  // Get all properties.
  if (challenge.HasRealm()) {
    std::string realm;
    if (!base::ConvertToUtf8AndNormalize(challenge.realm(),
        base::kCodepageLatin1, &realm))
      return false;
    original_realm_ = challenge.realm();
    realm_ = realm;
  }
  if (challenge.HasNonce()) {
    nonce_ = challenge.nonce();
  }
  if (challenge.HasDomain()) {
    domain_ = challenge.domain();
  }
  if (challenge.HasOpaque()) {
    opaque_ = challenge.opaque();
  }
  if (challenge.HasStale()) {
    stale_ = challenge.stale();
  }
  if (challenge.HasAlgorithm()) {
    std::string algorithm(challenge.algorithm());
    if (base::LowerCaseEqualsASCII(algorithm, "md5")) {
      algorithm_ = ALGORITHM_MD5;
    } else if (base::LowerCaseEqualsASCII(algorithm, "md5-sess")) {
      algorithm_ = ALGORITHM_MD5_SESS;
    } else {
      DVLOG(1) << "Unknown value of algorithm";
      return false;  // FAIL -- unsupported value of algorithm.
    }
  }
  if (challenge.HasQop()) {
    // Parse the comma separated list of qops.
    // auth is the preferred qop.
    std::string value(challenge.qop());
    net::HttpUtil::ValuesIterator qop_values(value.begin(), value.end(), ',');
    qop_ = QOP_UNSPECIFIED;
    while (qop_values.GetNext()) {
      if (base::LowerCaseEqualsASCII(qop_values.value(), "auth")) {
        qop_ = QOP_AUTH;
        break;
      }
      else if (base::LowerCaseEqualsASCII(qop_values.value(), "auth-int")) {
        qop_ = QOP_AUTH_INT;
        continue;
      }
    }
  }

  // Check that a minimum set of properties were provided.
  if (nonce_.empty())
    return false;

  return true;
}

// static
std::string AuthHandlerDigest::QopToString(QualityOfProtection qop) {
  switch (qop) {
    case QOP_UNSPECIFIED:
      return std::string();
    case QOP_AUTH:
      return "auth";
    case QOP_AUTH_INT:
      return "auth-int";
    default:
      NOTREACHED();
      return std::string();
  }
}

// static
Credentials::Algorithm AuthHandlerDigest::AlgorithmToCredentials(
    DigestAlgorithm algorithm) {
  switch (algorithm) {
    case ALGORITHM_MD5:
      return Credentials::MD5;
    case ALGORITHM_MD5_SESS:
      return Credentials::MD5_sess;
    default:
      NOTREACHED();
      return Credentials::Algorithm(-1);
  }
}

void AuthHandlerDigest::GetRequestMethodAndRequestUri(
    const scoped_refptr<Request> &request,
    std::string* method,
    std::string* request_uri) const {
  DCHECK(request);

  const GURL& uri = request->request_uri();
  *request_uri = uri.spec();

  *method = request->method().str();
}

std::string AuthHandlerDigest::AssembleResponseDigest(
    const std::string& method,
    const std::string& request_uri,
    const std::string& body,
    const net::AuthCredentials& credentials,
    const std::string& cnonce,
    int nonce_count) const {
  // the nonce-count is an 8 digit hex string.
  std::string nc = base::StringPrintf("%08x", nonce_count);

  // ha1 = MD5(A1)
  std::string ha1 = base::MD5String(base::UTF16ToUTF8(credentials.username())
      + ":" + original_realm_ + ":" +
      base::UTF16ToUTF8(credentials.password()));
  if (algorithm_ == AuthHandlerDigest::ALGORITHM_MD5_SESS)
    ha1 = base::MD5String(ha1 + ":" + nonce_ + ":" + cnonce);

  // ha2 = MD5(A2)
  std::string a2 = method + ":" + request_uri;
  if (qop_ == AuthHandlerDigest::QOP_AUTH_INT)
    a2 += ":" + base::MD5String(body);
  
  std::string ha2 = base::MD5String(a2);

  std::string nc_part;
  if (qop_ != AuthHandlerDigest::QOP_UNSPECIFIED) {
    nc_part = nc + ":" + cnonce + ":" + QopToString(qop_) + ":";
  }

  return base::MD5String(ha1 + ":" + nonce_ + ":" + nc_part + ha2);
}

void AuthHandlerDigest::AssembleCredentials(
    const std::string& method,
    const std::string& request_uri,
    const net::AuthCredentials& credentials,
    const std::string& cnonce,
    int nonce_count,
    const scoped_refptr<Request> &request) const {
  Credentials *cred;
  scoped_ptr<Header> credentials_header;
  if (net::HttpAuth::AUTH_PROXY == target_) {
    ProxyAuthorization *proxy_authorization = new ProxyAuthorization;
    credentials_header.reset(proxy_authorization);
    cred = proxy_authorization;
  }
  else {
    Authorization *authorization = new Authorization;
    credentials_header.reset(authorization);
    cred = authorization;
  }
  cred->set_scheme(Credentials::Digest);
  cred->set_username(base::UTF16ToUTF8(credentials.username()));
  cred->set_realm(original_realm_);
  cred->set_nonce(nonce_);
  cred->set_uri(request_uri);
  if (algorithm_ != ALGORITHM_UNSPECIFIED) {
    cred->set_algorithm(AlgorithmToCredentials(algorithm_));
  }
  std::string response = AssembleResponseDigest(method, request_uri,
    request->content(), credentials, cnonce, nonce_count);
  cred->set_response(response);
  if (!opaque_.empty()) {
    cred->set_opaque(opaque_);
  }
  if (qop_ != QOP_UNSPECIFIED) {
    cred->set_qop(QopToString(qop_));
    cred->set_nc(nonce_count);
    cred->set_cnonce(cnonce);
  }
  request->push_back(credentials_header.Pass());
}

} // namespace sippet
