// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// All supported headers must be kept in lexicographical order
// of their header names.
//
// Organized as follows:
// X(class_name, compact_form, header_name, enum_name, format)

X(Accept,                     0,   Accept,                        ACCEPT,                        MultipleTypeSubtypeParams)
//X(AcceptContact,              'a', Accept-Contact                 ACCEPT_CONTACT,                x)
X(AcceptEncoding,             0,   Accept-Encoding,               ACCEPT_ENCODING,               MultipleTokenParams)
X(AcceptLanguage,             0,   Accept-Language,               ACCEPT_LANGUAGE,               MultipleTokenParams)
//X(AcceptResourcePriority,     0,   Accept-Resource-Priority,      ACCEPT_RESOURCE_PRIORITY,      x)
X(AlertInfo,                  0,   Alert-Info,                    ALERT_INFO,                    MultipleUriParams)
X(Allow,                      0,   Allow,                         ALLOW,                         MultipleTokens)
//X(AllowEvents                 'u', Allow-Events,                  ALLOW_EVENTS,                  x)
//X(AnswerMode,                 0,   Answer-Mode,                   ANSWER_MODE,                   x)
X(AuthenticationInfo,         0,   Authentication-Info,           AUTHENTICATION_INFO,           OnlyAuthParams)
X(Authorization,              0,   Authorization,                 AUTHORIZATION,                 SchemeAndAuthParams)
X(CallId,                     'i', Call-ID,                       CALL_ID,                       SingleToken)
X(CallInfo,                   0,   Call-Info,                     CALL_INFO,                     MultipleUriParams)
X(Contact,                    'm', Contact,                       CONTACT,                       StarOrMultipleContactParams)
X(ContentDisposition,         0,   Content-Disposition,           CONTENT_DISPOSITION,           SingleTokenParams)
X(ContentEncoding,            'e', Content-Encoding,              CONTENT_ENCODING,              MultipleTokens)
X(ContentLanguage,            0,   Content-Language,              CONTENT_LANGUAGE,              MultipleTokens)
X(ContentLength,              'l', Content-Length,                CONTENT_LENGTH,                SingleInteger)
X(ContentType,                'c', Content-Type,                  CONTENT_TYPE,                  SingleTypeSubtypeParams)
X(Cseq,                       0,   CSeq,                          CSEQ,                          Cseq)
X(Date,                       0,   Date,                          DATE,                          Date)
X(ErrorInfo,                  0,   Error-Info,                    ERROR_INFO,                    MultipleUriParams)
//X(Event,                      'o', Event,                         EVENT,                         x)
X(Expires,                    0,   Expires,                       EXPIRES,                       SingleInteger)
//X(FeatureCaps,                0,   Feature-Caps,                  FEATURE_CAPS,                  x)
//X(Flow-Timer,                 0,   Flow-Timer                     FLOW_TIMER,                    x)
X(From,                       'f', From,                          FROM,                          SingleContactParams)
//X(Geolocation,                0,   Geolocation,                   GEOLOCATION,                   x)
//X(GeolocationError,           0,   Geolocation-Error,             GEOLOCATION_ERROR,             x)
//X(GeolocationRouting,         0,   Geolocation-Routing,           GEOLOCATION_ROUTING,           x)
//X(HistoryInfo,                0,   History-Info,                  HISTORY_INFO,                  x)
//X(Identity,                   'y', Identity,                      IDENTITY,                      x)
//X(IdentityInfo,               'n', Identity-Info,                 IDENTITY_INFO,                 x)
//X(InfoPackage,                0,   Info-Package,                  INFO_PACKAGE,                  x)
X(InReplyTo,                  0,   In-Reply-To,                   IN_REPLY_TO,                   MultipleTokens)
//X(Join,                       0,   Join,                          JOIN,                          x)
//X(MaxBreadth,                 0,   Max-Breadth,                   MAX_BREADTH,                   x)
X(MaxForwards,                0,   Max-Forwards,                  MAX_FORWARDS,                  SingleInteger)
X(MimeVersion,                0,   MIME-Version,                  MIME_VERSION,                  MimeVersion)
X(MinExpires,                 0,   Min-Expires,                   MIN_EXPIRES,                   SingleInteger)
//X(MinSE,                      0,   Min-SE,                        MIN_SE,                        x)
X(Organization,               0,   Organization,                  ORGANIZATION,                  TrimmedUtf8)
//X(PAccessNetworkInfo,         0,   P-Access-Network-Info,         P_ACCESS_NETWORK_INFO,         x)
//X(PAnswerState,               0,   P-Answer-State,                P_ANSWER_STATE,                x)
//X(PAssertedIdentity,          0,   P-Asserted-Identity,           P_ASSERTED_IDENTITY,           x)
//X(PAssertedService,           0,   P-Asserted-Service,            P_ASSERTED_SERVICE,            x)
//X(PAssociatedURI,             0,   P-Associated-URI,              P_ASSOCIATED_URI,              x)
//X(PCalledPartyID,             0,   P-Called-Party-ID,             P_CALLED_PARTY_ID,             x)
//X(PChargingFunctionAddresses, 0,   P-Charging-Function-Addresses, P_CHARGING_FUNCTION_ADDRESSES, x)
//X(PChargingVector,            0,   P-Charging-Vector,             P_CHARGING_VECTOR,             x)
//X(PDCSTracePartyID,           0,   P-DCS-Trace-Party-ID,          P_DCS_TRACE_PARTY_ID,          x)
//X(PDCSOSPS,                   0,   P-DCS-OSPS,                    P_DCS_OSPS,                    x)
//X(PDCSBillingInfo,            0,   P-DCS-Billing-Info,            P_DCS_BILLING_INFO,            x)
//X(PDCSLAES,                   0,   P-DCS-LAES,                    P_DCS_LAES,                    x)
//X(PDCSRedirect,               0,   P-DCS-Redirect,                P_DCS_REDIRECT,                x)
//X(PEarlyMedia,                0,   P-Early-Media,                 P_EARLY_MEDIA,                 x)
//X(PMediaAuthorization,        0,   P-Media-Authorization,         P_MEDIA_AUTHORIZATION,         x)
//X(PPreferredIdentity,         0,   P-Preferred-Identity,          P_PREFERRED_IDENTITY,          x)
//X(PPreferredService,          0,   P-Preferred-Service,           P_PREFERRED_SERVICE,           x)
//X(PProfileKey,                0,   P-Profile-Key,                 P_PROFILE_KEY,                 x)
//X(PRefusedURIList,            0,   P-Refused-URI-List,            P_REFUSED_URI_LIST,            x)
//X(PServedUser,                0,   P-Served-User,                 P_SERVED_USER,                 x)
//X(PUserDatabase,              0,   P-User-Database,               P_USER_DATABASE,               x)
//X(PVisitedNetworkID,          0,   P-Visited-Network-ID,          P_VISITED_NETWORK_ID,          x)
//X(Path,                       0,   Path,                          PATH,                          x)
//X(PermissionMissing,          0,   Permission-Missing,            PERMISSION_MISSING,            x)
//X(PolicyContact,              0,   Policy-Contact,                POLICY_CONTACT,                x)
//X(PolicyID,                   0,   Policy-ID,                     POLICY_ID,                     x)
X(Priority,                   0,   Priority,                      PRIORITY,                      SingleToken)
//X(PrivAnswerMode,             0,   Priv-Answer-Mode,              PRIV_ANSWER_MODE,              x)
//X(Privacy,                    0,   Privacy,                       PRIVACY,                       x)
X(ProxyAuthenticate,          0,   Proxy-Authenticate,            PROXY_AUTHENTICATE,            SchemeAndAuthParams)
X(ProxyAuthorization,         0,   Proxy-Authorization,           PROXY_AUTHORIZATION,           SchemeAndAuthParams)
X(ProxyRequire,               0,   Proxy-Require,                 PROXY_REQUIRE,                 MultipleTokens)
//X(RAck,                       0,   RAck,                          RACK,                          x)
//X(Reason,                     0,   Reason,                        REASON,                        x)
X(RecordRoute,                0,   Record-Route,                  RECORD_ROUTE,                  MultipleContactParams)
//X(RecvInfo,                   0,   Recv-Info,                     RECV_INFO,                     x)
//X(ReferSub,                   0,   ReferSub,                      REFER_SUB,                     x)
//X(ReferTo,                    'r', Refer-To,                      REFER_TO,                      x)
//X(ReferredBy,                 'b', Referred-By,                   REFERRED_BY,                   x)
//X(RejectContact,              'j', Reject-Contact,                REJECT_CONTACT,                x)
//X(Replaces,                   0,   Replaces,                      REPLACES,                      x)
X(ReplyTo,                    0,   Reply-To,                      REPLY_TO,                      SingleContactParams)
//X(RequestDisposition,         'd', Request-Disposition,           REQUEST_DISPOSITION,           x)
X(Require,                    0,   Require,                       REQUIRE,                       MultipleTokens)
//X(ResourcePriority,           0,   Resource-Priority,             RESOURCE_PRIORITY,             x)
X(RetryAfter,                 0,   Retry-After,                   RETRY_AFTER,                   RetryAfter)
X(Route,                      0,   Route,                         ROUTE,                         MultipleContactParams)
//X(RSeq,                       0,   RSeq,                          RSEQ,                          x)
//X(SecurityClient,             0,   Security-Client,               SECURITY_CLIENT,               x)
//X(SecurityServer,             0,   Security-Server,               SECURITY_SERVER,               x)
//X(SecurityVerify,             0,   Security-Verify,               SECURITY_VERIFY,               x)
X(Server,                     0,   Server,                        SERVER,                        TrimmedUtf8)
//X(ServiceRoute,               0,   Service-Route,                 SERVICE_ROUTE,                 x)
//X(SessionExpires,             0,   Session-Expires,               SESSION_EXPIRES,               x)
//X(SIPETag,                    0,   SIP-ETag,                      SIP_ETAG,                      x)
//X(SIPIfMatch,                 0,   SIP-If-Match,                  SIP_IF_MATCH,                  x)
X(Subject,                    's', Subject,                       SUBJECT,                       TrimmedUtf8)
//X(SubscriptionState,          0,   Subscription-State,            SUBSCRIPTION_STATE,            x)
X(Supported,                  'k', Supported,                     SUPPORTED,                     MultipleTokens)
//X(SuppressIfMatch,            0,   Suppress-If-Match,             SUPPRESS_IF_MATCH,             x)
//X(TargetDialog,               0,   Target-Dialog,                 TARGET_DIALOG,                 x)
X(Timestamp,                  0,   Timestamp,                     TIMESTAMP,                     Timestamp)
X(To,                         't', To,                            TO,                            SingleContactParams)
//X(TriggerConsent,             0,   Trigger-Consent,               TRIGGER_CONSENT,               x)
X(Unsupported,                0,   Unsupported,                   UNSUPPORTED,                   MultipleTokens)
X(UserAgent,                  0,   User-Agent,                    USER_AGENT,                    TrimmedUtf8)
X(Via,                        'v', Via,                           VIA,                           MultipleVias)
X(Warning,                    0,   Warning,                       WARNING,                       MultipleWarnings)
X(WwwAuthenticate,            0,   WWW-Authenticate,              WWW_AUTHENTICATE,              SchemeAndAuthParams)
