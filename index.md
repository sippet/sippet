---
layout: default
title: Home
description: C++ SIP stack based on Chrome source code
---

# Sippet

A C++ library designed to be a Chrome SIP stack.

**Current status**: Work-in-progress

Sippet is an open-source SIP User-Agent library, compliant with the IETF [RFC
3261](https://www.ietf.org/rfc/rfc3261.txt) specification. It can be used as a
building block for SIP client software for uses such as VoIP, IM, and many
other real-time and P2P communication services.

The main target was to enable Javascript applications to use UDP, TCP and TLS
transports along [WebSocket](http://en.wikipedia.org/wiki/WebSocket). Existing
SIP solutions for the browser are forced to use the
[WebSockets API](http://www.w3.org/TR/2011/WD-websockets-20110419/) to
send/receive SIP messages. But for many service providers, the WebSocket
protocol could not be a feasible solution due to scalability, support and other
business constraints.

By the way, the stack can be used as a full client-side C++ Stack
(full-featured) without Javascript integration.

# Features

* Support to symmetric response routing (RFC 3581).
* Support to client-initiated connections (RFC 5626).
* Multiplatform: same platforms supported by Chromium.
* Parsing of SIP and Tel URIs, compatible with GURL.
* TODO: Websockets, just in case.
* [SOCKS](http://en.wikipedia.org/wiki/SOCKS) for stream-oriented connections,
  inherited from system settings.
* Pass-through HTTP proxies using CONNECT (SIP over HTTP proxies).
* Flexible SSL/TLS support (client-side certificates, restrict or relaxed cipher list).

# Fun stuff

* Added G.729 support to WebRTC: use at your own risk, you have to pay
royalties if you want to use this codec commercially.
  * The source code is now based on ITU-T release of October 2006, and include
    some optimizations provided by the WebRTC signal processing library.
    Several optimizations available in other source codes (such as
    [CSipSimple](https://code.google.com/p/csipsimple/) and
    [Siphon](https://code.google.com/p/siphon/)) were discarded as they don't
    consider saturation. Precision was considered more important than
    optimization in this derived work.
* Created a 'compatibility mode' for WebRTC, so it can negotiate media without
supporting SRTP/DTLS and ICE.
